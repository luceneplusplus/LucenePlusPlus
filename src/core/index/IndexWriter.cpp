/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "IndexWriter.h"
#include "_IndexWriter.h"
#include "IndexWriterConfig.h"
#include "Directory.h"
#include "Analyzer.h"
#include "KeepOnlyLastCommitDeletionPolicy.h"
#include "DocumentsWriter.h"
#include "_DocumentsWriter.h"
#include "IndexFileDeleter.h"
#include "IndexFileNames.h"
#include "Lock.h"
#include "SegmentInfo.h"
#include "SegmentReader.h"
#include "ReadOnlyDirectoryReader.h"
#include "BufferedIndexInput.h"
#include "LogByteSizeMergePolicy.h"
#include "LogDocMergePolicy.h"
#include "Similarity.h"
#include "ConcurrentMergeScheduler.h"
#include "CompoundFileWriter.h"
#include "SegmentMerger.h"
#include "SegmentDeletes.h"
#include "DateTools.h"
#include "Constants.h"
#include "InfoStream.h"
#include "TestPoint.h"
#include "StringUtils.h"
#include "AtomicLong.h"
#include "BufferedDeletes.h"
#include "FieldInfos.h"
#include "CompoundFileReader.h"

namespace Lucene
{
    /// The normal read buffer size defaults to 1024, but increasing this during merging seems to 
    /// yield performance gains.  However we don't want to increase it too much because there are 
    /// quite a few BufferedIndexInputs created during merging.
    const int32_t IndexWriter::MERGE_READ_BUFFER_SIZE = 4096;
    
    InfoStreamPtr IndexWriter::defaultInfoStream;
    
    /// Default value for the write lock timeout (1000).
    /// @deprecated use {@link IndexWriterConfig#WRITE_LOCK_TIMEOUT} instead
    int64_t IndexWriter::WRITE_LOCK_TIMEOUT = IndexWriterConfig::WRITE_LOCK_TIMEOUT;
    
    const String IndexWriter::WRITE_LOCK_NAME = L"write.lock";
    
    /// Value to denote a flush trigger is disabled.
    /// @deprecated use {@link IndexWriterConfig#DISABLE_AUTO_FLUSH} instead
    const int32_t IndexWriter::DISABLE_AUTO_FLUSH = IndexWriterConfig::DISABLE_AUTO_FLUSH;
    
    /// Disabled by default (because IndexWriter flushes by RAM usage by default).
    /// @deprecated use {@link IndexWriterConfig#DEFAULT_MAX_BUFFERED_DOCS} instead.
    const int32_t IndexWriter::DEFAULT_MAX_BUFFERED_DOCS = IndexWriterConfig::DEFAULT_MAX_BUFFERED_DOCS;
    
    /// Default value is 16 MB (which means flush when buffered docs consume 16 MB RAM).
    /// @deprecated use {@link IndexWriterConfig#DEFAULT_RAM_BUFFER_SIZE_MB} instead.
    const double IndexWriter::DEFAULT_RAM_BUFFER_SIZE_MB = IndexWriterConfig::DEFAULT_RAM_BUFFER_SIZE_MB;
    
    /// Disabled by default (because IndexWriter flushes by RAM usage by default).
    /// @deprecated use {@link IndexWriterConfig#DEFAULT_MAX_BUFFERED_DELETE_TERMS} instead
    const int32_t IndexWriter::DEFAULT_MAX_BUFFERED_DELETE_TERMS = IndexWriterConfig::DEFAULT_MAX_BUFFERED_DELETE_TERMS;
    
    /// Sets the maximum field length to INT_MAX
    const int32_t IndexWriter::MaxFieldLengthUNLIMITED = INT_MAX;
    
    /// Sets the maximum field length to 10000
    const int32_t IndexWriter::MaxFieldLengthLIMITED = 10000;
    
    /// Default value is 10000.
    /// @deprecated see {@link IndexWriterConfig}
    const int32_t IndexWriter::DEFAULT_MAX_FIELD_LENGTH = IndexWriter::MaxFieldLengthUNLIMITED;
    
    /// Default value is 128.
    const int32_t IndexWriter::DEFAULT_TERM_INDEX_INTERVAL = IndexWriterConfig::DEFAULT_TERM_INDEX_INTERVAL;
    
    IndexWriter::IndexWriter(DirectoryPtr d, AnalyzerPtr a, bool create, int32_t mfl)
    {
        ConstructIndexWriter(d, newLucene<IndexWriterConfig>(LuceneVersion::LUCENE_31, a)->setOpenMode(create ? IndexWriterConfig::CREATE : IndexWriterConfig::APPEND));
        maxFieldLength = mfl;
    }
    
    IndexWriter::IndexWriter(DirectoryPtr d, AnalyzerPtr a, int32_t mfl)
    {
        ConstructIndexWriter(d, newLucene<IndexWriterConfig>(LuceneVersion::LUCENE_31, a));
        maxFieldLength = mfl;
    }
    
    IndexWriter::IndexWriter(DirectoryPtr d, AnalyzerPtr a, IndexDeletionPolicyPtr deletionPolicy, int32_t mfl)
    {
        ConstructIndexWriter(d, newLucene<IndexWriterConfig>(LuceneVersion::LUCENE_31, a)->setIndexDeletionPolicy(deletionPolicy));
        maxFieldLength = mfl;
    }
    
    IndexWriter::IndexWriter(DirectoryPtr d, AnalyzerPtr a, bool create, IndexDeletionPolicyPtr deletionPolicy, int32_t mfl)
    {
        ConstructIndexWriter(d, newLucene<IndexWriterConfig>(LuceneVersion::LUCENE_31, a)->setOpenMode(create ? IndexWriterConfig::CREATE : IndexWriterConfig::APPEND)->setIndexDeletionPolicy(deletionPolicy));
        maxFieldLength = mfl;
    }
    
    IndexWriter::IndexWriter(DirectoryPtr d, AnalyzerPtr a, IndexDeletionPolicyPtr deletionPolicy, int32_t mfl, IndexCommitPtr commit)
    {
        ConstructIndexWriter(d, newLucene<IndexWriterConfig>(LuceneVersion::LUCENE_31, a)->setOpenMode(IndexWriterConfig::APPEND)->setIndexDeletionPolicy(deletionPolicy)->setIndexCommit(commit));
        maxFieldLength = mfl;
    }
    
    IndexWriter::IndexWriter(DirectoryPtr d, IndexWriterConfigPtr conf)
    {
        ConstructIndexWriter(d, conf);
    }
    
    IndexWriter::~IndexWriter()
    {
    }
    
    void IndexWriter::ConstructIndexWriter(DirectoryPtr d, IndexWriterConfigPtr conf)
    {
        messageID = MESSAGE_ID()->getAndIncrement();
        writeLockTimeout = 0;
        segmentInfos = newLucene<SegmentInfos>();
        pendingMerges = Collection<OneMergePtr>::newInstance();
        mergeExceptions = Collection<OneMergePtr>::newInstance();
        segmentsToOptimize = SetSegmentInfo::newInstance();
        optimizeMaxNumSegments = 0;
        mergingSegments = SetSegmentInfo::newInstance();
        runningMerges = SetOneMerge::newInstance();
        changeCount = 0;
        lastCommitChangeCount = 0;
        poolReaders = false;
        closed = false;
        closing = false;
        hitOOM = false;
        stopMerges = false;
        mergeGen = 0;
        flushCount = newLucene<AtomicLong>();
        flushDeletesCount = newLucene<AtomicLong>();
        pendingCommitChangeCount = 0;
        similarity = Similarity::getDefault(); // how to normalize
        commitLock  = newInstance<Synchronize>();
        readerFinishedListeners = SetReaderFinishedListener::newInstance();
        anyNonBulkMerges = false;
        maxFieldLength = DEFAULT_MAX_FIELD_LENGTH;
        _keepFullyDeletedSegments = false;
        
        config = boost::static_pointer_cast<IndexWriterConfig>(conf->clone());
        directory = d;
        analyzer = conf->getAnalyzer();
        infoStream = defaultInfoStream;
        writeLockTimeout = conf->getWriteLockTimeout();
        similarity = conf->getSimilarity();
        mergePolicy = conf->getMergePolicy();
        mergeScheduler = conf->getMergeScheduler();
        bufferedDeletes = newLucene<BufferedDeletes>(messageID);
        bufferedDeletes->setInfoStream(infoStream);
        poolReaders = conf->getReaderPooling();
    }
    
