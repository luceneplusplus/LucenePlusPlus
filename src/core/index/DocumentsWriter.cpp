/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "DocumentsWriter.h"
#include "_DocumentsWriter.h"
#include "DocumentsWriterThreadState.h"
#include "LuceneThread.h"
#include "IndexWriter.h"
#include "_IndexWriter.h"
#include "IndexWriterConfig.h"
#include "IndexReader.h"
#include "IndexSearcher.h"
#include "DocFieldProcessor.h"
#include "Term.h"
#include "TermDocs.h"
#include "TermVectorsTermsWriter.h"
#include "_TermVectorsTermsWriter.h"
#include "FreqProxTermsWriter.h"
#include "TermsHashConsumer.h"
#include "InvertedDocConsumer.h"
#include "TermsHash.h"
#include "DocInverter.h"
#include "NormsWriter.h"
#include "BufferedDeletes.h"
#include "FieldInfos.h"
#include "InfoStream.h"
#include "DocConsumerPerThread.h"
#include "SegmentWriteState.h"
#include "IndexFileNames.h"
#include "IndexFileDeleter.h"
#include "CompoundFileWriter.h"
#include "MergeDocIDRemapper.h"
#include "SegmentReader.h"
#include "SegmentInfos.h"
#include "SegmentInfo.h"
#include "SegmentDeletes.h"
#include "Query.h"
#include "Weight.h"
#include "Scorer.h"
#include "TestPoint.h"
#include "MiscUtils.h"
#include "StringUtils.h"
#include "AtomicLong.h"

namespace Lucene
{
    /// Initial chunks size of the shared byte[] blocks used to store postings data
    const int32_t DocumentsWriter::BYTE_BLOCK_SHIFT = 15;
    const int32_t DocumentsWriter::BYTE_BLOCK_SIZE = 1 << DocumentsWriter::BYTE_BLOCK_SHIFT;
    const int32_t DocumentsWriter::BYTE_BLOCK_MASK = DocumentsWriter::BYTE_BLOCK_SIZE - 1;
    const int32_t DocumentsWriter::BYTE_BLOCK_NOT_MASK = ~DocumentsWriter::BYTE_BLOCK_MASK;

    /// Initial chunk size of the shared char[] blocks used to store term text
    const int32_t DocumentsWriter::CHAR_BLOCK_SHIFT = 14;
    const int32_t DocumentsWriter::CHAR_BLOCK_SIZE = 1 << DocumentsWriter::CHAR_BLOCK_SHIFT;
    const int32_t DocumentsWriter::CHAR_BLOCK_MASK = DocumentsWriter::CHAR_BLOCK_SIZE - 1;

    const int32_t DocumentsWriter::MAX_TERM_LENGTH = DocumentsWriter::CHAR_BLOCK_SIZE - 1;

    /// Initial chunks size of the shared int[] blocks used to store postings data
    const int32_t DocumentsWriter::INT_BLOCK_SHIFT = 13;
    const int32_t DocumentsWriter::INT_BLOCK_SIZE = 1 << DocumentsWriter::INT_BLOCK_SHIFT;
    const int32_t DocumentsWriter::INT_BLOCK_MASK = DocumentsWriter::INT_BLOCK_SIZE - 1;

    const int32_t DocumentsWriter::PER_DOC_BLOCK_SIZE = 1024;
    
    DocumentsWriter::DocumentsWriter(IndexWriterConfigPtr config, DirectoryPtr directory, IndexWriterPtr writer, 
                                     FieldInfosPtr fieldInfos, BufferedDeletesPtr bufferedDeletes)
    {
        this->_bytesUsed = newLucene<AtomicLong>();
        this->threadStates = Collection<DocumentsWriterThreadStatePtr>::newInstance();
        this->threadBindings = SortedMapThreadDocumentsWriterThreadState::newInstance();
        this->freeIntBlocks = Collection<IntArray>::newInstance();
        this->freeCharBlocks = Collection<CharArray>::newInstance();
        
        this->directory = directory;
        this->_writer = writer;
        this->similarity = config->getSimilarity();
        this->maxThreadStates = config->getMaxThreadStates();
        this->fieldInfos = fieldInfos;
        this->bufferedDeletes = bufferedDeletes;
        this->flushControl = writer->flushControl;
        this->config = config;
    }
    
    DocumentsWriter::~DocumentsWriter()
    {
    }
    
    void DocumentsWriter::initialize()
    {
        nextDocID = 0;
        numDocs = 0;
        bufferIsFull = false;
        aborting = false;
        maxThreadStates = 0;
        pendingDeletes = newLucene<SegmentDeletes>();
        maxFieldLength = IndexWriter::DEFAULT_MAX_FIELD_LENGTH;
        closed = false;
        waitQueue = newLucene<WaitQueue>(shared_from_this());
        skipDocWriter = newLucene<SkipDocWriter>();
        byteBlockAllocator = newLucene<ByteBlockAllocator>(shared_from_this(), BYTE_BLOCK_SIZE);
        perDocAllocator = newLucene<ByteBlockAllocator>(shared_from_this(), PER_DOC_BLOCK_SIZE);
        consumer = config->getIndexingChain()->getChain(shared_from_this());
    }
    