    void IndexWriter::initialize()
    {
        readerPool = newLucene<ReaderPool>(shared_from_this());
        mergePolicy->setIndexWriter(shared_from_this());
        flushControl = newLucene<FlushControl>(shared_from_this());
        
        IndexWriterConfig::OpenMode mode = config->getOpenMode();
        bool create;
        if (mode == IndexWriterConfig::CREATE)
            create = true;
        else if (mode == IndexWriterConfig::APPEND)
            create = false;
        else
        {
            // CREATE_OR_APPEND - create only if an index does not exist
            create = !IndexReader::indexExists(directory);
        }

        writeLock = directory->makeLock(WRITE_LOCK_NAME);
        
        if (!writeLock->obtain((int32_t)writeLockTimeout)) // obtain write lock
            boost::throw_exception(LockObtainFailedException(L"Index locked for write: " + writeLock->toString()));
        
        bool success = false;
        LuceneException finally;        
        try
        {
            if (create)
            {
                // Try to read first.  This is to allow create against an index that's currently open for
                // searching.  In this case we write the next segments_N file with no segments
                try
                {
                    segmentInfos->read(directory);
                    segmentInfos->clear();
                } 
                catch (LuceneException&)
                {
                    // Likely this means it's a fresh directory
                }

                // Record that we have a change (zero out all segments) pending
                ++changeCount;
                segmentInfos->changed();
            }
            else
            {
                segmentInfos->read(directory);
                
                IndexCommitPtr commit(config->getIndexCommit());
                if (commit)
                {
                    // Swap out all segments, but, keep metadata in SegmentInfos, like version & generation, to
                    // preserve write-once.  This is important if readers are open against the future commit points.
                    if (commit->getDirectory() != directory)
                        boost::throw_exception(IllegalArgumentException(L"IndexCommit's directory doesn't match my directory"));
                    SegmentInfosPtr oldInfos(newLucene<SegmentInfos>());
                    oldInfos->read(directory, commit->getSegmentsFileName());
                    segmentInfos->replace(oldInfos);
                    ++changeCount;
                    segmentInfos->changed();
                    if (infoStream)
                        message(L"init: loaded commit \"" + commit->getSegmentsFileName() + L"\"");
                }
            }
            
            setRollbackSegmentInfos(segmentInfos);
            
            docWriter = newLucene<DocumentsWriter>(config, directory, shared_from_this(), getCurrentFieldInfos(), bufferedDeletes);
            docWriter->setInfoStream(infoStream);
            docWriter->setMaxFieldLength(maxFieldLength);
            
            // Default deleter (for backwards compatibility) is KeepOnlyLastCommitDeleter
            deleter = newLucene<IndexFileDeleter>(directory, config->getIndexDeletionPolicy(), segmentInfos, infoStream);
            
            if (deleter->startingCommitDeleted)
            {
                // Deletion policy deleted the "head" commit point.  We have to mark ourself as changed so that if we
                // are closed without any further changes we write a new segments_N file.
                ++changeCount;
                segmentInfos->changed();
            }
            
            pushMaxBufferedDocs();
            
            if (infoStream)
                messageState();
            
            success = true;
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        
        if (!success)
        {
            if (infoStream)
                message(L"init: hit exception on init; releasing write lock");
            try
            {
                this->writeLock->release();
            }
            catch (...)
            {
                // don't mask the original exception
            }
            this->writeLock.reset();
        }
        
        finally.throwException();
        
        // must be set after initialise and not in the constructor because we create docWriter in here
        setMaxFieldLength(this->maxFieldLength);
    }
    
    AtomicLongPtr IndexWriter::MESSAGE_ID()
    {
        static AtomicLongPtr _MESSAGE_ID;
        if (!_MESSAGE_ID)
        {
            _MESSAGE_ID = newLucene<AtomicLong>();
            CycleCheck::addStatic(_MESSAGE_ID);
        }
        return _MESSAGE_ID;
    }
    
    int32_t IndexWriter::MAX_TERM_LENGTH()
    {
        static int32_t _MAX_TERM_LENGTH = 0;
        if (_MAX_TERM_LENGTH == 0)
            _MAX_TERM_LENGTH = DocumentsWriter::MAX_TERM_LENGTH;
        return _MAX_TERM_LENGTH;
    }
    
    IndexReaderPtr IndexWriter::getReader()
    {
        return getReader(config->getReaderTermsIndexDivisor(), true);
    }
    
    IndexReaderPtr IndexWriter::getReader(bool applyAllDeletes)
    {
        return getReader(config->getReaderTermsIndexDivisor(), applyAllDeletes);
    }
    
    IndexReaderPtr IndexWriter::getReader(int32_t termInfosIndexDivisor)
    {
        return getReader(termInfosIndexDivisor, true);
    }
    
    IndexReaderPtr IndexWriter::getReader(int32_t termInfosIndexDivisor, bool applyAllDeletes)
    {
        ensureOpen();
        
        int64_t tStart = MiscUtils::currentTimeMillis();
        
        if (infoStream)
            message(L"flush at getReader");
        
        // Do this up front before flushing so that the readers obtained during this flush are pooled, the first time
        // this method is called
        poolReaders = true;

        // Prevent segmentInfos from changing while opening the reader; in theory we could do similar retry logic,
        // just like we do when loading segments_N
        IndexReaderPtr r;
        {
            SyncLock syncLock(this);
            flush(false, applyAllDeletes);
            r = newLucene<ReadOnlyDirectoryReader>(shared_from_this(), segmentInfos, termInfosIndexDivisor, applyAllDeletes);
            if (infoStream)
                message(L"return reader version=" + StringUtils::toString(r->getVersion()) + L" reader=" + r->toString());
        }
        maybeMerge();
        
        if (infoStream)
            message(L"getReader took " + StringUtils::toString(MiscUtils::currentTimeMillis() - tStart) + L" msec");
            
        return r;
    }
    
    SetReaderFinishedListener IndexWriter::getReaderFinishedListeners()
    {
        return readerFinishedListeners;
    }
    
    int32_t IndexWriter::numDeletedDocs(SegmentInfoPtr info)
    {
        SegmentReaderPtr reader(readerPool->getIfExists(info));
        int32_t deletedDocs = 0;
        LuceneException finally;
        try
        {
            deletedDocs = reader ? reader->numDeletedDocs() : info->getDelCount();
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        if (reader)
            readerPool->release(reader);
        finally.throwException();
        return deletedDocs;
    }
    
    void IndexWriter::ensureOpen(bool includePendingClose)
    {
        if (closed || (includePendingClose && closing))
            boost::throw_exception(AlreadyClosedException(L"This IndexWriter is closed"));
    }
    
    void IndexWriter::ensureOpen()
    {
        ensureOpen(true);
    }
    
    void IndexWriter::message(const String& message)
    {
        if (infoStream)
        {
            *infoStream << L"IW " << StringUtils::toString(messageID);
            *infoStream << L" [" << DateTools::timeToString(MiscUtils::currentTimeMillis(), DateTools::RESOLUTION_SECOND);
            *infoStream << L"; " << StringUtils::toString(LuceneThread::currentId()) << L"]: " << message << L"\n";
        }
    }
    
    LogMergePolicyPtr IndexWriter::getLogMergePolicy()
    {
        LogMergePolicyPtr logMergePolicy(boost::static_pointer_cast<LogMergePolicy>(mergePolicy));
        if (logMergePolicy)
            return logMergePolicy;
        boost::throw_exception(IllegalArgumentException(L"This method can only be called when the merge policy is the default LogMergePolicy"));
        return LogMergePolicyPtr();
    }
    
    bool IndexWriter::getUseCompoundFile()
    {
        return getLogMergePolicy()->getUseCompoundFile();
    }
    
    void IndexWriter::setUseCompoundFile(bool value)
    {
        getLogMergePolicy()->setUseCompoundFile(value);
    }
    
    void IndexWriter::setSimilarity(SimilarityPtr similarity)
    {
        ensureOpen();
        this->similarity = similarity;
        docWriter->setSimilarity(similarity);
        // Required so config.getSimilarity returns the right value.
        config->setSimilarity(similarity);
    }

    SimilarityPtr IndexWriter::getSimilarity()
    {
        ensureOpen();
        return similarity;
    }
    
    void IndexWriter::setTermIndexInterval(int32_t interval)
    {
        ensureOpen();
        config->setTermIndexInterval(interval);
    }
    
    int32_t IndexWriter::getTermIndexInterval()
    {
        // We pass false because this method is called by SegmentMerger while we are in the process of closing
        ensureOpen(false);
        return config->getTermIndexInterval();
    }
    
    FieldInfosPtr IndexWriter::getFieldInfos(SegmentInfoPtr info)
    {
        DirectoryPtr cfsDir;
        FieldInfosPtr fieldInfos;
        LuceneException finally;
        try
        {
            if (info->getUseCompoundFile())
                cfsDir = newLucene<CompoundFileReader>(directory, IndexFileNames::segmentFileName(info->name, IndexFileNames::COMPOUND_FILE_EXTENSION()));
            else
                cfsDir = directory;
            fieldInfos = newLucene<FieldInfos>(cfsDir, IndexFileNames::segmentFileName(info->name, IndexFileNames::FIELD_INFOS_EXTENSION()));
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        if (info->getUseCompoundFile() && cfsDir)
            cfsDir->close();
        finally.throwException();
        return fieldInfos;
    }
    
    FieldInfosPtr IndexWriter::getCurrentFieldInfos()
    {
        FieldInfosPtr fieldInfos;
        if (!segmentInfos->empty())
        {
            if (segmentInfos->getFormat() > SegmentInfos::FORMAT_DIAGNOSTICS)
            {
                // Pre-3.1 index.  In this case we sweep all segments, merging their FieldInfos
                fieldInfos = newLucene<FieldInfos>();
                for (int32_t i = 0; i < segmentInfos->size(); ++i)
                {
                    SegmentInfoPtr info(segmentInfos->info(i));
                    FieldInfosPtr segFieldInfos(getFieldInfos(info));
                    int32_t fieldCount = segFieldInfos->size();
                    for (int32_t fieldNumber = 0; fieldNumber < fieldCount; ++fieldNumber)
                        fieldInfos->add(segFieldInfos->fieldInfo(fieldNumber));
                }
            }
            else
            {
                // Already a 3.1 index; just seed the FieldInfos from the last segment
                fieldInfos = getFieldInfos(segmentInfos->info(segmentInfos->size() - 1));
            }
        }
        else
            fieldInfos = newLucene<FieldInfos>();
        return fieldInfos;
    }

    void IndexWriter::setRollbackSegmentInfos(SegmentInfosPtr infos)
    {
        SyncLock syncLock(this);
        rollbackSegmentInfos = boost::static_pointer_cast<SegmentInfos>(infos->clone());
    }
    
    IndexWriterConfigPtr IndexWriter::getConfig()
    {
        return config;
    }
    
    void IndexWriter::setMergePolicy(MergePolicyPtr mp)
    {
        ensureOpen();
        if (!mp)
            boost::throw_exception(NullPointerException(L"MergePolicy must be non-null"));
        
        if (mergePolicy != mp)
            mergePolicy->close();
        mergePolicy = mp;
        mergePolicy->setIndexWriter(shared_from_this());
        pushMaxBufferedDocs();
        if (infoStream)
            message(L"setMergePolicy");
        // Required so config.getMergePolicy returns the right value.
        config->setMergePolicy(mp);
    }
    
    MergePolicyPtr IndexWriter::getMergePolicy()
    {
        ensureOpen();
        return mergePolicy;
    }
    
    void IndexWriter::setMergeScheduler(MergeSchedulerPtr mergeScheduler)
    {
        SyncLock syncLock(this);
        ensureOpen();
        if (!mergeScheduler)
            boost::throw_exception(NullPointerException(L"MergeScheduler must be non-null"));
        if (this->mergeScheduler != mergeScheduler)
        {
            finishMerges(true);
            this->mergeScheduler->close();
        }
        this->mergeScheduler = mergeScheduler;
        if (infoStream)
            message(L"setMergeScheduler");
        // Required so config.getMergeScheduler returns the right value.
        config->setMergeScheduler(mergeScheduler);
    }
    
    MergeSchedulerPtr IndexWriter::getMergeScheduler()
    {
        ensureOpen();
        return mergeScheduler;
    }
    
    void IndexWriter::setMaxMergeDocs(int32_t maxMergeDocs)
    {
        getLogMergePolicy()->setMaxMergeDocs(maxMergeDocs);
    }
    
    int32_t IndexWriter::getMaxMergeDocs()
    {
        return getLogMergePolicy()->getMaxMergeDocs();
    }
    
    void IndexWriter::setMaxFieldLength(int32_t maxFieldLength)
    {
        ensureOpen();
        this->maxFieldLength = maxFieldLength;
        docWriter->setMaxFieldLength(maxFieldLength);
        if (infoStream)
            message(L"setMaxFieldLength " + StringUtils::toString(maxFieldLength));
    }
    
    int32_t IndexWriter::getMaxFieldLength()
    {
        ensureOpen();
        return maxFieldLength;
    }
    
    void IndexWriter::setReaderTermsIndexDivisor(int32_t divisor)
    {
        ensureOpen();
        config->setReaderTermsIndexDivisor(divisor);
        if (infoStream)
            message(L"setReaderTermsIndexDivisor " + StringUtils::toString(divisor));
    }
    
    int32_t IndexWriter::getReaderTermsIndexDivisor()
    {
        ensureOpen();
        return config->getReaderTermsIndexDivisor();
    }
    
    void IndexWriter::setMaxBufferedDocs(int32_t maxBufferedDocs)
    {
        ensureOpen();
        pushMaxBufferedDocs();
        if (infoStream)
            message(L"setMaxBufferedDocs " + StringUtils::toString(maxBufferedDocs));
        // Required so config.getMaxBufferedDocs returns the right value.
        config->setMaxBufferedDocs(maxBufferedDocs);
    }
    
    void IndexWriter::pushMaxBufferedDocs()
    {
        if (config->getMaxBufferedDocs() != DISABLE_AUTO_FLUSH)
        {
            LogDocMergePolicyPtr lmp(boost::dynamic_pointer_cast<LogDocMergePolicy>(mergePolicy));
            if (lmp)
            {
                int32_t maxBufferedDocs = config->getMaxBufferedDocs();
                if (lmp->getMinMergeDocs() != maxBufferedDocs)
                {
                    if (infoStream)
                        message(L"now push maxBufferedDocs " + StringUtils::toString(maxBufferedDocs) + L" to LogDocMergePolicy");
                    lmp->setMinMergeDocs(maxBufferedDocs);
                }
            }
        }
    }
    
    int32_t IndexWriter::getMaxBufferedDocs()
    {
        ensureOpen();
        return config->getMaxBufferedDocs();
    }
    
    void IndexWriter::setRAMBufferSizeMB(double mb)
    {
        if (infoStream)
            message(L"setRAMBufferSizeMB " + StringUtils::toString(mb));
        // Required so config.getRAMBufferSizeMB returns the right value.
        config->setRAMBufferSizeMB(mb);
    }
    
    double IndexWriter::getRAMBufferSizeMB()
    {
        return config->getRAMBufferSizeMB();
    }
    
    void IndexWriter::setMaxBufferedDeleteTerms(int32_t maxBufferedDeleteTerms)
    {
        ensureOpen();
        if (infoStream)
            message(L"setMaxBufferedDeleteTerms " + StringUtils::toString(maxBufferedDeleteTerms));
        // Required so config.getMaxBufferedDeleteTerms returns the right value.
        config->setMaxBufferedDeleteTerms(maxBufferedDeleteTerms);
    }
    
    int32_t IndexWriter::getMaxBufferedDeleteTerms()
    {
        ensureOpen();
        return config->getMaxBufferedDeleteTerms();
    }
    
    void IndexWriter::setMergeFactor(int32_t mergeFactor)
    {
        getLogMergePolicy()->setMergeFactor(mergeFactor);
    }
    
    int32_t IndexWriter::getMergeFactor()
    {
        return getLogMergePolicy()->getMergeFactor();
    }
    
    void IndexWriter::setDefaultInfoStream(InfoStreamPtr infoStream)
    {
        IndexWriter::defaultInfoStream = infoStream;
    }
    
    InfoStreamPtr IndexWriter::getDefaultInfoStream()
    {
        return IndexWriter::defaultInfoStream;
    }
    
    void IndexWriter::setInfoStream(InfoStreamPtr infoStream)
    {
        ensureOpen();
        this->infoStream = infoStream;
        docWriter->setInfoStream(infoStream);
        deleter->setInfoStream(infoStream);
        bufferedDeletes->setInfoStream(infoStream);
        if (infoStream)
            messageState();
    }
    
    void IndexWriter::messageState()
    {
        message(L"\ndir=" + directory->toString() + L"\n" +
                L"index=" + segString() + L"\n" +
                L"version=" + StringUtils::toString(Constants::LUCENE_VERSION) + L"\n" +
                config->toString());
    }
    
    InfoStreamPtr IndexWriter::getInfoStream()
    {
        ensureOpen();
        return infoStream;
    }
    
    bool IndexWriter::verbose()
    {
        return infoStream;
    }
    
    void IndexWriter::setWriteLockTimeout(int64_t writeLockTimeout)
    {
        ensureOpen();
        this->writeLockTimeout = writeLockTimeout;
        // Required so config.getWriteLockTimeout returns the right value.
        config->setWriteLockTimeout(writeLockTimeout);
    }
    
    int64_t IndexWriter::getWriteLockTimeout()
    {
        ensureOpen();
        return writeLockTimeout;
    }
    
    void IndexWriter::setDefaultWriteLockTimeout(int64_t writeLockTimeout)
    {
        IndexWriterConfig::setDefaultWriteLockTimeout(writeLockTimeout);
    }
    
    int64_t IndexWriter::getDefaultWriteLockTimeout()
    {
        return IndexWriterConfig::getDefaultWriteLockTimeout();
    }
    
    void IndexWriter::close()
    {
        close(true);
    }
    
    void IndexWriter::close(bool waitForMerges)
    {
        // Ensure that only one thread actually gets to do the closing
        if (shouldClose())
        {
            // If any methods have hit std::bad_alloc, then abort on close, in case the internal state of IndexWriter
            // or DocumentsWriter is corrupt
            if (hitOOM)
                rollbackInternal();
            else
                closeInternal(waitForMerges);
        }
    }

    bool IndexWriter::shouldClose()
    {
        SyncLock syncLock(this);
        while (true)
        {
            if (!closed)
            {
                if (!closing)
                {
                    closing = true;
                    return true;
                }
                else
                {
                    // Another thread is presently trying to close; wait until it finishes one way (closes
                    // successfully) or another (fails to close)
                    doWait();
                }
            }
            else
                return false;
        }
    }
    
    void IndexWriter::closeInternal(bool waitForMerges)
    {
        LuceneException finally;
        try
        {
            if (infoStream)
                message(L"now flush at close waitForMerges=" + StringUtils::toString(waitForMerges));
                
            docWriter->close();
            
            // Only allow a new merge to be triggered if we are going to wait for merges
            if (!hitOOM)
                flush(waitForMerges, true);
            
            // Give merge scheduler last chance to run, in case any pending merges are waiting
            if (waitForMerges)
                mergeScheduler->merge(shared_from_this());
            
            mergePolicy->close();
            
            {
                SyncLock syncLock(this);
                finishMerges(waitForMerges);
                stopMerges = true;
            }
            
            mergeScheduler->close();
            
            if (infoStream)
                message(L"now call final commit()");
            
            if (!hitOOM)
                commitInternal(MapStringString());
            
            if (infoStream)
                message(L"at close: " + segString());
            
            {
                SyncLock syncLock(this);
                readerPool->close();
                docWriter.reset();
                deleter->close();
            }
            
            if (writeLock)
            {
                writeLock->release(); // release write lock
                writeLock.reset();
            }
            
            {
                SyncLock syncLock(this);
                closed = true;
            }
        }
        catch (std::bad_alloc& oom)
        {
            finally = handleOOM(oom, L"closeInternal");
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        {
            SyncLock syncLock(this);
            closing = false;
            notifyAll();
            if (!closed)
            {
                if (infoStream)
                    message(L"hit exception while closing");
            }
        }
        finally.throwException();
    }
    
    DirectoryPtr IndexWriter::getDirectory()
    {
        ensureOpen(false); // Pass false because the flush during closing calls getDirectory
        return directory;
    }
    
    AnalyzerPtr IndexWriter::getAnalyzer()
    {
        ensureOpen();
        return analyzer;
    }
    
    int32_t IndexWriter::maxDoc()
    {
        SyncLock syncLock(this);
        int32_t count = docWriter ? docWriter->getNumDocs() : 0;
        for (int32_t i = 0; i < segmentInfos->size(); ++i)
            count += segmentInfos->info(i)->docCount;
        return count;
    }
    
    int32_t IndexWriter::numDocs()
    {
        SyncLock syncLock(this);
        int32_t count = docWriter ? docWriter->getNumDocs() : 0;
        for (int32_t i = 0; i < segmentInfos->size(); ++i)
        {
            SegmentInfoPtr info(segmentInfos->info(i));
            count += info->docCount - numDeletedDocs(info);
        }
        return count;
    }
    
    bool IndexWriter::hasDeletions()
    {
        SyncLock syncLock(this);
        ensureOpen();
        if (bufferedDeletes->any())
            return true;
        if (docWriter->anyDeletions())
            return true;
        for (int32_t i = 0; i < segmentInfos->size(); ++i)
        {
            if (segmentInfos->info(i)->hasDeletions())
                return true;
        }
        return false;
    }
    
    void IndexWriter::addDocument(DocumentPtr doc)
    {
        addDocument(doc, analyzer);
    }
    
    void IndexWriter::addDocument(DocumentPtr doc, AnalyzerPtr analyzer)
    {
        ensureOpen();
        bool doFlush = false;
        bool success = false;
        try
        {
            LuceneException finally;
            try
            {
                doFlush = docWriter->updateDocument(doc, analyzer, TermPtr());
                success = true;
            }
            catch (LuceneException& e)
            {
                finally = e;
            }
            if (!success && infoStream)
                message(L"hit exception adding document");
            finally.throwException();
            if (doFlush)
                flush(true, false);
        }
        catch (std::bad_alloc& oom)
        {
            boost::throw_exception(handleOOM(oom, L"addDocument"));
        }
    }
    
    void IndexWriter::deleteDocuments(TermPtr term)
    {
        ensureOpen();
        try
        {
            if (docWriter->deleteTerm(term, false))
                flush(true, false);
        }
        catch (std::bad_alloc& oom)
        {
            boost::throw_exception(handleOOM(oom, L"deleteDocuments(Term)"));
        }
    }
    
    void IndexWriter::deleteDocuments(Collection<TermPtr> terms)
    {
        ensureOpen();
        try
        {
            if (docWriter->deleteTerms(terms))
                flush(true, false);
        }
        catch (std::bad_alloc& oom)
        {
            boost::throw_exception(handleOOM(oom, L"deleteDocuments(Collection<TermPtr>)"));
        }
    }
    
    void IndexWriter::deleteDocuments(QueryPtr query)
    {
        ensureOpen();
        try
        {
            if (docWriter->deleteQuery(query))
                flush(true, false);
        }
        catch (std::bad_alloc& oom)
        {
            boost::throw_exception(handleOOM(oom, L"deleteDocuments(Query)"));
        }
    }
    
    void IndexWriter::deleteDocuments(Collection<QueryPtr> queries)
    {
        ensureOpen();
        try
        {
            if (docWriter->deleteQueries(queries))
                flush(true, false);
        }
        catch (std::bad_alloc& oom)
        {
            boost::throw_exception(handleOOM(oom, L"deleteDocuments(Collection<QueryPtr>)"));
        }
    }
    
    void IndexWriter::updateDocument(TermPtr term, DocumentPtr doc)
    {
        ensureOpen();
        updateDocument(term, doc, getAnalyzer());
    }
    
    void IndexWriter::updateDocument(TermPtr term, DocumentPtr doc, AnalyzerPtr analyzer)
    {
        ensureOpen();
        try
        {
            bool doFlush = false;
            bool success = false;
            LuceneException finally;
            try
            {
                doFlush = docWriter->updateDocument(doc, analyzer, term);
                success = true;
            }
            catch (LuceneException& e)
            {
                finally = e;
            }
            if (!success && infoStream)
                message(L"hit exception updating document");
            finally.throwException();
            if (doFlush)
                flush(true, false);
        }
        catch (std::bad_alloc& oom)
        {
            boost::throw_exception(handleOOM(oom, L"updateDocument"));
        }
    }
    
    int32_t IndexWriter::getSegmentCount()
    {
        SyncLock syncLock(this);
        return segmentInfos->size();
    }
    
    int32_t IndexWriter::getNumBufferedDocuments()
    {
        SyncLock syncLock(this);
        return docWriter->getNumDocs();
    }
    
    int32_t IndexWriter::getDocCount(int32_t i)
    {
        SyncLock syncLock(this);
        return (i >= 0 && i < segmentInfos->size()) ? segmentInfos->info(i)->docCount : -1;
    }
    
    int32_t IndexWriter::getFlushCount()
    {
        return flushCount->get();
    }
    
    int32_t IndexWriter::getFlushDeletesCount()
    {
        return flushDeletesCount->get();
    }
    
    String IndexWriter::newSegmentName()
    {
        // Cannot synchronize on IndexWriter because that causes deadlock
        SyncLock segmentLock(segmentInfos);
    
        // Important to increment changeCount so that the segmentInfos is written on close.  
        // Otherwise we could close, re-open and re-return the same segment name that was 
        // previously returned which can cause problems at least with ConcurrentMergeScheduler.
        ++changeCount;
        segmentInfos->changed();
        return L"_" + StringUtils::toString(segmentInfos->counter++, StringUtils::CHARACTER_MAX_RADIX);
    }
    
    void IndexWriter::optimize()
    {
        optimize(true);
    }
    
    void IndexWriter::optimize(int32_t maxNumSegments)
    {
        optimize(maxNumSegments, true);
    }
    
    void IndexWriter::optimize(bool doWait)
    {
        optimize(1, doWait);
    }
    
    void IndexWriter::optimize(int32_t maxNumSegments, bool doWait)
    {
        ensureOpen();
        
        if (maxNumSegments < 1)
            boost::throw_exception(IllegalArgumentException(L"maxNumSegments must be >= 1; got " + StringUtils::toString(maxNumSegments)));
        
        if (infoStream)
        {
            message(L"optimize: index now " + segString());
            message(L"now flush at optimize");
        }
        
        flush(true, true);
        
        {
            SyncLock syncLock(this);
            
            resetMergeExceptions();
            segmentsToOptimize.clear();
            optimizeMaxNumSegments = maxNumSegments;
            int32_t numSegments = segmentInfos->size();
            for (int32_t i = 0; i < numSegments; ++i)
                segmentsToOptimize.add(segmentInfos->info(i));
            
            // Now mark all pending & running merges as optimize merge
            for (Collection<OneMergePtr>::iterator merge = pendingMerges.begin(); merge != pendingMerges.end(); ++merge)
            {
                (*merge)->optimize = true;
                (*merge)->maxNumSegmentsOptimize = maxNumSegments;
            }
            
            for (SetOneMerge::iterator merge = runningMerges.begin(); merge != runningMerges.end(); ++merge)
            {
                (*merge)->optimize = true;
                (*merge)->maxNumSegmentsOptimize = maxNumSegments;
            }
        }
        
        maybeMerge(maxNumSegments, true);
        
        if (doWait)
        {
            {
                SyncLock syncLock(this);
                while (true)
                {
                    if (hitOOM)
                        boost::throw_exception(IllegalStateException(L"this writer hit an OutOfMemoryError; cannot complete optimize"));
                    
                    if (!mergeExceptions.empty())
                    {
                        // Forward any exceptions in background merge threads to the current thread
                        for (Collection<OneMergePtr>::iterator merge = mergeExceptions.begin(); merge != mergeExceptions.end(); ++merge)
                        {
                            if ((*merge)->optimize)
                            {
                                LuceneException err = (*merge)->getException();
                                if (!err.isNull())
                                    boost::throw_exception(IOException(L"background merge hit exception: " + (*merge)->segString(directory)));
                            }
                        }
                    }
                    
                    if (optimizeMergesPending())
                        IndexWriter::doWait();
                    else
                        break;
                }
            }
            
            // If close is called while we are still running, throw an exception so the calling thread will know the 
            // optimize did not complete
            ensureOpen();
        }
        
        // NOTE: in the ConcurrentMergeScheduler case, when doWait is false, we can return immediately while background 
        // threads accomplish the optimization
    }
    
    bool IndexWriter::optimizeMergesPending()
    {
        SyncLock syncLock(this);
        
        for (Collection<OneMergePtr>::iterator merge = pendingMerges.begin(); merge != pendingMerges.end(); ++merge)
        {
            if ((*merge)->optimize)
                return true;
        }
        
        for (SetOneMerge::iterator merge = runningMerges.begin(); merge != runningMerges.end(); ++merge)
        {
            if ((*merge)->optimize)
                return true;
        }
        
        return false;
    }
    
    void IndexWriter::expungeDeletes(bool doWait)
    {
        ensureOpen();
        
        if (infoStream)
            message(L"expungeDeletes: index now " + segString());
        
        MergeSpecificationPtr spec;
        
        {
            SyncLock syncLock(this);
            spec = mergePolicy->findMergesToExpungeDeletes(segmentInfos);
            for (Collection<OneMergePtr>::iterator merge = spec->merges.begin(); merge != spec->merges.end(); ++merge)
                registerMerge(*merge);
        }
        
        mergeScheduler->merge(shared_from_this());
        
        if (doWait)
        {
            {
                SyncLock syncLock(this);
                bool running = true;
                while (running)
                {
                    if (hitOOM)
                        boost::throw_exception(IllegalStateException(L"this writer hit an OutOfMemoryError; cannot complete expungeDeletes"));
                    
                    // Check each merge that MergePolicy asked us to do, to see if any of them are still running and
                    // if any of them have hit an exception.
                    running = false;
                    for (Collection<OneMergePtr>::iterator merge = spec->merges.begin(); merge != spec->merges.end(); ++merge)
                    {
                        if (pendingMerges.contains(*merge) || runningMerges.contains(*merge))
                            running = true;
                        LuceneException err = (*merge)->getException();
                        if (!err.isNull())
                            boost::throw_exception(IOException(L"background merge hit exception: " + (*merge)->segString(directory)));
                    }
                    
                    // If any of our merges are still running, wait
                    if (running)
                        IndexWriter::doWait();
                }
            }
        }
        
        // NOTE: in the ConcurrentMergeScheduler case, when doWait is false, we can return immediately while background 
        // threads accomplish the optimization
    }
    
    void IndexWriter::expungeDeletes()
    {
        expungeDeletes(true);
    }
    
    void IndexWriter::maybeMerge()
    {
        maybeMerge(false);
    }
    
    void IndexWriter::maybeMerge(bool optimize)
    {
        maybeMerge(1, optimize);
    }
    
    void IndexWriter::maybeMerge(int32_t maxNumSegmentsOptimize, bool optimize)
    {
        updatePendingMerges(maxNumSegmentsOptimize, optimize);
        mergeScheduler->merge(shared_from_this());
    }
    
    void IndexWriter::updatePendingMerges(int32_t maxNumSegmentsOptimize, bool optimize)
    {
        SyncLock syncLock(this);
        BOOST_ASSERT(!optimize || maxNumSegmentsOptimize > 0);
        
        if (stopMerges)
            return;
        
        // Do not start new merges if we've hit std::bad_alloc
        if (hitOOM)
            return;
        
        MergeSpecificationPtr spec;
        
        if (optimize)
        {
            spec = mergePolicy->findMergesForOptimize(segmentInfos, maxNumSegmentsOptimize, segmentsToOptimize);
            
            if (spec)
            {
                for (Collection<OneMergePtr>::iterator merge = spec->merges.begin(); merge != spec->merges.end(); ++merge)
                {
                    (*merge)->optimize = true;
                    (*merge)->maxNumSegmentsOptimize = maxNumSegmentsOptimize;
                }
            }
        }
        else
            spec = mergePolicy->findMerges(segmentInfos);
        
        if (spec)
        {
            for (Collection<OneMergePtr>::iterator merge = spec->merges.begin(); merge != spec->merges.end(); ++merge)
                registerMerge(*merge);
        }
    }
    
    OneMergePtr IndexWriter::getNextMerge()
    {
        SyncLock syncLock(this);
        if (pendingMerges.empty())
            return OneMergePtr();
        else
        {
            // Advance the merge from pending to running
            OneMergePtr merge(pendingMerges.removeFirst());
            runningMerges.add(merge);
            return merge;
        }
    }

    void IndexWriter::rollback()
    {
        ensureOpen();
        
        // Ensure that only one thread actually gets to do the closing
        if (shouldClose())
            rollbackInternal();
    }
    
    void IndexWriter::rollbackInternal()
    {
        bool success = false;
        
        if (infoStream)
            message(L"rollback");
        
        LuceneException finally;
        try
        {
            {
                SyncLock syncLock(this);
                finishMerges(false);
                stopMerges = true;
            }
            
            if (infoStream)
                message(L"rollback: done finish merges");
            
            // Must pre-close these two, in case they increment changeCount so that we can then set it to false before 
            // calling closeInternal
            mergePolicy->close();
            mergeScheduler->close();
            
            bufferedDeletes->clear();

            {
                SyncLock syncLock(this);
                
                if (pendingCommit)
                {
                    pendingCommit->rollbackCommit(directory);
                    deleter->decRef(pendingCommit);
                    pendingCommit.reset();
                    notifyAll();
                }

                // Keep the same segmentInfos instance but replace all of its SegmentInfo instances.  This is so the next 
                // attempt to commit using this instance of IndexWriter will always write to a new generation ("write once").
                segmentInfos->clear();
                segmentInfos->addAll(rollbackSegmentInfos);
                
                docWriter->abort();
                
                bool test = testPoint(L"rollback before checkpoint");
                BOOST_ASSERT(test);
                
                // Ask deleter to locate unreferenced files & remove them
                deleter->checkpoint(segmentInfos, false);
                deleter->refresh();
            }
            
            // Don't bother saving any changes in our segmentInfos
            readerPool->clear(SegmentInfosPtr());
            
            lastCommitChangeCount = changeCount;
            
            success = true;
        }
        catch (std::bad_alloc& oom)
        {
            finally = handleOOM(oom, L"rollbackInternal");
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        {
            SyncLock syncLock(this);
            
            if (!success)
            {
                closing = false;
                notifyAll();
                if (infoStream)
                    message(L"hit exception during rollback");
            }
        }
        finally.throwException();
        
        closeInternal(false);
    }
    
    void IndexWriter::deleteAll()
    {
        SyncLock syncLock(this);
        bool success = false;
        LuceneException finally;
        try
        {
            // Abort any running merges
            finishMerges(false);
            
            // Remove any buffered docs
            docWriter->abort();
            
            // Remove all segments
            segmentInfos->clear();
            
            // Ask deleter to locate unreferenced files & remove them
            deleter->checkpoint(segmentInfos, false);
            deleter->refresh();
            
            // Don't bother saving any changes in our segmentInfos
            readerPool->clear(SegmentInfosPtr());
            
            // Mark that the index has changed
            ++changeCount;
            segmentInfos->changed();
            
            success = true;
        }
        catch (std::bad_alloc& oom)
        {
            finally = handleOOM(oom, L"deleteAll");
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        
        if (!success && infoStream)
            message(L"hit exception during deleteAll");
        
        finally.throwException();
    }
    
    void IndexWriter::finishMerges(bool waitForMerges)
    {
        SyncLock syncLock(this);
        if (!waitForMerges)
        {
            stopMerges = true;
            
            // Abort all pending and running merges
            for (Collection<OneMergePtr>::iterator merge = pendingMerges.begin(); merge != pendingMerges.end(); ++merge)
            {
                if (infoStream)
                    message(L"now abort pending merge " + (*merge)->segString(directory));
                (*merge)->abort();
                mergeFinish(*merge);
            }
            pendingMerges.clear();
            
            for (SetOneMerge::iterator merge = runningMerges.begin(); merge != runningMerges.end(); ++merge)
            {
                if (infoStream)
                    message(L"now abort running merge " + (*merge)->segString(directory));
                (*merge)->abort();
            }
            
            // These merges periodically check whether they have been aborted, and stop if so.  We wait here to make 
            // sure they all stop.  It should not take very long because the merge threads periodically check if they 
            // are aborted.
            while (!runningMerges.empty())
            {
                if (infoStream)
                    message(L"now wait for " + StringUtils::toString(runningMerges.size()) + L" running merge to abort");
                doWait();
            }
            
            stopMerges = false;
            notifyAll();
            
            BOOST_ASSERT(mergingSegments.empty());
            
            if (infoStream)
                message(L"all running merges have aborted");
        }
        else
        {
            // waitForMerges() will ensure any running addIndexes finishes.   It's fine if a new one attempts to start 
            // because from our caller above the call will see that we are in the process of closing, and will throw 
            // an AlreadyClosed exception.
            IndexWriter::waitForMerges();
        }
    }
    
    void IndexWriter::waitForMerges()
    {
        SyncLock syncLock(this);
        if (infoStream)
            message(L"waitForMerges");
            
        while (!pendingMerges.empty() || !runningMerges.empty())
            doWait();
        
        // sanity check
        BOOST_ASSERT(mergingSegments.empty());
        
        if (infoStream)
            message(L"waitForMerges done");
    }
    
    void IndexWriter::checkpoint()
    {
        SyncLock syncLock(this);
        ++changeCount;
        segmentInfos->changed();
        deleter->checkpoint(segmentInfos, false);
    }
    
    void IndexWriter::resetMergeExceptions()
    {
        SyncLock syncLock(this);
        mergeExceptions.clear();
        ++mergeGen;
    }
    
    void IndexWriter::noDupDirs(Collection<DirectoryPtr> dirs)
    {
        Collection<DirectoryPtr> dups(Collection<DirectoryPtr>::newInstance());
        for (Collection<DirectoryPtr>::iterator dir = dirs.begin(); dir != dirs.end(); ++dir)
        {
            if (dups.contains(*dir))
                boost::throw_exception(IllegalArgumentException(L"Directory " + (*dir)->getLockID() + L" appears more than once"));
            if (*dir == directory)
                boost::throw_exception(IllegalArgumentException(L"Cannot add directory to itself"));
            dups.add(*dir);
        }
    }
    
    void IndexWriter::addIndexesNoOptimize(Collection<DirectoryPtr> dirs)
    {
        addIndexes(dirs);
    }
    
    void IndexWriter::addIndexes(Collection<IndexReaderPtr> readers)
    {
        ensureOpen();

        LuceneException finally;
        try
        {
            String mergedName(newSegmentName());
            SegmentMergerPtr merger(newLucene<SegmentMerger>(directory, config->getTermIndexInterval(), mergedName, 
                                    OneMergePtr(), payloadProcessorProvider, 
                                    boost::static_pointer_cast<FieldInfos>(docWriter->getFieldInfos()->clone())));

            for (Collection<IndexReaderPtr>::iterator reader = readers.begin(); reader != readers.end(); ++reader) // add new indexes
                merger->add(*reader);

            int32_t docCount = merger->merge();

            SegmentInfoPtr info(newLucene<SegmentInfo>(mergedName, docCount, directory, false, true, 
                                                       merger->fieldInfos()->hasProx(), merger->fieldInfos()->hasVectors()));
            setDiagnostics(info, L"addIndexes(IndexReader...)");

            bool useCompoundFile;
            {
                SyncLock syncLock(this); // Guard segmentInfos
                useCompoundFile = mergePolicy->useCompoundFile(segmentInfos, info);
            }

            // Now create the compound file if needed
            if (useCompoundFile)
            {
                merger->createCompoundFile(mergedName + L".cfs", info);

                // delete new non cfs files directly: they were never registered with IFD
                deleter->deleteNewFiles(info->files());
                info->setUseCompoundFile(true);
            }

            // Register the new segment
            {
                SyncLock syncLock(this);
                segmentInfos->add(info);
                checkpoint();
            }
        }
        catch (std::bad_alloc& oom)
        {
            finally = handleOOM(oom, L"addIndexes(Collection<IndexReaderPtr>)");
        }
        finally.throwException();
    }
    
    void IndexWriter::addIndexes(Collection<DirectoryPtr> dirs)
    {
        ensureOpen();

        noDupDirs(dirs);

        LuceneException finally;
        try
        {
            if (infoStream)
                message(L"flush at addIndexes(Directory...)");
            flush(false, true);

            int32_t docCount = 0;
            Collection<SegmentInfoPtr> infos(Collection<SegmentInfoPtr>::newInstance());
            for (Collection<DirectoryPtr>::iterator dir = dirs.begin(); dir != dirs.end(); ++dir)
            {
                if (infoStream)
                    message(L"addIndexes: process directory " + (*dir)->toString());
                SegmentInfosPtr sis(newLucene<SegmentInfos>()); // read infos from dir
                sis->read(*dir);
                HashSet<String> dsFilesCopied(HashSet<String>::newInstance());
                MapStringString dsNames(MapStringString::newInstance());
                
                for (int32_t i = 0; i < sis->size(); ++i)
                {
                    SegmentInfoPtr info(sis->info(i));
                    BOOST_ASSERT(!infos.contains(info));

                    docCount += info->docCount;
                    String newSegName(newSegmentName());
                    String dsName(info->getDocStoreSegment());

                    if (infoStream)
                    {
                        message(L"addIndexes: process segment origName=" + info->name + 
                                L" newName=" + newSegName + 
                                L" dsName=" + dsName + 
                                L" info=" + info->toString());
                    }

                    // Determine if the doc store of this segment needs to be copied. It's only relevant for segments 
                    // who share doc store with others, because the DS might have been copied already, in which case 
                    // we just want to update the DS name of this SegmentInfo. 
                    // NOTE: pre-3x segments include a null DSName if they don't share doc store. So the following 
                    // code ensures we don't accidentally insert an empty string to the map.
                    String newDsName;
                    if (!dsName.empty())
                    {
                        if (dsNames.contains(dsName))
                            newDsName = dsNames.get(dsName);
                        else
                        {
                            dsNames.put(dsName, newSegName);
                            newDsName = newSegName;
                        }
                    }
                    else
                        newDsName = newSegName;

                    // Copy the segment files
                    HashSet<String> files(info->files());
                    for (HashSet<String>::iterator file = files.begin(); file != files.end(); ++file)
                    {
                        String newFileName;
                        if (IndexFileNames::isDocStoreFile(*file))
                        {
                            newFileName = newDsName + IndexFileNames::stripSegmentName(*file);
                            if (dsFilesCopied.contains(newFileName))
                                continue;
                            dsFilesCopied.add(newFileName);
                        }
                        else
                            newFileName = newSegName + IndexFileNames::stripSegmentName(*file);
                        BOOST_ASSERT(!directory->fileExists(newFileName));
                        (*dir)->copy(directory, *file, newFileName);
                    }

                    // Update SI appropriately
                    info->setDocStore(info->getDocStoreOffset(), newDsName, info->getDocStoreIsCompoundFile());
                    info->dir = directory;
                    info->name = newSegName;

                    infos.add(info);
                }
            }

            {
                SyncLock syncLock(this);
                ensureOpen();
                segmentInfos->addAll(infos);
                checkpoint();
            }
        }
        catch (std::bad_alloc& oom)
        {
            finally = handleOOM(oom, L"addIndexes(Collection<DirectoryPtr>)");
        }
        finally.throwException();
    }
    
    void IndexWriter::doAfterFlush()
    {
        // override
    }
    
    void IndexWriter::doBeforeFlush()
    {
        // override
    }    
    
    void IndexWriter::prepareCommit()
    {
        ensureOpen();
        prepareCommit(MapStringString());
    }
    
    void IndexWriter::prepareCommit(MapStringString commitUserData)
    {
        if (hitOOM)
            boost::throw_exception(IllegalStateException(L"this writer hit an OutOfMemoryError; cannot commit"));
        
        if (pendingCommit)
            boost::throw_exception(IllegalStateException(L"prepareCommit was already called with no corresponding call to commit"));

        if (infoStream)
            message(L"prepareCommit: flush");
        
        flush(true, true);
        
        startCommit(commitUserData);
    }
    
    void IndexWriter::commit()
    {
        commit(MapStringString());
    }
    
    void IndexWriter::commit(MapStringString commitUserData)
    {
        ensureOpen();
        commitInternal(commitUserData);
    }
        
    void IndexWriter::commitInternal(MapStringString commitUserData)
    {    
        if (infoStream)
            message(L"commit: start");
        
        {
            SyncLock messageLock(commitLock);
            
            if (infoStream)
                message(L"commit: enter lock");
            
            if (!pendingCommit)
            {
                if (infoStream)
                    message(L"commit: now prepare");
                prepareCommit(commitUserData);
            }
            else if (infoStream) 
                message(L"commit: already prepared");
            
            finishCommit();
        }
    }
    
    void IndexWriter::finishCommit()
    {
        SyncLock syncLock(this);
        if (pendingCommit)
        {
            LuceneException finally;
            try
            {
                if (infoStream)
                    message(L"commit: pendingCommit != null");
                pendingCommit->finishCommit(directory);
                if (infoStream)
                    message(L"commit: wrote segments file \"" + pendingCommit->getCurrentSegmentFileName() + L"\"");
                lastCommitChangeCount = pendingCommitChangeCount;
                segmentInfos->updateGeneration(pendingCommit);
                segmentInfos->setUserData(pendingCommit->getUserData());
                setRollbackSegmentInfos(pendingCommit);
                deleter->checkpoint(pendingCommit, true);
            }
            catch (LuceneException& e)
            {
                finally = e;
            }
            
            // Matches the incRef done in startCommit
            deleter->decRef(pendingCommit);
            pendingCommit.reset();
            notifyAll();
            finally.throwException();
        }
        else if (infoStream) 
            message(L"commit: pendingCommit == null; skip");
        
        if (infoStream)
            message(L"commit: done");
    }
    
    void IndexWriter::flush(bool triggerMerge, bool flushDocStores, bool flushDeletes)
    {
        flush(triggerMerge, flushDeletes);
    }
    
    void IndexWriter::flush(bool triggerMerge, bool applyAllDeletes)
    {
        // NOTE: this method cannot be sync'd because maybeMerge() in turn calls mergeScheduler.merge which
        // in turn can take a long time to run and we don't want to hold the lock for that.  In the case of
        // ConcurrentMergeScheduler this can lead to deadlock when it stalls due to too many running merges.

        // We can be called during close, when closing==true, so we must pass false to ensureOpen:
        ensureOpen(false);
        if (doFlush(applyAllDeletes) && triggerMerge)
            maybeMerge();
    }
    
    bool IndexWriter::doFlush(bool applyAllDeletes)
    {
        TestScope testScope(L"IndexWriter", L"doFlush");
        SyncLock syncLock(this);
        if (hitOOM) 
            boost::throw_exception(IllegalStateException(L"this writer hit an OutOfMemoryError; cannot flush"));

        doBeforeFlush();

        BOOST_ASSERT(testPoint(L"startDoFlush"));

        // We may be flushing because it was triggered by doc count, del count, ram usage (in which case flush
        // pending is already set), or we may be flushing due to external event eg getReader or commit is 
        // called (in which case we now set it, and this will pause all threads)
        flushControl->setFlushPendingNoWait(L"explicit flush");

        bool success = false;
        bool flushSuccess = false;
        
        LuceneException finally;
        try
        {
            if (infoStream)
            {
                message(L"  start flush: applyAllDeletes=" + StringUtils::toString(applyAllDeletes));
                message(L"  index before flush " + segString());
            }

            SegmentInfoPtr newSegment(docWriter->flush(shared_from_this(), deleter, mergePolicy, segmentInfos));
            if (newSegment)
            {
                setDiagnostics(newSegment, L"flush");
                segmentInfos->add(newSegment);
                checkpoint();
            }

            if (!applyAllDeletes)
            {
                // If deletes alone are consuming > 1/2 our RAM buffer, force them all to apply now. This is to
                // prevent too-frequent flushing of a long tail of tiny segments
                if (flushControl->getFlushDeletes() || (config->getRAMBufferSizeMB() != IndexWriterConfig::DISABLE_AUTO_FLUSH &&
                    bufferedDeletes->bytesUsed() > (1024 * 1024 * config->getRAMBufferSizeMB() / 2)))
                {
                    applyAllDeletes = true;
                    if (infoStream)
                    {
                        message(L"force apply deletes bytesUsed=" + StringUtils::toString(bufferedDeletes->bytesUsed()) + 
                                L" vs ramBuffer=" + StringUtils::toString(1024 * 1024 * config->getRAMBufferSizeMB()));
                    }
                }
            }

            if (applyAllDeletes)
            {
                if (infoStream)
                    message(L"apply all deletes during flush");
                flushDeletesCount->incrementAndGet();
                if (bufferedDeletes->applyDeletes(readerPool, segmentInfos, segmentInfos))
                    checkpoint();
                flushControl->clearDeletes();
            }
            else if (infoStream)
            {
                message(L"don't apply deletes now delTermCount=" + StringUtils::toString(bufferedDeletes->numTerms()) + 
                        L" bytesUsed=" + StringUtils::toString(bufferedDeletes->bytesUsed()));
            }

            doAfterFlush();
            flushCount->incrementAndGet();

            success = true;
            flushSuccess = newSegment;
        }
        catch (std::bad_alloc& oom)
        {
            finally = handleOOM(oom, L"doFlush");
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        
        flushControl->clearFlushPending();
        if (!success && infoStream)
            message(L"hit exception during flush");
        
        finally.throwException();
        
        return flushSuccess;
    }
    
    int64_t IndexWriter::ramSizeInBytes()
    {
        ensureOpen();
        return docWriter->bytesUsed() + bufferedDeletes->bytesUsed();
    }
    
    int32_t IndexWriter::numRamDocs()
    {
        SyncLock syncLock(this);
        ensureOpen();
        return docWriter->getNumDocs();
    }
    
    int32_t IndexWriter::ensureContiguousMerge(OneMergePtr merge)
    {
        int32_t first = segmentInfos->find(merge->segments->info(0));
        if (first == -1)
            boost::throw_exception(MergeException(L"Could not find segment " + merge->segments->info(0)->name + L" in current index " + segString()));
        
        int32_t numSegments = segmentInfos->size();
        int32_t numSegmentsToMerge = merge->segments->size();
        
        for (int32_t i = 0; i < numSegmentsToMerge; ++i)
        {
            SegmentInfoPtr info(merge->segments->info(i));
            
            if (first + i >= numSegments || !segmentInfos->info(first + i)->equals(info))
            {
                if (!segmentInfos->contains(info))
                    boost::throw_exception(MergeException(L"MergePolicy selected a segment (" + info->name + L") that is not in the current index " + segString()));
                else
                    boost::throw_exception(MergeException(L"MergePolicy selected non-contiguous segments to merge (" + merge->segString(directory) + L" vs " + segString() + L"), which IndexWriter (currently) cannot handle"));
            }
        }
        
        return first;
    }
    
    void IndexWriter::commitMergedDeletes(OneMergePtr merge, SegmentReaderPtr mergedReader)
    {
        SyncLock syncLock(this);
        BOOST_ASSERT(testPoint(L"startCommitMergeDeletes"));
        
        SegmentInfosPtr sourceSegments(merge->segments);
        
        if (infoStream)
            message(L"commitMergeDeletes " + merge->segString(directory));
        
        // Carefully merge deletes that occurred after we started merging
        int32_t docUpto = 0;
        int32_t delCount = 0;
        
        for (int32_t i = 0; i < sourceSegments->size(); ++i)
        {
            SegmentInfoPtr info(sourceSegments->info(i));
            int32_t docCount = info->docCount;
            SegmentReaderPtr previousReader(merge->readersClone[i]);
            SegmentReaderPtr currentReader(merge->readers[i]);
            if (previousReader->hasDeletions())
            {
                // There were deletes on this segment when the merge started.  The merge has collapsed away those deletes, 
                // but if new deletes were flushed since the merge started, we must now carefully keep any newly flushed 
                // deletes but mapping them to the new docIDs.
                
                if (currentReader->numDeletedDocs() > previousReader->numDeletedDocs())
                {
                    // This means this segment has had new deletes committed since we started the merge, so we must merge them
                    for (int32_t j = 0; j < docCount; ++j)
                    {
                        if (previousReader->isDeleted(j))
                            BOOST_ASSERT(currentReader->isDeleted(j));
                        else
                        {
                            if (currentReader->isDeleted(j))
                            {
                                mergedReader->doDelete(docUpto);
                                ++delCount;
                            }
                            ++docUpto;
                        }
                    }
                }
                else
                    docUpto += docCount - previousReader->numDeletedDocs();
            }
            else if (currentReader->hasDeletions())
            {
                // This segment had no deletes before but now it does
                for (int32_t j = 0; j < docCount; ++j)
                {
                    if (currentReader->isDeleted(j))
                    {
                        mergedReader->doDelete(docUpto);
                        ++delCount;
                    }
                    ++docUpto;
                }
            }
            else
            {
                // No deletes before or after
                docUpto += info->docCount;
            }
        }
        
        BOOST_ASSERT(mergedReader->numDeletedDocs() == delCount);
        
        mergedReader->_hasChanges = (delCount > 0);
    }
    
    bool IndexWriter::commitMerge(OneMergePtr merge, SegmentReaderPtr mergedReader)
    {
        SyncLock syncLock(this);
        BOOST_ASSERT(testPoint(L"startCommitMerge"));
        
        if (hitOOM)
            boost::throw_exception(IllegalStateException(L"this writer hit an OutOfMemoryError; cannot complete merge"));
        
        if (infoStream)
            message(L"commitMerge: " + merge->segString(directory) + L" index=" + segString());
        
        BOOST_ASSERT(merge->registerDone);
        
        // If merge was explicitly aborted, or, if rollback() or rollbackTransaction() had been called since our merge 
        // started (which results in an unqualified deleter.refresh() call that will remove any index file that current 
        // segments does not reference), we abort this merge
        if (merge->isAborted())
        {
            if (infoStream)
                message(L"commitMerge: skipping merge " + merge->segString(directory) + L": it was aborted");
            return false;
        }
        
        int32_t start = ensureContiguousMerge(merge);
        
        commitMergedDeletes(merge, mergedReader);
        
        // If the doc store we are using has been closed and is in now compound format (but wasn't when we started), 
        // then we will switch to the compound format as well
        setMergeDocStoreIsCompoundFile(merge);
        
        segmentInfos->remove(start, start + merge->segments->size());
        BOOST_ASSERT(!segmentInfos->contains(merge->info));
        segmentInfos->add(start, merge->info);
        
        closeMergeReaders(merge, false);
        
        // Must note the change to segmentInfos so any commits in-flight don't lose it
        checkpoint();
        
        // If the merged segments had pending changes, clear them so that they don't bother writing 
        // them to disk, updating SegmentInfo, etc.
        readerPool->clear(merge->segments);
        
        // remove pending deletes of the segments  that were merged, moving them onto the segment just
        // before the merged segment Lock order: IW -> BD
        bufferedDeletes->commitMerge(merge);
        
        if (merge->optimize)
        {
            // cascade the optimize
            segmentsToOptimize.add(merge->info);
        }
        return true;
    }
    
    LuceneException IndexWriter::handleMergeException(const LuceneException& exc, OneMergePtr merge)
    {
        if (infoStream)
            message(L"handleMergeException: merge=" + merge->segString(directory) + L" exc=" + exc.getError());
        
        // Set the exception on the merge, so if optimize() is waiting on us it sees the root cause exception
        merge->setException(exc);
        addMergeException(merge);
        
        switch (exc.getType())
        {
            case LuceneException::MergeAborted:
                // We can ignore this exception (it happens when close(false) or rollback is called), unless the
                // merge involves segments from external directories, in which case we must throw it so, for 
                // example, the rollbackTransaction code in addIndexes* is executed.
                if (merge->isExternal)
                    return exc;
                break;
            case LuceneException::IO:
            case LuceneException::Runtime:
                return exc;
            default:
                return RuntimeException(); // Should not get here
        }
        return LuceneException();
    }
    
    void IndexWriter::merge(OneMergePtr merge)
    {
        bool success = false;
        
        int64_t t0 = MiscUtils::currentTimeMillis();
        
        try
        {
            LuceneException finally;
            try
            {
                try
                {
                    mergeInit(merge);
                    if (infoStream)
                        message(L"now merge\n merge=" + merge->segString(directory) + L"\n index=" + segString());
                    
                    mergeMiddle(merge);
                    mergeSuccess(merge);
                    success = true;
                }
                catch (LuceneException& e)
                {
                    finally = handleMergeException(e, merge);
                }
            
                {
                    SyncLock syncLock(this);
                    mergeFinish(merge);
                    
                    if (!success)
                    {
                        if (infoStream)
                            message(L"hit exception during merge");
                    
                        if (merge->info && !segmentInfos->contains(merge->info))
                            deleter->refresh(merge->info->name);
                    }

                    // This merge (and, generally, any change to the segments) may now enable 
                    // new merges, so we call merge policy & update pending merges.
                    if (success && !merge->isAborted() && (merge->optimize || (!closed && !closing)))
                        updatePendingMerges(merge->maxNumSegmentsOptimize, merge->optimize);
                }
            }
            catch (LuceneException& e)
            {
                finally = e;
            }
            finally.throwException();
        }
        catch (std::bad_alloc& oom)
        {
            boost::throw_exception(handleOOM(oom, L"merge"));
        }
        if (infoStream)
        {
            message(L"merge time " + StringUtils::toString(MiscUtils::currentTimeMillis() - t0) + 
                    L" msec for " + StringUtils::toString(merge->info->docCount) + L" docs");
        }
    }
    
    void IndexWriter::mergeSuccess(OneMergePtr merge)
    {
        // override
    }
    
    bool IndexWriter::registerMerge(OneMergePtr merge)
    {
        SyncLock syncLock(this);
        
        if (merge->registerDone)
            return true;
        
        if (stopMerges)
        {
            merge->abort();
            boost::throw_exception(MergeAbortedException(L"merge is aborted: " + merge->segString(directory)));
        }
        
        int32_t count = merge->segments->size();
        bool isExternal = false;
        for (int32_t i = 0; i < count; ++i)
        {
            SegmentInfoPtr info(merge->segments->info(i));
            if (mergingSegments.contains(info))
                return false;
            if (!segmentInfos->contains(info))
                return false;
            if (info->dir != directory)
                isExternal = true;
            if (segmentsToOptimize.contains(info))
            {
                merge->optimize = true;
                merge->maxNumSegmentsOptimize = optimizeMaxNumSegments;
            }
        }
        
        ensureContiguousMerge(merge);
        
        pendingMerges.add(merge);
        
        if (infoStream)
            message(L"add merge to pendingMerges: " + merge->segString(directory) + L" [total " + StringUtils::toString(pendingMerges.size()) + L" pending]");
        
        merge->mergeGen = mergeGen;
        merge->isExternal = isExternal;
        
        // OK it does not conflict; now record that this merge is running (while synchronized) 
        // to avoid race condition where two conflicting merges from different threads, start
        for (int32_t i = 0; i < count; ++i)
            mergingSegments.add(merge->segments->info(i));
        
        // Merge is now registered
        merge->registerDone = true;
        return true;
    }
    
    void IndexWriter::mergeInit(OneMergePtr merge)
    {
        SyncLock syncLock(this);
        bool success = false;
        LuceneException finally;
        try
        {
            _mergeInit(merge);
            success = true;
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        
        if (!success)
        {
            if (infoStream)
                message(L"hit exception in mergeInit");
            mergeFinish(merge);
        }
        finally.throwException();
    }
    
    void IndexWriter::_mergeInit(OneMergePtr merge)
    {
        SyncLock syncLock(this);
        bool test = testPoint(L"startMergeInit");
        BOOST_ASSERT(test);
        
        BOOST_ASSERT(merge->registerDone);
        BOOST_ASSERT(!merge->optimize || merge->maxNumSegmentsOptimize > 0);
        
        if (hitOOM)
            boost::throw_exception(IllegalStateException(L"this writer hit an OutOfMemoryError; cannot merge"));
        
        if (merge->info)
        {
            // mergeInit already done
            return;
        }
        
        if (merge->isAborted())
            return;
        
        // Lock order: IW -> BD
        if (bufferedDeletes->applyDeletes(readerPool, segmentInfos, merge->segments))
            checkpoint();

        bool hasVectors = false;
        SegmentInfosPtr sourceSegments(merge->segments);
        int32_t end = sourceSegments->size();
        for (int32_t i = 0; i < end; ++i)
        {
            if (sourceSegments->info(i)->getHasVectors())
                hasVectors = true;
        }

        // Bind a new segment name here so even with ConcurrentMergePolicy we keep deterministic segment names.
        merge->info = newLucene<SegmentInfo>(newSegmentName(), 0, directory, false, true, false, hasVectors);

        MapStringString details(MapStringString::newInstance());
        details.put(L"optimize", merge->optimize ? L"true" : L"false");
        details.put(L"mergeFactor", StringUtils::toString(merge->segments->size()));
        setDiagnostics(merge->info, L"merge", details);

        if (infoStream)
            message(L"merge seg=" + merge->info->name);

        // Also enroll the merged segment into mergingSegments; this prevents it from getting selected for a 
        // merge after our merge is done but while we are building the CFS
        mergingSegments.add(merge->info);
    }
    
    void IndexWriter::setDiagnostics(SegmentInfoPtr info, const String& source)
    {
        setDiagnostics(info, source, MapStringString());
    }
    
    void IndexWriter::setDiagnostics(SegmentInfoPtr info, const String& source, MapStringString details)
    {
        MapStringString diagnostics(MapStringString::newInstance());
        diagnostics.put(L"source", source);
        diagnostics.put(L"lucene.version", Constants::LUCENE_VERSION);
        diagnostics.put(L"os", Constants::OS_NAME);
        if (details)
            diagnostics.putAll(details.begin(), details.end());
        info->setDiagnostics(diagnostics);
    }
    
    void IndexWriter::mergeFinish(OneMergePtr merge)
    {
        SyncLock syncLock(this);
        // Optimize, addIndexes or finishMerges may be waiting on merges to finish.
        notifyAll();
        
        // It's possible we are called twice, eg if there was an exception inside mergeInit
        if (merge->registerDone)
        {
            SegmentInfosPtr sourceSegments(merge->segments);
            int32_t end = sourceSegments->size();
            for (int32_t i = 0; i < end; ++i)
                mergingSegments.remove(sourceSegments->info(i));
            
            mergingSegments.remove(merge->info);
            merge->registerDone = false;
        }

        runningMerges.remove(merge);
    }
    
    void IndexWriter::setMergeDocStoreIsCompoundFile(OneMergePtr merge)
    {
        SyncLock syncLock(this);
        
        String mergeDocStoreSegment(merge->info->getDocStoreSegment());
        if (!mergeDocStoreSegment.empty() && !merge->info->getDocStoreIsCompoundFile())
        {
            int32_t size = segmentInfos->size();
            for (int32_t i = 0; i < size; ++i)
            {
                SegmentInfoPtr info(segmentInfos->info(i));
                String docStoreSegment(info->getDocStoreSegment());
                if (!docStoreSegment.empty() && docStoreSegment == mergeDocStoreSegment && info->getDocStoreIsCompoundFile())
                {
                    merge->info->setDocStoreIsCompoundFile(true);
                    break;
                }
            }
        }
    }
    
    void IndexWriter::closeMergeReaders(OneMergePtr merge, bool suppressExceptions)
    {
        SyncLock syncLock(this);
        
        int32_t numSegments = merge->segments->size();
        if (suppressExceptions)
        {
            // Suppress any new exceptions so we throw the original cause
            bool anyChanges = false;
            for (int32_t i = 0; i < numSegments; ++i)
            {
                if (merge->readers[i])
                {
                    try
                    {
                        if (readerPool->release(merge->readers[i], false))
                            anyChanges = true;
                    }
                    catch (...)
                    {
                    }
                    merge->readers[i].reset();
                }
                
                if (merge->readersClone[i])
                {
                    try
                    {
                        merge->readersClone[i]->close();
                    }
                    catch (...)
                    {
                    }
                    // This was a private clone and we had the only reference
                    BOOST_ASSERT(merge->readersClone[i]->getRefCount() == 0);
                    merge->readersClone[i].reset();
                }
            }
            if (anyChanges)
                checkpoint();
        }
        else
        {
            for (int32_t i = 0; i < numSegments; ++i)
            {
                if (merge->readers[i])
                {
                    readerPool->release(merge->readers[i], true);
                    merge->readers[i].reset();
                }
                
                if (merge->readersClone[i])
                {
                    merge->readersClone[i]->close();
                    // This was a private clone and we had the only reference
                    BOOST_ASSERT(merge->readersClone[i]->getRefCount() == 0);
                    merge->readersClone[i].reset();
                }
            }
        }
    }
    
    int32_t IndexWriter::mergeMiddle(OneMergePtr merge)
    {
        merge->checkAborted(directory);
        
        String mergedName(merge->info->name);
        int32_t mergedDocCount = 0;
        
        SegmentInfosPtr sourceSegments(merge->segments);
        int32_t numSegments = sourceSegments->size();
        
        SegmentMergerPtr merger(newLucene<SegmentMerger>(directory, config->getTermIndexInterval(), mergedName, 
                                                         merge, payloadProcessorProvider,
                                                         boost::static_pointer_cast<FieldInfos>(docWriter->getFieldInfos()->clone())));
        
        if (infoStream)
        {
            message(L"merging " + merge->segString(directory) + 
                    L" mergeVectors=" + StringUtils::toString(merge->info->getHasVectors()));
        }
        
        merge->info->setHasVectors(merger->fieldInfos()->hasVectors());
        merge->readers = Collection<SegmentReaderPtr>::newInstance(numSegments);
        merge->readersClone = Collection<SegmentReaderPtr>::newInstance(numSegments);
        
        // This is try/finally to make sure merger's readers are closed
        LuceneException finally;
        bool success = false;
        try
        {
            int32_t totDocCount = 0;
            for (int32_t i = 0; i < numSegments; ++i)
            {
                SegmentInfoPtr info(sourceSegments->info(i));
                
                // Hold onto the "live" reader; we will use this to commit merged deletes
                merge->readers[i] = readerPool->get(info, true, MERGE_READ_BUFFER_SIZE, -1);
                SegmentReaderPtr reader(merge->readers[i]);
                
                // We clone the segment readers because other deletes may come in while we're merging so we need readers that will not change
                merge->readersClone[i] = boost::static_pointer_cast<SegmentReader>(reader->clone(true));
                SegmentReaderPtr clone(merge->readersClone[i]);
                merger->add(clone);

                totDocCount += clone->numDocs();
            }
            
            if (infoStream)
                message(L"merge: total " + StringUtils::toString(totDocCount) + L" docs");
            
            merge->checkAborted(directory);
            
            // This is where all the work happens
            merge->info->docCount = merger->merge();
            mergedDocCount = merge->info->docCount;

            BOOST_ASSERT(mergedDocCount == totDocCount);

            if (infoStream)
            {
                message(L"merge store matchedCount=" + StringUtils::toString(merger->getMatchedSubReaderCount()) + 
                        L" vs " + StringUtils::toString(numSegments));
            }

            if (merger->getMatchedSubReaderCount() != numSegments)
                anyNonBulkMerges = true;

            // Very important to do this before opening the reader because SegmentReader must know if prox was written 
            // for this segment
            merge->info->setHasProx(merger->fieldInfos()->hasProx());

            bool useCompoundFile = false;
            {
                SyncLock syncLock(this); // Guard segmentInfos
                useCompoundFile = mergePolicy->useCompoundFile(segmentInfos, merge->info);
            }

            if (useCompoundFile)
            {
                success = false;
                
                String compoundFileName(IndexFileNames::segmentFileName(mergedName, IndexFileNames::COMPOUND_FILE_EXTENSION()));
                
                try
                {
                    if (infoStream)
                        message(L"create compound file " + compoundFileName);
                    merger->createCompoundFile(compoundFileName, merge->info);
                    success = true;
                }
                catch (IOException& ioe)
                {
                    SyncLock syncLock(this);
                    if (merge->isAborted())
                    {
                        // This can happen if rollback or close(false) is called - fall through to logic 
                        // below to remove the partially created CFS
                    }
                    else
                        finally = handleMergeException(ioe, merge);
                }
                catch (LuceneException& e)
                {
                    finally = handleMergeException(e, merge);
                }
                
                if (!success)
                {
                    if (infoStream)
                        message(L"hit exception creating compound file during merge");
                    {
                        SyncLock syncLock(this);
                        deleter->deleteFile(compoundFileName);
                        deleter->deleteNewFiles(merge->info->files());
                    }
                }
                
                finally.throwException();
                
                success = false;
                
                {
                    SyncLock syncLock(this);
                    
                    // delete new non cfs files directly: they were never registered with IFD
                    deleter->deleteNewFiles(merge->info->files());
                    
                    if (merge->isAborted())
                    {
                        if (infoStream)
                            message(L"abort merge after building CFS");
                        deleter->deleteFile(compoundFileName);
                        boost::throw_exception(TemporaryException());
                    }
                }
                
                merge->info->setUseCompoundFile(true);
            }
            
            IndexReaderWarmerPtr mergedSegmentWarmer(config->getMergedSegmentWarmer());
            int32_t termsIndexDivisor = -1;
            bool loadDocStores = false;

            if (mergedSegmentWarmer)
            {
                // Load terms index & doc stores so the segment warmer can run searches, load documents/term vectors
                termsIndexDivisor = config->getReaderTermsIndexDivisor();
                loadDocStores = true;
            }
            else
            {
                termsIndexDivisor = -1;
                loadDocStores = false;
            }
            
            SegmentReaderPtr mergedReader(readerPool->get(merge->info, loadDocStores, BufferedIndexInput::BUFFER_SIZE, termsIndexDivisor));
            
            try
            {
                if (poolReaders && mergedSegmentWarmer)
                    mergedSegmentWarmer->warm(mergedReader);
                if (!commitMerge(merge, mergedReader))
                {
                    // commitMerge will return false if this merge was aborted
                    boost::throw_exception(TemporaryException());
                }
            }
            catch (LuceneException& e)
            {
                finally = e;
            }
            
            {
                SyncLock syncLock(this);
                if (readerPool->release(mergedReader))
                {
                    // Must checkpoint after releasing the mergedReader since it may have written a new deletes file
                    checkpoint();
                }
            }
            
            finally.throwException();
            
            success = true;
        }
        catch (LuceneException& e)
        {
            finally = e;
        }

        // Readers are already closed in commitMerge if we didn't hit an exc
        if (!success)
            closeMergeReaders(merge, true);
        
        // has this merge been aborted?
        if (finally.getType() == LuceneException::Temporary)
            return 0;
        
        finally.throwException();
        
        return mergedDocCount;
    }
    
    void IndexWriter::addMergeException(OneMergePtr merge)
    {
        SyncLock syncLock(this);
        BOOST_ASSERT(!merge->getException().isNull());
        if (!mergeExceptions.contains(merge) && mergeGen == merge->mergeGen)
            mergeExceptions.add(merge);
    }
    
    int32_t IndexWriter::getBufferedDeleteTermsSize()
    {
        return docWriter->getPendingDeletes()->terms.size();
    }
    
    int32_t IndexWriter::getNumBufferedDeleteTerms()
    {
        return docWriter->getPendingDeletes()->numTermDeletes->get();
    }
    
    SegmentInfoPtr IndexWriter::newestSegment()
    {
        return !segmentInfos->empty() ? segmentInfos->info(segmentInfos->size() - 1) : SegmentInfoPtr();
    }

    String IndexWriter::segString()
    {
        return segString(segmentInfos);
    }
    
    String IndexWriter::segString(SegmentInfosPtr infos)
    {
        SyncLock syncLock(this);
        StringStream buffer;
        int32_t count = infos->size();
        for (int32_t i = 0; i < count; ++i)
        {
            if (i > 0)
                buffer << L" ";
            SegmentInfoPtr info(infos->info(i));
            buffer << info->toString(directory, 0);
            if (info->dir != directory)
                buffer << L"**";
        }
        return buffer.str();
    }
    
    void IndexWriter::doWait()
    {
        SyncLock syncLock(this);
        // NOTE: the callers of this method should in theory be able to do simply wait(), but, as a defense against 
        // thread timing hazards where notifyAll() fails to be called, we wait for at most 1 second and then return 
        // so caller can check if wait conditions are satisfied
        wait(1000);
    }

    void IndexWriter::keepFullyDeletedSegments()
    {
        _keepFullyDeletedSegments = true;
    }
    
    bool IndexWriter::filesExist(SegmentInfosPtr toSync)
    {
        HashSet<String> files(toSync->files(directory, false));
        for (HashSet<String>::iterator fileName = files.begin(); fileName != files.end(); ++fileName)
        {
            BOOST_ASSERT(directory->fileExists(*fileName));
            // If this trips it means we are missing a call to .checkpoint somewhere, because by the time we
            // are called, deleter should know about every file referenced by the current head segmentInfos
            BOOST_ASSERT(deleter->exists(*fileName));
        }
        return true;
    }
    
    void IndexWriter::startCommit(MapStringString commitUserData)
    {
        BOOST_ASSERT(testPoint(L"startStartCommit"));
        BOOST_ASSERT(!pendingCommit);
        
        if (hitOOM)
            boost::throw_exception(IllegalStateException(L"this writer hit an OutOfMemoryError; cannot commit"));
        
        LuceneException finally;
        try
        {
            if (infoStream)
                message(L"startCommit(): start");

            SegmentInfosPtr toSync;
            int64_t myChangeCount = 0;

            {
                SyncLock syncLock(this);
                BOOST_ASSERT(lastCommitChangeCount <= changeCount);
                myChangeCount = changeCount;

                if (changeCount == lastCommitChangeCount)
                {
                    if (infoStream)
                        message(L"  skip startCommit(): no changes pending");
                    return;
                }

                // First, we clone & incref the segmentInfos we intend to sync, then, without locking, 
                // we sync() all files referenced by toSync, in the background.

                if (infoStream)
                {
                    message(L"startCommit index=" + segString(segmentInfos) + 
                            L" changeCount=" + StringUtils::toString(changeCount));
                }
                readerPool->commit();

                toSync = boost::static_pointer_cast<SegmentInfos>(segmentInfos->clone());
                if (!_keepFullyDeletedSegments)
                    toSync->pruneDeletedSegments();

                BOOST_ASSERT(filesExist(toSync));

                if (commitUserData)
                    toSync->setUserData(commitUserData);

                // This protects the segmentInfos we are now going to commit.  This is important in case,
                // eg., while we are trying to sync all referenced files, a merge completes which would 
                // otherwise have removed the files we are now syncing.
                deleter->incRef(toSync, false);
            }

            BOOST_ASSERT(testPoint(L"midStartCommit"));

            try
            {
                // This call can take a long time -- 10s of seconds or more.  We do it without sync
                directory->sync(toSync->files(directory, false));

                BOOST_ASSERT(testPoint(L"midStartCommit2"));

                {
                    SyncLock syncLock(this);
                    BOOST_ASSERT(!pendingCommit);

                    BOOST_ASSERT(segmentInfos->getGeneration() == toSync->getGeneration());

                    // Exception here means nothing is prepared (this method unwinds everything it did 
                    // on an exception)
                    toSync->prepareCommit(directory);

                    pendingCommit = toSync;
                    pendingCommitChangeCount = myChangeCount;
                }

                if (infoStream)
                    message(L"done all syncs");

                BOOST_ASSERT(testPoint(L"midStartCommitSuccess"));
            }
            catch (LuceneException& e)
            {
                finally = e;
            }
            {
                SyncLock syncLock(this);

                // Have our master segmentInfos record the generations we just prepared.  We do this
                // on error or success so we don't double-write a segments_N file.
                segmentInfos->updateGeneration(toSync);

                if (!pendingCommit)
                {
                    if (infoStream)
                        message(L"hit exception committing segments file");
                    deleter->decRef(toSync);
                }            
            }
            finally.throwException();
        }
        catch (std::bad_alloc& oom)
        {
            finally = handleOOM(oom, L"startCommit");
        }
        BOOST_ASSERT(L"finishStartCommit");
        finally.throwException();
    }
    
    bool IndexWriter::isLocked(DirectoryPtr directory)
    {
        return directory->makeLock(WRITE_LOCK_NAME)->isLocked();
    }
    
    void IndexWriter::unlock(DirectoryPtr directory)
    {
        directory->makeLock(IndexWriter::WRITE_LOCK_NAME)->release();
    }

    void IndexWriter::setMergedSegmentWarmer(IndexReaderWarmerPtr warmer)
    {
        config->setMergedSegmentWarmer(warmer);
    }
    
    IndexReaderWarmerPtr IndexWriter::getMergedSegmentWarmer()
    {
        return config->getMergedSegmentWarmer();
    }
    
    LuceneException IndexWriter::handleOOM(const std::bad_alloc& oom, const String& location)
    {
        if (infoStream)
            message(L"hit OutOfMemoryError inside " + location);
        hitOOM = true;
        return OutOfMemoryError();
    }
    
    bool IndexWriter::testPoint(const String& name)
    {
        return true;
    }
    
    bool IndexWriter::nrtIsCurrent(SegmentInfosPtr infos)
    {
        SyncLock syncLock(this);
        return (infos->version == segmentInfos->version && !docWriter->anyChanges() && !bufferedDeletes->any());
    }
    
    bool IndexWriter::isClosed()
    {
        SyncLock syncLock(this);
        return closed;
    }
    
    void IndexWriter::deleteUnusedFiles()
    {
        SyncLock syncLock(this);
        deleter->deletePendingFiles();
        deleter->revisitPolicy();
    }
    
    void IndexWriter::setPayloadProcessorProvider(PayloadProcessorProviderPtr pcp)
    {
        payloadProcessorProvider = pcp;
    }
    
    PayloadProcessorProviderPtr IndexWriter::getPayloadProcessorProvider()
    {
        return payloadProcessorProvider;
    }
    
    ReaderPool::ReaderPool(IndexWriterPtr writer)
    {
        readerMap = MapSegmentInfoSegmentReader::newInstance();
        _indexWriter = writer;
    }

    ReaderPool::~ReaderPool()
    {
    }
    
    void ReaderPool::clear(SegmentInfosPtr infos)
    {
        SyncLock syncLock(this);
        if (!infos)
        {
            for (MapSegmentInfoSegmentReader::iterator ent = readerMap.begin(); ent != readerMap.end(); ++ent)
                ent->second->_hasChanges = false;
        }
        else
        {
            for (int32_t i = 0; i < infos->size(); ++i)
            {
                MapSegmentInfoSegmentReader::iterator ent = readerMap.find(infos->info(i));
                if (ent != readerMap.end())
                    ent->second->_hasChanges = false;
            }
        }
    }
    
    bool ReaderPool::infoIsLive(SegmentInfoPtr info)
    {
        SyncLock syncLock(this);
        IndexWriterPtr indexWriter(_indexWriter);
        int32_t idx = indexWriter->segmentInfos->find(info);
        BOOST_ASSERT(idx != -1);
        BOOST_ASSERT(indexWriter->segmentInfos->info(idx) == info);
        return true;
    }
    
    SegmentInfoPtr ReaderPool::mapToLive(SegmentInfoPtr info)
    {
        SyncLock syncLock(this);
        IndexWriterPtr indexWriter(_indexWriter);
        int32_t idx = indexWriter->segmentInfos->find(info);
        if (idx != -1)
            info = indexWriter->segmentInfos->info(idx);
        return info;
    }
    
    bool ReaderPool::release(SegmentReaderPtr sr)
    {
        return release(sr, false);
    }
    
    bool ReaderPool::release(SegmentReaderPtr sr, bool drop)
    {
        SyncLock syncLock(this);
        IndexWriterPtr indexWriter(_indexWriter);
        
        bool pooled = readerMap.contains(sr->getSegmentInfo());
        
        BOOST_ASSERT(!pooled || readerMap.get(sr->getSegmentInfo()) == sr);
        
        // Drop caller's ref; for an external reader (not pooled), this decRef will close it
        sr->decRef();
        
        if (pooled && (drop || (!indexWriter->poolReaders && sr->getRefCount() == 1)))
        {
            // We invoke deleter.checkpoint below, so we must be sync'd on IW if there are changes
            BOOST_ASSERT(!sr->_hasChanges || holdsLock());

            // Discard (don't save) changes when we are dropping the reader; this is used only on the 
            // sub-readers after a successful merge.
            sr->_hasChanges = sr->_hasChanges && !drop;

            bool hasChanges = sr->_hasChanges;

            // Drop our ref - this will commit any pending changes to the dir
            sr->close();

            // We are the last ref to this reader; since we're not pooling readers, we release it
            readerMap.remove(sr->getSegmentInfo());

            return hasChanges;
        }
        
        return false;
    }
    
    void ReaderPool::close()
    {
        SyncLock syncLock(this);
        IndexWriterPtr indexWriter(_indexWriter);
        
        // We invoke deleter.checkpoint below, so we must be sync'd on IW
        BOOST_ASSERT(holdsLock());
        
        for (MapSegmentInfoSegmentReader::iterator iter = readerMap.begin(); iter != readerMap.end(); ++iter)
        {
            if (iter->second->_hasChanges)
            {
                BOOST_ASSERT(infoIsLive(iter->second->getSegmentInfo()));
                iter->second->doCommit(MapStringString());
                
                // Must checkpoint with deleter, because this segment reader will have created 
                // new _X_N.del file.
                indexWriter->deleter->checkpoint(indexWriter->segmentInfos, false);
            }
                
            // NOTE: it is allowed that this decRef does not actually close the SR; this can happen when a
            // near real-time reader is kept open after the IndexWriter instance is closed
            iter->second->decRef();
        }
        readerMap.clear();
    }
    
    void ReaderPool::commit()
    {
        SyncLock syncLock(this);
        IndexWriterPtr indexWriter(_indexWriter);
        
        // We invoke deleter.checkpoint below, so we must be sync'd on IW
        BOOST_ASSERT(holdsLock());
        
        for (MapSegmentInfoSegmentReader::iterator ent = readerMap.begin(); ent != readerMap.end(); ++ent)
        {
            if (ent->second->_hasChanges)
            {
                BOOST_ASSERT(infoIsLive(ent->second->getSegmentInfo()));
                ent->second->doCommit(MapStringString());
                
                // Must checkpoint with deleter, because this segment reader will have created 
                // new _X_N.del file.
                indexWriter->deleter->checkpoint(indexWriter->segmentInfos, false);
            }
        }
    }
    
    IndexReaderPtr ReaderPool::getReadOnlyClone(SegmentInfoPtr info, bool doOpenStores, int32_t termInfosIndexDivisor)
    {
        SyncLock syncLock(this);
        SegmentReaderPtr sr(get(info, doOpenStores, BufferedIndexInput::BUFFER_SIZE, termInfosIndexDivisor));
        IndexReaderPtr clone;
        LuceneException finally;
        try
        {
            clone = boost::static_pointer_cast<IndexReader>(sr->clone(true));
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        sr->decRef();
        finally.throwException();
        return clone;
    }
    
    SegmentReaderPtr ReaderPool::get(SegmentInfoPtr info, bool doOpenStores)
    {
        return get(info, doOpenStores, BufferedIndexInput::BUFFER_SIZE, IndexWriterPtr(_indexWriter)->config->getReaderTermsIndexDivisor());
    }
    
    SegmentReaderPtr ReaderPool::get(SegmentInfoPtr info, bool doOpenStores, int32_t readBufferSize, int32_t termsIndexDivisor)
    {
        SyncLock syncLock(this);
        IndexWriterPtr indexWriter(_indexWriter);
        if (indexWriter->poolReaders)
            readBufferSize = BufferedIndexInput::BUFFER_SIZE;
        
        SegmentReaderPtr sr(readerMap.get(info));
        if (!sr)
        {
            // Returns a ref, which we xfer to readerMap
            sr = SegmentReader::get(false, info->dir, info, readBufferSize, doOpenStores, termsIndexDivisor);
            sr->readerFinishedListeners = indexWriter->readerFinishedListeners;
            if (info->dir == indexWriter->directory)
            {
                // Only pool if reader is not external
                readerMap.put(info, sr);
            }
        }
        else
        {
            if (doOpenStores)
                sr->openDocStores();
            if (termsIndexDivisor != -1 && !sr->termsIndexLoaded())
            {
                // If this reader was originally opened because we needed to merge it, we didn't load the terms
                // index.  But now, if the caller wants the terms index (eg because it's doing deletes, or an NRT
                // reader is being opened) we ask the reader to load its terms index.
                sr->loadTermsIndex(termsIndexDivisor);
            }
        }
        
        // Return a ref to our caller
        if (info->dir == indexWriter->directory)
        {
            // Only incRef if we pooled (reader is not external)
            sr->incRef();
        }
        return sr;
    }
    
    SegmentReaderPtr ReaderPool::getIfExists(SegmentInfoPtr info)
    {
        SyncLock syncLock(this);
        SegmentReaderPtr sr(readerMap.get(info));
        if (sr)
            sr->incRef();
        return sr;
    }
    
    FlushControl::FlushControl(IndexWriterPtr writer)
    {
        _writer = writer;
        flushPending = false;
        flushDeletes = false;
        delCount = 0;
        docCount = 0;
        flushing = false;
    }
    
    FlushControl::~FlushControl()
    {
    }
    
    bool FlushControl::setFlushPending(const String& reason, bool doWait)
    {
        SyncLock syncLock(this);
        IndexWriterPtr writer(_writer);
        if (flushPending || flushing)
        {
            if (doWait)
            {
                while (flushPending || flushing)
                    wait();
            }
            return false;
        }
        else
        {
            if (writer->infoStream)
                writer->message(L"now trigger flush reason=" + reason);
            flushPending = true;
            return flushPending;
        }
    }
    
    void FlushControl::setFlushPendingNoWait(const String& reason)
    {
        SyncLock syncLock(this);
        setFlushPending(reason, false);
    }
    
    bool FlushControl::getFlushPending()
    {
        SyncLock syncLock(this);
        return flushPending;
    }
    
    bool FlushControl::getFlushDeletes()
    {
        SyncLock syncLock(this);
        return flushDeletes;
    }
    
    void FlushControl::clearFlushPending()
    {
        SyncLock syncLock(this);
        IndexWriterPtr writer(_writer);
        if (writer->infoStream)
            writer->message(L"clearFlushPending");
        flushPending = false;
        flushDeletes = false;
        docCount = 0;
        notifyAll();
    }
    
    void FlushControl::clearDeletes()
    {
        SyncLock syncLock(this);
        delCount = 0;
    }
    
    bool FlushControl::waitUpdate(int32_t docInc, int32_t delInc)
    {
        return waitUpdate(docInc, delInc, false);
    }
    
    bool FlushControl::waitUpdate(int32_t docInc, int32_t delInc, bool skipWait)
    {
        SyncLock syncLock(this);
        IndexWriterPtr writer(_writer);
        while (flushPending)
            wait();
        
        // skipWait is only used when a thread is BOTH adding a doc and buffering a del 
        // term, and, the adding of the doc already triggered a flush
        if (skipWait)
        {
            docCount += docInc;
            delCount += delInc;
            return false;
        }

        int32_t maxBufferedDocs = writer->config->getMaxBufferedDocs();
        
        if (maxBufferedDocs != IndexWriterConfig::DISABLE_AUTO_FLUSH && (docCount + docInc) >= maxBufferedDocs)
            return setFlushPending(L"maxBufferedDocs", true);
        docCount += docInc;

        int32_t maxBufferedDeleteTerms = writer->config->getMaxBufferedDeleteTerms();
        if (maxBufferedDeleteTerms != IndexWriterConfig::DISABLE_AUTO_FLUSH && (delCount + delInc) >= maxBufferedDeleteTerms)
        {
            flushDeletes = true;
            return setFlushPending(L"maxBufferedDeleteTerms", true);
        }
        delCount += delInc;

        return flushByRAMUsage(L"add delete/doc");
    }
    
    bool FlushControl::flushByRAMUsage(const String& reason)
    {
        SyncLock syncLock(this);
        IndexWriterPtr writer(_writer);
        double ramBufferSizeMB = writer->config->getRAMBufferSizeMB();
        if (ramBufferSizeMB != IndexWriterConfig::DISABLE_AUTO_FLUSH)
        {
            int64_t limit = (int64_t)(ramBufferSizeMB * 1024 * 1024);
            int64_t used = writer->bufferedDeletes->bytesUsed() + writer->docWriter->bytesUsed();
            if (used >= limit)
            {
                // DocumentsWriter may be able to free up some RAM
                // Lock order: FC -> DW
                writer->docWriter->balanceRAM();

                used = writer->bufferedDeletes->bytesUsed() + writer->docWriter->bytesUsed();
                if (used >= limit)
                    return setFlushPending(L"ram full: " + reason, false);
            }
        }
        return false;
    }
    
    IndexReaderWarmer::~IndexReaderWarmer()
    {
    }
}