    void DocumentsWriter::deleteDocID(int32_t docIDUpto)
    {
        SyncLock syncLock(this);
        pendingDeletes->addDocID(docIDUpto);
        // NOTE: we do not trigger flush here.  This is potentially a RAM leak, if you have an app 
        // that tries to add docs but every single doc always hits a non-aborting exception.  
        // Allowing a flush here gets very messy because we are only invoked when handling exceptions 
        // so to do this properly, while handling an exception we'd have to go off and flush new deletes
        // which is risky (likely would hit some other confounding exception).
    }
    
    bool DocumentsWriter::deleteQueries(Collection<QueryPtr> queries)
    {
        bool doFlush = flushControl->waitUpdate(0, queries.size());
        {
            SyncLock syncLock(this);
            for (Collection<QueryPtr>::iterator query = queries.begin(); query != queries.end(); ++query)
                pendingDeletes->addQuery(*query, numDocs);
        }
        return doFlush;
    }
    
    bool DocumentsWriter::deleteQuery(QueryPtr query)
    {
        bool doFlush = flushControl->waitUpdate(0, 1);
        {
            SyncLock syncLock(this);
            pendingDeletes->addQuery(query, numDocs);
        }
        return doFlush;
    }
    
    bool DocumentsWriter::deleteTerms(Collection<TermPtr> terms)
    {
        bool doFlush = flushControl->waitUpdate(0, terms.size());
        {
            SyncLock syncLock(this);
            for (Collection<TermPtr>::iterator term = terms.begin(); term != terms.end(); ++term)
                pendingDeletes->addTerm(*term, numDocs);
        }
        return doFlush;
    }
    
    bool DocumentsWriter::deleteTerm(TermPtr term, bool skipWait)
    {
        bool doFlush = flushControl->waitUpdate(0, 1, skipWait);
        {
            SyncLock syncLock(this);
            pendingDeletes->addTerm(term, numDocs);
        }
        return doFlush;
    }
    
    FieldInfosPtr DocumentsWriter::getFieldInfos()
    {
        return fieldInfos;
    }
    
    PerDocBufferPtr DocumentsWriter::newPerDocBuffer()
    {
        return newLucene<PerDocBuffer>(shared_from_this());
    }
    
    IndexingChainPtr DocumentsWriter::defaultIndexingChain()
    {
        static DefaultIndexingChainPtr _defaultIndexingChain;
        if (!_defaultIndexingChain)
        {
            _defaultIndexingChain = newLucene<DefaultIndexingChain>();
            CycleCheck::addStatic(_defaultIndexingChain);
        }
        return _defaultIndexingChain;
    }
    
    void DocumentsWriter::setInfoStream(InfoStreamPtr infoStream)
    {
        SyncLock syncLock(this);
        this->infoStream = infoStream;
        for (Collection<DocumentsWriterThreadStatePtr>::iterator threadState = threadStates.begin(); threadState != threadStates.end(); ++threadState)
            (*threadState)->docState->infoStream = infoStream;
    }
    
    void DocumentsWriter::setMaxFieldLength(int32_t maxFieldLength)
    {
        SyncLock syncLock(this);
        this->maxFieldLength = maxFieldLength;
        for (Collection<DocumentsWriterThreadStatePtr>::iterator threadState = threadStates.begin(); threadState != threadStates.end(); ++threadState)
            (*threadState)->docState->maxFieldLength = maxFieldLength;
    }
    
    void DocumentsWriter::setSimilarity(SimilarityPtr similarity)
    {
        SyncLock syncLock(this);
        this->similarity = similarity;
        for (Collection<DocumentsWriterThreadStatePtr>::iterator threadState = threadStates.begin(); threadState != threadStates.end(); ++threadState)
            (*threadState)->docState->similarity = similarity;
    }
    
    String DocumentsWriter::getSegment()
    {
        SyncLock syncLock(this);
        return segment;
    }
    
    int32_t DocumentsWriter::getNumDocs()
    {
        SyncLock syncLock(this);
        return numDocs;
    }
    
    void DocumentsWriter::message(const String& message)
    {
        if (infoStream)
            *infoStream << L"DW " << message << L"\n";
    }
    
    void DocumentsWriter::setAborting()
    {
        SyncLock syncLock(this);
        if (infoStream)
            message(L"setAborting");
        aborting = true;
    }
    
    void DocumentsWriter::abort()
    {
        TestScope testScope(L"DocumentsWriter", L"abort");
        SyncLock syncLock(this);
        if (infoStream)
            message(L"docWriter: abort");

        bool success = false;

        LuceneException finally;
        try
        {
            // Forcefully remove waiting ThreadStates from line
            waitQueue->abort();

            // Wait for all other threads to finish with
            // DocumentsWriter:
            waitIdle();

            if (infoStream)
                message(L"docWriter: abort waitIdle done");

            BOOST_ASSERT(waitQueue->numWaiting == 0);

            waitQueue->waitingBytes = 0;

            pendingDeletes->clear();

            for (Collection<DocumentsWriterThreadStatePtr>::iterator threadState = threadStates.begin(); threadState != threadStates.end(); ++threadState)
            {
                try
                {
                    (*threadState)->consumer->abort();
                }
                catch (...)
                {
                }
            }

            try
            {
                consumer->abort();
            }
            catch (...)
            {
            }

            // Reset all postings data
            doAfterFlush();
            success = true;
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        aborting = false;
        notifyAll();
        if (infoStream)
            message(L"docWriter: done abort; success=" + StringUtils::toString(success));
        finally.throwException();
    }
    
    void DocumentsWriter::doAfterFlush()
    {
        // All ThreadStates should be idle when we are called
        BOOST_ASSERT(allThreadsIdle());
        threadBindings.clear();
        waitQueue->reset();
        segment.clear();
        numDocs = 0;
        nextDocID = 0;
        bufferIsFull = false;
        for (Collection<DocumentsWriterThreadStatePtr>::iterator threadState = threadStates.begin(); threadState != threadStates.end(); ++threadState)
            (*threadState)->doAfterFlush();
    }
    
    bool DocumentsWriter::allThreadsIdle()
    {
        SyncLock syncLock(this);
        for (Collection<DocumentsWriterThreadStatePtr>::iterator threadState = threadStates.begin(); threadState != threadStates.end(); ++threadState)
        {
            if (!(*threadState)->isIdle)
                return false;
        }
        return true;
    }
    
    bool DocumentsWriter::anyChanges()
    {
        SyncLock syncLock(this);
        return (numDocs != 0 || pendingDeletes->any());
    }
    
    SegmentDeletesPtr DocumentsWriter::getPendingDeletes()
    {
        return pendingDeletes;
    }
    
    void DocumentsWriter::pushDeletes(SegmentInfoPtr newSegment, SegmentInfosPtr segmentInfos)
    {
        // Lock order: DW -> BD
        if (pendingDeletes->any())
        {
            if (newSegment)
            {
                if (infoStream)
                    message(L"flush: push buffered deletes to newSegment");
                bufferedDeletes->pushDeletes(pendingDeletes, newSegment);
            }
            else if (!segmentInfos->empty())
            {
                if (infoStream)
                    message(L"flush: push buffered deletes to previously flushed segment " + segmentInfos->info(segmentInfos->size() - 1)->toString());
                bufferedDeletes->pushDeletes(pendingDeletes, segmentInfos->info(segmentInfos->size() - 1), true);
            }
            else
            {
                if (infoStream)
                    message(L"flush: drop buffered deletes: no segments");
                // We can safely discard these deletes: since there are no segments, the deletions cannot affect anything.
            }
            pendingDeletes = newLucene<SegmentDeletes>();
        }
    }
    
    bool DocumentsWriter::anyDeletions()
    {
        return pendingDeletes->any();
    }
    
    SegmentInfoPtr DocumentsWriter::flush(IndexWriterPtr writer, IndexFileDeleterPtr deleter, MergePolicyPtr mergePolicy, SegmentInfosPtr segmentInfos)
    {
        int64_t startTime = MiscUtils::currentTimeMillis();

        // We change writer's segmentInfos:
        BOOST_ASSERT(writer->holdsLock());

        waitIdle();

        if (numDocs == 0)
        {
            // nothing to do!
            if (infoStream)
                message(L"flush: no docs; skipping");
            // Lock order: IW -> DW -> BD
            pushDeletes(SegmentInfoPtr(), segmentInfos);
            return SegmentInfoPtr();
        }

        if (aborting)
        {
            if (infoStream)
                message(L"flush: skip because aborting is set");
            return SegmentInfoPtr();
        }

        bool success = false;

        SegmentInfoPtr newSegment;

        LuceneException finally;
        try
        {
            BOOST_ASSERT(nextDocID == numDocs);
            BOOST_ASSERT(waitQueue->numWaiting == 0);
            BOOST_ASSERT(waitQueue->waitingBytes == 0);

            if (infoStream)
                message(L"flush postings as segment " + segment + L" numDocs=" + StringUtils::toString(numDocs));

            SegmentWriteStatePtr flushState(newLucene<SegmentWriteState>(infoStream, directory, segment, fieldInfos,
                                                                         numDocs, writer->getConfig()->getTermIndexInterval()));

            newSegment = newLucene<SegmentInfo>(segment, numDocs, directory, false, true, fieldInfos->hasProx(), false);

            Collection<DocConsumerPerThreadPtr> threads(Collection<DocConsumerPerThreadPtr>::newInstance());
            for (Collection<DocumentsWriterThreadStatePtr>::iterator threadState = threadStates.begin(); threadState != threadStates.end(); ++threadState)
                threads.add((*threadState)->consumer);

            double startMBUsed = bytesUsed() / 1024.0 / 1024.0;

            consumer->flush(threads, flushState);
            newSegment->setHasVectors(flushState->hasVectors);

            if (infoStream)
            {
                message(L"new segment has " + String(flushState->hasVectors ? L"vectors" : L"no vectors"));
                message(L"flushedFiles=" + StringUtils::toString(newSegment->files().size()));
            }

            if (mergePolicy->useCompoundFile(segmentInfos, newSegment))
            {
                String cfsFileName(IndexFileNames::segmentFileName(segment, IndexFileNames::COMPOUND_FILE_EXTENSION()));

                if (infoStream)
                    message(L"flush: create compound file \"" + cfsFileName + L"\"");
                
                CompoundFileWriterPtr cfsWriter(newLucene<CompoundFileWriter>(directory, cfsFileName));
                HashSet<String> files(newSegment->files());
                for (HashSet<String>::iterator fileName = files.begin(); fileName != files.end(); ++fileName)
                    cfsWriter->addFile(*fileName);
                cfsWriter->close();
                deleter->deleteNewFiles(newSegment->files());
                newSegment->setUseCompoundFile(true);
            }

            if (infoStream)
            {
                message(L"flush: segment=" + newSegment->toString());
                double newSegmentSizeNoStore = newSegment->sizeInBytes(false) / 1024.0 / 1024.0;
                double newSegmentSize = newSegment->sizeInBytes(true) / 1024.0 / 1024.0;
                message(L"  ramUsed=" + StringUtils::toString(startMBUsed) + L" MB" +
                        L" newFlushedSize=" + StringUtils::toString(newSegmentSize) + L" MB" +
                        L" (" + StringUtils::toString(newSegmentSizeNoStore) + L" MB w/o doc stores)" +
                        L" docs/MB=" + StringUtils::toString(numDocs / newSegmentSize) +
                        L" new/old=" + StringUtils::toString(100.0 * newSegmentSizeNoStore / startMBUsed) + L"%");
            }

            success = true;
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        notifyAll();
        if (!success)
        {
            if (!segment.empty())
                deleter->refresh(segment);
            abort();
        }
        finally.throwException();
        
        doAfterFlush();

        // Lock order: IW -> DW -> BD
        pushDeletes(newSegment, segmentInfos);

        if (infoStream)
            message(L"flush time " + StringUtils::toString(MiscUtils::currentTimeMillis() - startTime) + L" msec");

        return newSegment;
    }
    
    void DocumentsWriter::close()
    {
        SyncLock syncLock(this);
        closed = true;
        notifyAll();
    }
    
    DocumentsWriterThreadStatePtr DocumentsWriter::getThreadState(DocumentPtr doc, TermPtr delTerm)
    {
        SyncLock syncLock(this);
        
        int64_t currentThread = LuceneThread::currentId();
        IndexWriterPtr writer(_writer);
        BOOST_ASSERT(writer->holdsLock());
        
        // First, find a thread state.  If this thread already has affinity to a specific ThreadState, use that one again.
        DocumentsWriterThreadStatePtr state(threadBindings.get(currentThread));
        if (!state)
        {
            // First time this thread has called us since last flush.  Find the least loaded thread state
            DocumentsWriterThreadStatePtr minThreadState;
            for (Collection<DocumentsWriterThreadStatePtr>::iterator threadState = threadStates.begin(); threadState != threadStates.end(); ++threadState)
            {
                if (!minThreadState || (*threadState)->numThreads < minThreadState->numThreads)
                    minThreadState = *threadState;
            }
            if (minThreadState && (minThreadState->numThreads == 0 || threadStates.size() >= maxThreadStates))
            {
                state = minThreadState;
                ++state->numThreads;
            }
            else
            {
                // Just create a new "private" thread state
                threadStates.resize(threadStates.size() + 1);
                state = newLucene<DocumentsWriterThreadState>(shared_from_this());
                threadStates[threadStates.size() - 1] = state;
            }
            threadBindings.put(currentThread, state);
        }
        
        // Next, wait until my thread state is idle (in case it's shared with other threads), 
        // and no flush/abort pending 
        waitReady(state);

        // Allocate segment name if this is the first doc since last flush
        if (segment.empty())
        {
            segment = writer->newSegmentName();
            BOOST_ASSERT(numDocs == 0);
        }

        state->docState->docID = nextDocID++;

        if (delTerm)
            pendingDeletes->addTerm(delTerm, state->docState->docID);

        ++numDocs;
        state->isIdle = false;
        return state;
    }
    
    bool DocumentsWriter::addDocument(DocumentPtr doc, AnalyzerPtr analyzer)
    {
        return updateDocument(doc, analyzer, TermPtr());
    }
    
    bool DocumentsWriter::updateDocument(DocumentPtr doc, AnalyzerPtr analyzer, TermPtr delTerm)
    {
        // Possibly trigger a flush, or wait until any running flush completes
        bool doFlush = flushControl->waitUpdate(1, delTerm ? 1 : 0);

        // This call is synchronized but fast
        DocumentsWriterThreadStatePtr state(getThreadState(doc, delTerm));
        
        DocStatePtr docState(state->docState);
        docState->doc = doc;
        docState->analyzer = analyzer;
        
        bool success = false;
        LuceneException finally;
        try
        {
            // This call is not synchronized and does all the work
            DocWriterPtr perDoc;
            try
            {
                perDoc = state->consumer->processDocument();
            }
            catch (LuceneException& e)
            {
                finally = e;
            }
            docState->clear();
            finally.throwException();
            
            // This call is synchronized but fast
            finishDocument(state, perDoc);
            
            success = true;
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        if (!success)
        {
            // If this thread state had decided to flush, we must clear it so another thread can flush
            if (doFlush)
                flushControl->clearFlushPending();

            if (infoStream)
                message(L"exception in updateDocument aborting=" + StringUtils::toString(aborting));
            
            {
                SyncLock syncLock(this);

                state->isIdle = true;
                notifyAll();

                if (aborting)
                    abort();
                else
                {
                    skipDocWriter->docID = docState->docID;
                    bool success2 = false;
                    try
                    {
                        waitQueue->add(skipDocWriter);
                        success2 = true;
                    }
                    catch (LuceneException& e)
                    {
                        finally = e;
                    }
                    if (!success2)
                    {
                        abort();
                        return false;
                    }

                    // Immediately mark this document as deleted since likely it was partially added.  
                    // This keeps indexing as "all or none" (atomic) when adding a document
                    deleteDocID(state->docState->docID);
                }
            }
        }

        finally.throwException();
        
        if (flushControl->flushByRAMUsage(L"new document"))
            doFlush = true;

        return doFlush;
    }
    
    void DocumentsWriter::waitIdle()
    {
        SyncLock syncLock(this);
        while (!allThreadsIdle())
            wait();
    }
    
    void DocumentsWriter::waitReady(DocumentsWriterThreadStatePtr state)
    {
        SyncLock syncLock(this);
        while (!closed && (!state->isIdle || aborting))
            wait();
        if (closed)
            boost::throw_exception(AlreadyClosedException(L"this IndexWriter is closed"));
    }
    
    void DocumentsWriter::finishDocument(DocumentsWriterThreadStatePtr perThread, DocWriterPtr docWriter)
    {
        // Must call this without holding synchronized(this) else we'll hit deadlock
        balanceRAM();
        
        {
            SyncLock syncLock(this);
            BOOST_ASSERT(!docWriter || docWriter->docID == perThread->docState->docID);
            
            if (aborting)
            {
                // We are currently aborting, and another thread is waiting for me to become idle.  We 
                // just forcefully idle this threadState; it will be fully reset by abort()
                if (docWriter)
                {
                    try
                    {
                        docWriter->abort();
                    }
                    catch (...)
                    {
                    }
                }
                
                perThread->isIdle = true;
                
                // wakes up any threads waiting on the wait queue
                notifyAll();
                
                return;
            }
            
            bool doPause;
            
            if (docWriter)
                doPause = waitQueue->add(docWriter);
            else
            {
                skipDocWriter->docID = perThread->docState->docID;
                doPause = waitQueue->add(skipDocWriter);
            }
            
            if (doPause)
                waitForWaitQueue();
            
            perThread->isIdle = true;
            
            // wakes up any threads waiting on the wait queue
            notifyAll();
        }
    }
    
    void DocumentsWriter::waitForWaitQueue()
    {
        SyncLock syncLock(this);
        do
        {
            wait();
        }
        while (!waitQueue->doResume());
    }
    
    IntArray DocumentsWriter::getIntBlock()
    {
        SyncLock syncLock(this);
        int32_t size = freeIntBlocks.size();
        IntArray b;
        if (size == 0)
        {
            b = IntArray::newInstance(INT_BLOCK_SIZE);
            _bytesUsed->addAndGet(INT_BLOCK_SIZE * sizeof(int32_t));
        }
        else
            b = freeIntBlocks.removeLast();
        return b;
    }
    
    void DocumentsWriter::bytesUsed(int64_t numBytes)
    {
        SyncLock syncLock(this);
        _bytesUsed->addAndGet(numBytes);
    }
    
    int64_t DocumentsWriter::bytesUsed()
    {
        SyncLock syncLock(this);
        return _bytesUsed->get() + pendingDeletes->bytesUsed->get();
    }
    
    void DocumentsWriter::recycleIntBlocks(Collection<IntArray> blocks, int32_t start, int32_t end)
    {
        SyncLock syncLock(this);
        for (int32_t i = start; i < end; ++i)
        {
            freeIntBlocks.add(blocks[i]);
            blocks[i].reset();
        }
    }
    
    CharArray DocumentsWriter::getCharBlock()
    {
        SyncLock syncLock(this);
        int32_t size = freeCharBlocks.size();
        CharArray c;
        if (size == 0)
        {
            _bytesUsed->addAndGet(CHAR_BLOCK_SIZE * sizeof(wchar_t));
            c = CharArray::newInstance(CHAR_BLOCK_SIZE);
        }
        else
            c = freeCharBlocks.removeLast();
        // We always track allocations of char blocks for now because nothing that skips allocation tracking
        // (currently only term vectors) uses its own char blocks.
        return c;
    }
    
    void DocumentsWriter::recycleCharBlocks(Collection<CharArray> blocks, int32_t numBlocks)
    {
        SyncLock syncLock(this);
        for (int32_t i = 0; i < numBlocks; ++i)
        {
            freeCharBlocks.add(blocks[i]);
            blocks[i].reset();
        }
    }
    
    String DocumentsWriter::toMB(int64_t v)
    {
        return StringUtils::toString((double)v / 1024.0 / 1024.0);
    }
    
    void DocumentsWriter::balanceRAM()
    {
        bool doBalance = false;
        int64_t deletesRAMUsed = bufferedDeletes->bytesUsed();

        int64_t ramBufferSize = 0;
        double mb = config->getRAMBufferSizeMB();
        if (mb == IndexWriterConfig::DISABLE_AUTO_FLUSH)
            ramBufferSize = IndexWriterConfig::DISABLE_AUTO_FLUSH;
        else
            ramBufferSize = (int64_t)(mb * 1024 * 1024);
        
        {
            SyncLock syncLock(this);
            if (ramBufferSize == IndexWriterConfig::DISABLE_AUTO_FLUSH || bufferIsFull)
                return;
            doBalance = (bytesUsed() + deletesRAMUsed >= ramBufferSize);
        }

        if (doBalance)
        {
            if (infoStream)
            {
                message(L"  RAM: balance allocations: usedMB=" + toMB(bytesUsed()) +
                        L" vs trigger=" + toMB(ramBufferSize) +
                        L" deletesMB=" + toMB(deletesRAMUsed) +
                        L" byteBlockFree=" + toMB(byteBlockAllocator->freeByteBlocks.size() * BYTE_BLOCK_SIZE) +
                        L" perDocFree=" + toMB(perDocAllocator->freeByteBlocks.size() * PER_DOC_BLOCK_SIZE) +
                        L" charBlockFree=" + toMB(freeCharBlocks.size() * CHAR_BLOCK_SIZE * sizeof(wchar_t)));
            }

            int64_t startBytesUsed = bytesUsed() + deletesRAMUsed;

            int32_t iter = 0;

            // We free equally from each pool in 32 KB chunks until we are below our threshold (freeLevel)
            bool any = true;

            int64_t freeLevel = (int64_t)(0.95 * (double)ramBufferSize);

            while (bytesUsed() + deletesRAMUsed > freeLevel)
            {
                {
                    SyncLock syncLock(this);
                    if (perDocAllocator->freeByteBlocks.empty() && byteBlockAllocator->freeByteBlocks.empty() && 
                        freeCharBlocks.empty() && freeIntBlocks.empty() && !any)
                    {
                        // Nothing else to free -- must flush now.
                        bufferIsFull = (bytesUsed() + deletesRAMUsed > ramBufferSize);
                        if (infoStream)
                        {
                            if (bytesUsed() + deletesRAMUsed > ramBufferSize)
                                message(L"    nothing to free; set bufferIsFull");
                            else
                                message(L"    nothing to free");
                        }
                        break;
                    }

                    if ((0 == iter % 5) && !byteBlockAllocator->freeByteBlocks.empty())
                    {
                        byteBlockAllocator->freeByteBlocks.removeLast();
                        _bytesUsed->addAndGet(-BYTE_BLOCK_SIZE);
                    }

                    if ((1 == iter % 5) && !freeCharBlocks.empty())
                    {
                        freeCharBlocks.removeLast();
                        _bytesUsed->addAndGet(-CHAR_BLOCK_SIZE * (int32_t)sizeof(wchar_t));
                    }

                    if ((2 == iter % 5) && !freeIntBlocks.empty())
                    {
                        freeIntBlocks.removeLast();
                        _bytesUsed->addAndGet(-INT_BLOCK_SIZE * (int32_t)sizeof(int32_t));
                    }

                    if ((3 == iter % 5) && !perDocAllocator->freeByteBlocks.empty())
                    {
                        // Remove upwards of 32 blocks (each block is 1K)
                        for (int32_t i = 0; i < 32; ++i)
                        {
                            perDocAllocator->freeByteBlocks.removeLast();
                            _bytesUsed->addAndGet(-PER_DOC_BLOCK_SIZE);
                            if (perDocAllocator->freeByteBlocks.empty())
                                break;
                        }
                    }
                }

                if ((4 == iter % 5) && any)
                {
                    // Ask consumer to free any recycled state
                    any = consumer->freeRAM();
                }

                ++iter;
            }

            if (infoStream)
            {
                message(L"    after free: freedMB=" + 
                        StringUtils::toString((startBytesUsed - bytesUsed() - deletesRAMUsed) / 1024.0 / 1024.0) + 
                        L" usedMB=" + StringUtils::toString((bytesUsed() + deletesRAMUsed) / 1024.0 / 1024.0));
            }
        }
    }
    
    DocState::DocState()
    {
        maxFieldLength = 0;
        docID = 0;
    }
    
    DocState::~DocState()
    {
    }
    
    bool DocState::testPoint(const String& name)
    {
        return IndexWriterPtr(DocumentsWriterPtr(_docWriter)->_writer)->testPoint(name);
    }
    
    void DocState::clear()
    {
        // don't hold onto doc nor analyzer, in case it is large
        doc.reset();
        analyzer.reset();
    }
    
    PerDocBuffer::PerDocBuffer(DocumentsWriterPtr docWriter)
    {
        _docWriter = docWriter;
    }
    
    PerDocBuffer::~PerDocBuffer()
    {
    }
    
    ByteArray PerDocBuffer::newBuffer(int32_t size)
    {
        BOOST_ASSERT(size == DocumentsWriter::PER_DOC_BLOCK_SIZE);
        return DocumentsWriterPtr(_docWriter)->perDocAllocator->getByteBlock();
    }
    
    void PerDocBuffer::recycle()
    {
        SyncLock syncLock(this);
        if (!buffers.empty())
        {
            setLength(0);

            // Recycle the blocks
            DocumentsWriterPtr(_docWriter)->perDocAllocator->recycleByteBlocks(buffers);
            buffers.clear();
            sizeInBytes = 0;

            BOOST_ASSERT(numBuffers() == 0);
        }
    }
    
    DocWriter::DocWriter()
    {
        docID = 0;
    }
    
    DocWriter::~DocWriter()
    {
    }
    
    void DocWriter::setNext(DocWriterPtr next)
    {
        this->next = next;
    }
    
    IndexingChain::~IndexingChain()
    {
    }
    
    DefaultIndexingChain::~DefaultIndexingChain()
    {
    }
    
    DocConsumerPtr DefaultIndexingChain::getChain(DocumentsWriterPtr documentsWriter)
    {
        TermsHashConsumerPtr termVectorsWriter(newLucene<TermVectorsTermsWriter>(documentsWriter));
        TermsHashConsumerPtr freqProxWriter(newLucene<FreqProxTermsWriter>());
        
        InvertedDocConsumerPtr termsHash(newLucene<TermsHash>(documentsWriter, true, freqProxWriter,
                                                                 newLucene<TermsHash>(documentsWriter, false, 
                                                                 termVectorsWriter, TermsHashPtr())));
                                                                     
        DocInverterPtr docInverter(newLucene<DocInverter>(termsHash, newLucene<NormsWriter>()));
        return newLucene<DocFieldProcessor>(documentsWriter, docInverter);
    }
    
    SkipDocWriter::~SkipDocWriter()
    {
    }
    
    void SkipDocWriter::finish()
    {
    }
    
    void SkipDocWriter::abort()
    {
    }
    
    int64_t SkipDocWriter::sizeInBytes()
    {
        return 0;
    }
    
    WaitQueue::WaitQueue(DocumentsWriterPtr docWriter)
    {
        this->_docWriter = docWriter;
        waiting = Collection<DocWriterPtr>::newInstance(10);
        nextWriteDocID = 0;
        nextWriteLoc = 0;
        numWaiting = 0;
        waitingBytes = 0;
    }
    
    WaitQueue::~WaitQueue()
    {
    }
    
    void WaitQueue::reset()
    {
        SyncLock syncLock(this);
        // NOTE: nextWriteLoc doesn't need to be reset
        BOOST_ASSERT(numWaiting == 0);
        BOOST_ASSERT(waitingBytes == 0);
        nextWriteDocID = 0;
    }
    
    bool WaitQueue::doResume()
    {
        SyncLock syncLock(this);
        DocumentsWriterPtr docWriter(_docWriter);
        double mb = docWriter->config->getRAMBufferSizeMB();
        int64_t waitQueueResumeBytes = 0;
        if (mb == IndexWriterConfig::DISABLE_AUTO_FLUSH)
            waitQueueResumeBytes = 2 * 1024 * 1024;
        else
            waitQueueResumeBytes = (int64_t)(mb * 1024 * 1024 * 0.05);
        return (waitingBytes <= waitQueueResumeBytes);
    }
    
    bool WaitQueue::doPause()
    {
        SyncLock syncLock(this);
        DocumentsWriterPtr docWriter(_docWriter);
        double mb = docWriter->config->getRAMBufferSizeMB();
        int64_t waitQueuePauseBytes = 0;
        if (mb == IndexWriterConfig::DISABLE_AUTO_FLUSH)
            waitQueuePauseBytes = 4 * 1024 * 1024;
        else
            waitQueuePauseBytes = (int64_t)(mb * 1024 * 1024 * 0.1);
        return (waitingBytes > waitQueuePauseBytes);
    }
    
    void WaitQueue::abort()
    {
        SyncLock syncLock(this);
        int32_t count = 0;
        for (Collection<DocWriterPtr>::iterator doc = waiting.begin(); doc != waiting.end(); ++doc)
        {
            if (*doc)
            {
                (*doc)->abort();
                doc->reset();
                ++count;
            }
        }
        waitingBytes = 0;
        BOOST_ASSERT(count == numWaiting);
        numWaiting = 0;
    }
    
    void WaitQueue::writeDocument(DocWriterPtr doc)
    {
        DocumentsWriterPtr docWriter(_docWriter);
        BOOST_ASSERT(doc == DocumentsWriterPtr(docWriter)->skipDocWriter || nextWriteDocID == doc->docID);
        bool success = false;
        LuceneException finally;
        try
        {
            doc->finish();
            ++nextWriteDocID;
            ++nextWriteLoc;
            BOOST_ASSERT(nextWriteLoc <= waiting.size());
            if (nextWriteLoc == waiting.size())
                nextWriteLoc = 0;
            success = true;
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        if (!success)
            docWriter->setAborting();
        finally.throwException();
    }
    
    bool WaitQueue::add(DocWriterPtr doc)
    {
        SyncLock syncLock(this);
        BOOST_ASSERT(doc->docID >= nextWriteDocID);
        if (doc->docID == nextWriteDocID)
        {
            writeDocument(doc);
            while (true)
            {
                doc = waiting[nextWriteLoc];
                if (doc)
                {
                    --numWaiting;
                    waiting[nextWriteLoc].reset();
                    waitingBytes -= doc->sizeInBytes();
                    writeDocument(doc);
                }
                else
                    break;
            }
        }
        else
        {
            // I finished before documents that were added before me.  This can easily happen when I am a small doc 
            // and the docs before me were large, or just due to luck in the thread scheduling.  Just add myself to 
            // the queue and when that large doc finishes, it will flush me
            int32_t gap = doc->docID - nextWriteDocID;
            if (gap >= waiting.size())
            {
                // Grow queue
                Collection<DocWriterPtr> newArray(Collection<DocWriterPtr>::newInstance(MiscUtils::oversize(gap, sizeof(DocWriterPtr))));
                BOOST_ASSERT(nextWriteLoc >= 0);
                MiscUtils::arrayCopy(waiting.begin(), nextWriteLoc, newArray.begin(), 0, waiting.size() - nextWriteLoc);
                MiscUtils::arrayCopy(waiting.begin(), 0, newArray.begin(), waiting.size() - nextWriteLoc, nextWriteLoc);
                nextWriteLoc = 0;
                waiting = newArray;
                gap = doc->docID - nextWriteDocID;          
            }
            
            int32_t loc = nextWriteLoc + gap;
            if (loc >= waiting.size())
                loc -= waiting.size();
            
            // We should only wrap one time
            BOOST_ASSERT(loc < waiting.size());
            
            // Nobody should be in my spot!
            BOOST_ASSERT(!waiting[loc]);
            waiting[loc] = doc;
            ++numWaiting;
            waitingBytes += doc->sizeInBytes();
        }
        
        return doPause();
    }
    
    ByteBlockAllocator::ByteBlockAllocator(DocumentsWriterPtr docWriter, int32_t blockSize)
    {
        this->blockSize = blockSize;
        this->freeByteBlocks = Collection<ByteArray>::newInstance();
        this->_docWriter = docWriter;
    }
    
    ByteBlockAllocator::~ByteBlockAllocator()
    {
    }
    
    ByteArray ByteBlockAllocator::getByteBlock()
    {
        DocumentsWriterPtr docWriter(_docWriter);
        SyncLock syncLock(docWriter);
        int32_t size = freeByteBlocks.size();
        ByteArray b;
        if (size == 0)
        {
            b = ByteArray::newInstance(blockSize);
            MiscUtils::arrayFill(b.get(), 0, b.size(), 0);
        }
        else
            b = freeByteBlocks.removeLast();
        return b;
    }
    
    void ByteBlockAllocator::recycleByteBlocks(Collection<ByteArray> blocks, int32_t start, int32_t end)
    {
        DocumentsWriterPtr docWriter(_docWriter);
        SyncLock syncLock(docWriter);
        for (int32_t i = start; i < end; ++i)
        {
            freeByteBlocks.add(blocks[i]);
            blocks[i].reset();
        }
    }
    
    void ByteBlockAllocator::recycleByteBlocks(Collection<ByteArray> blocks)
    {
        DocumentsWriterPtr docWriter(_docWriter);
        SyncLock syncLock(docWriter);
        int32_t size = blocks.size();
        for (int32_t i = 0; i < size; ++i)
            freeByteBlocks.add(blocks[i]);
    }
}
