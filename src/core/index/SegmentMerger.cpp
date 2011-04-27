/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "SegmentMerger.h"
#include "MergePolicy.h"
#include "IndexWriter.h"
#include "IndexOutput.h"
#include "FieldInfos.h"
#include "FieldInfo.h"
#include "FieldsReader.h"
#include "FieldsWriter.h"
#include "IndexFileNames.h"
#include "CompoundFileWriter.h"
#include "SegmentReader.h"
#include "_SegmentReader.h"
#include "SegmentInfo.h"
#include "Directory.h"
#include "TermPositions.h"
#include "TermVectorsReader.h"
#include "TermVectorsWriter.h"
#include "FormatPostingsDocsConsumer.h"
#include "FormatPostingsFieldsWriter.h"
#include "FormatPostingsPositionsConsumer.h"
#include "FormatPostingsTermsConsumer.h"
#include "SegmentMergeInfo.h"
#include "SegmentMergeQueue.h"
#include "SegmentWriteState.h"
#include "TestPoint.h"
#include "MiscUtils.h"
#include "StringUtils.h"
#include "IndexWriterConfig.h"
#include "ReaderUtil.h"
#include "PayloadProcessorProvider.h"
#include "_PayloadProcessorProvider.h"

namespace Lucene
{
    /// Maximum number of contiguous documents to bulk-copy when merging stored fields
    const int32_t SegmentMerger::MAX_RAW_MERGE_DOCS = 4192;
    
    /// norms header placeholder
    const uint8_t SegmentMerger::NORMS_HEADER[] = {'N', 'R', 'M', -1};
    const int32_t SegmentMerger::NORMS_HEADER_LENGTH = 4;
    
    SegmentMerger::SegmentMerger(DirectoryPtr dir, int32_t termIndexInterval, const String& name, OneMergePtr merge, 
                                 PayloadProcessorProviderPtr payloadProcessorProvider, FieldInfosPtr fieldInfos)
    {
        readers = Collection<IndexReaderPtr>::newInstance();
        mergedDocs = 0;
        omitTermFreqAndPositions = false;
        matchedCount = 0;
        
        this->payloadProcessorProvider = payloadProcessorProvider;
        this->directory = dir;
        this->_fieldInfos = fieldInfos;
        this->segment = name;
        
        if (merge)
            checkAbort = newLucene<CheckAbort>(merge, directory);
        else
            checkAbort = newLucene<CheckAbortNull>();
        this->termIndexInterval = termIndexInterval;
    }
    
    SegmentMerger::~SegmentMerger()
    {
    }

    FieldInfosPtr SegmentMerger::fieldInfos()
    {
        return _fieldInfos;
    }
    
    void SegmentMerger::add(IndexReaderPtr reader)
    {
        ReaderUtil::gatherSubReaders(readers, reader);
    }
    
    int32_t SegmentMerger::merge()
    {
        // NOTE: it's important to add calls to checkAbort.work(...) if you make any changes to this method that will spend a lot of time.  
        // The frequency of this check impacts how long IndexWriter.close(false) takes to actually stop the threads.
        
        mergedDocs = mergeFields();
        mergeTerms();
        mergeNorms();
        
        if (_fieldInfos->hasVectors())
            mergeVectors();
        
        return mergedDocs;
    }
    
    HashSet<String> SegmentMerger::createCompoundFile(const String& fileName, SegmentInfoPtr info)
    {
        // Now merge all added files
        HashSet<String> files(info->files());
        CompoundFileWriterPtr cfsWriter(newLucene<CompoundFileWriter>(directory, fileName, checkAbort));
        for (HashSet<String>::iterator file = files.begin(); file != files.end(); ++file)
            cfsWriter->addFile(*file);
        // Perform the merge
        cfsWriter->close();

        return files;
    }
    
    void SegmentMerger::addIndexed(IndexReaderPtr reader, FieldInfosPtr fInfos, HashSet<String> names, 
                                   bool storeTermVectors, bool storePositionWithTermVector, 
                                   bool storeOffsetWithTermVector, bool storePayloads, bool omitTFAndPositions)
    {
        for (HashSet<String>::iterator field = names.begin(); field != names.end(); ++field)
        {
            fInfos->add(*field, true, storeTermVectors, storePositionWithTermVector, storeOffsetWithTermVector, 
                        !reader->hasNorms(*field), storePayloads, omitTFAndPositions);
        }
    }
    
    int32_t SegmentMerger::getMatchedSubReaderCount()
    {
        return matchedCount;
    }

    void SegmentMerger::setMatchingSegmentReaders()
    {
        // If the i'th reader is a SegmentReader and has identical fieldName -> number mapping, then 
        // this array will be non-null at position i
        int32_t numReaders = readers.size();
        matchingSegmentReaders = Collection<SegmentReaderPtr>::newInstance(numReaders);
        
        // If this reader is a SegmentReader, and all of its field name -> number mappings match the 
        // "merged" FieldInfos, then we can do a bulk copy of the stored fields
        for (int32_t i = 0; i < numReaders; ++i)
        {
            IndexReaderPtr reader(readers[i]);
            SegmentReaderPtr segmentReader(boost::dynamic_pointer_cast<SegmentReader>(reader));
            if (segmentReader)
            {
                bool same = true;
                FieldInfosPtr segmentFieldInfos(segmentReader->fieldInfos());
                int32_t numFieldInfos = segmentFieldInfos->size();
                for (int32_t j = 0; same && j < numFieldInfos; ++j)
                    same = (_fieldInfos->fieldName(j) == segmentFieldInfos->fieldName(j));
                if (same)
                {
                    matchingSegmentReaders[i] = segmentReader;
                    ++matchedCount;
                }
            }
        }
        
        // Used for bulk-reading raw bytes for stored fields
        rawDocLengths = Collection<int32_t>::newInstance(MAX_RAW_MERGE_DOCS);
        rawDocLengths2 = Collection<int32_t>::newInstance(MAX_RAW_MERGE_DOCS);
    }

    int32_t SegmentMerger::mergeFields()
    {
        for (Collection<IndexReaderPtr>::iterator reader = readers.begin(); reader != readers.end(); ++reader)
        {
            SegmentReaderPtr segmentReader(boost::dynamic_pointer_cast<SegmentReader>(*reader));
            if (segmentReader)
            {
                FieldInfosPtr readerFieldInfos(segmentReader->fieldInfos());
                int32_t numReaderFieldInfos = readerFieldInfos->size();
                for (int32_t j = 0; j < numReaderFieldInfos; ++j)
                    _fieldInfos->add(readerFieldInfos->fieldInfo(j));
            }
            else
            {
                addIndexed(*reader, _fieldInfos, (*reader)->getFieldNames(IndexReader::FIELD_OPTION_TERMVECTOR_WITH_POSITION_OFFSET), true, true, true, false, false);
                addIndexed(*reader, _fieldInfos, (*reader)->getFieldNames(IndexReader::FIELD_OPTION_TERMVECTOR_WITH_POSITION), true, true, false, false, false);
                addIndexed(*reader, _fieldInfos, (*reader)->getFieldNames(IndexReader::FIELD_OPTION_TERMVECTOR_WITH_OFFSET), true, false, true, false, false);
                addIndexed(*reader, _fieldInfos, (*reader)->getFieldNames(IndexReader::FIELD_OPTION_TERMVECTOR), true, false, false, false, false);
                addIndexed(*reader, _fieldInfos, (*reader)->getFieldNames(IndexReader::FIELD_OPTION_OMIT_TERM_FREQ_AND_POSITIONS), false, false, false, false, true);
                addIndexed(*reader, _fieldInfos, (*reader)->getFieldNames(IndexReader::FIELD_OPTION_STORES_PAYLOADS), false, false, false, true, false);
                addIndexed(*reader, _fieldInfos, (*reader)->getFieldNames(IndexReader::FIELD_OPTION_INDEXED), false, false, false, false, false);
                _fieldInfos->add((*reader)->getFieldNames(IndexReader::FIELD_OPTION_UNINDEXED), false);
            }
        }
        _fieldInfos->write(directory, segment + L".fnm");
        
        int32_t docCount = 0;
        
        setMatchingSegmentReaders();
        
        // merge field values
        FieldsWriterPtr fieldsWriter(newLucene<FieldsWriter>(directory, segment, _fieldInfos));
        
        LuceneException finally;
        try
        {
            int32_t idx = 0;
            for (Collection<IndexReaderPtr>::iterator reader = readers.begin(); reader != readers.end(); ++reader)
            {
                SegmentReaderPtr matchingSegmentReader(matchingSegmentReaders[idx++]);
                FieldsReaderPtr matchingFieldsReader;
                if (matchingSegmentReader)
                {
                    FieldsReaderPtr fieldsReader(matchingSegmentReader->getFieldsReader());
                    if (fieldsReader && fieldsReader->canReadRawDocs())
                        matchingFieldsReader = fieldsReader;
                }
                if ((*reader)->hasDeletions())
                    docCount += copyFieldsWithDeletions(fieldsWriter, *reader, matchingFieldsReader);
                else
                    docCount += copyFieldsNoDeletions(fieldsWriter, *reader, matchingFieldsReader);
            }
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        fieldsWriter->close();
        finally.throwException();
        
        String fileName(IndexFileNames::segmentFileName(segment, IndexFileNames::FIELDS_INDEX_EXTENSION()));
        int64_t fdxFileLength = directory->fileLength(fileName);
        
        if (4 + ((int64_t)docCount) * 8 != fdxFileLength)
        {
            boost::throw_exception(RuntimeException(L"mergeFields produced an invalid result: docCount is " + 
                                                    StringUtils::toString(docCount) + L" but fdx file size is " + 
                                                    StringUtils::toString(fdxFileLength) + L" file=" + fileName + 
                                                    L" file exists?=" + StringUtils::toString(directory->fileExists(fileName)) + 
                                                    L"; now aborting this merge to prevent index corruption"));
        }
        
        return docCount;
    }

    int32_t SegmentMerger::copyFieldsWithDeletions(FieldsWriterPtr fieldsWriter, IndexReaderPtr reader, FieldsReaderPtr matchingFieldsReader)
    {
        int32_t docCount = 0;
        int32_t maxDoc = reader->maxDoc();
        if (matchingFieldsReader)
        {
            // We can bulk-copy because the fieldInfos are "congruent"
            for (int32_t j = 0; j < maxDoc;)
            {
                if (reader->isDeleted(j))
                {
                    // skip deleted docs
                    ++j;
                    continue;
                }
                // We can optimize this case (doing a bulk byte copy) since the field numbers are identical
                int32_t start = j;
                int32_t numDocs = 0;
                do
                {
                    ++j;
                    ++numDocs;
                    if (j >= maxDoc)
                        break;
                    if (reader->isDeleted(j))
                    {
                        ++j;
                        break;
                    }
                }
                while (numDocs < MAX_RAW_MERGE_DOCS);
                
                IndexInputPtr stream(matchingFieldsReader->rawDocs(rawDocLengths, start, numDocs));
                fieldsWriter->addRawDocuments(stream, rawDocLengths, numDocs);
                docCount += numDocs;
                checkAbort->work(300 * numDocs);
            }
        }
        else
        {
            for (int32_t j = 0; j < maxDoc; ++j)
            {
                if (reader->isDeleted(j))
                {
                    // skip deleted docs
                    continue;
                }
                
                // NOTE: it's very important to first assign to doc then pass it to termVectorsWriter.addAllDocVectors
                fieldsWriter->addDocument(reader->document(j));
                ++docCount;
                checkAbort->work(300);
            }
        }
        return docCount;
    }
    
    int32_t SegmentMerger::copyFieldsNoDeletions(FieldsWriterPtr fieldsWriter, IndexReaderPtr reader, FieldsReaderPtr matchingFieldsReader)
    {
        int32_t docCount = 0;
        int32_t maxDoc = reader->maxDoc();
        if (matchingFieldsReader)
        {
            // We can bulk-copy because the fieldInfos are "congruent"
            while (docCount < maxDoc)
            {
                int32_t len = std::min(MAX_RAW_MERGE_DOCS, maxDoc - docCount);
                IndexInputPtr stream(matchingFieldsReader->rawDocs(rawDocLengths, docCount, len));
                fieldsWriter->addRawDocuments(stream, rawDocLengths, len);
                docCount += len;
                checkAbort->work(300 * len);
            }
        }
        else
        {
            for (; docCount < maxDoc; ++docCount)
            {
                // NOTE: it's very important to first assign to doc then pass it to termVectorsWriter.addAllDocVectors
                fieldsWriter->addDocument(reader->document(docCount));
                checkAbort->work(300);
            }
        }
        return docCount;
    }

    void SegmentMerger::mergeVectors()
    {
        TermVectorsWriterPtr termVectorsWriter(newLucene<TermVectorsWriter>(directory, segment, _fieldInfos));
        
        LuceneException finally;
        try
        {
            int32_t idx = 0;
            for (Collection<IndexReaderPtr>::iterator reader = readers.begin(); reader != readers.end(); ++reader)
            {
                SegmentReaderPtr matchingSegmentReader(matchingSegmentReaders[idx++]);
                TermVectorsReaderPtr matchingVectorsReader;
                if (matchingSegmentReader)
                {
                    TermVectorsReaderPtr vectorsReader(matchingSegmentReader->getTermVectorsReader());
                    
                    // If the TV* files are an older format then they cannot read raw docs
                    if (vectorsReader && vectorsReader->canReadRawDocs())
                        matchingVectorsReader = vectorsReader;
                }
                if ((*reader)->hasDeletions())
                    copyVectorsWithDeletions(termVectorsWriter, matchingVectorsReader, *reader);
                else
                    copyVectorsNoDeletions(termVectorsWriter, matchingVectorsReader, *reader);
            }
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        termVectorsWriter->close();
        finally.throwException();
        
        String fileName(IndexFileNames::segmentFileName(segment, IndexFileNames::VECTORS_INDEX_EXTENSION()));
        int64_t tvxSize = directory->fileLength(fileName);
        
        if (4 + ((int64_t)mergedDocs) * 16 != tvxSize)
        {
            boost::throw_exception(RuntimeException(L"mergeVectors produced an invalid result: mergedDocs is " + 
                                                    StringUtils::toString(mergedDocs) + L" but tvx size is " + 
                                                    StringUtils::toString(tvxSize) + L" file=" + fileName + 
                                                    L" file exists?=" + StringUtils::toString(directory->fileExists(fileName)) + 
                                                    L"; now aborting this merge to prevent index corruption"));
        }
    }
    
    void SegmentMerger::copyVectorsWithDeletions(TermVectorsWriterPtr termVectorsWriter, TermVectorsReaderPtr matchingVectorsReader, IndexReaderPtr reader)
    {
        int32_t maxDoc = reader->maxDoc();
        if (matchingVectorsReader)
        {
            // We can bulk-copy because the fieldInfos are "congruent"
            for (int32_t docNum = 0; docNum < maxDoc;)
            {
                if (reader->isDeleted(docNum))
                {
                    // skip deleted docs
                    ++docNum;
                    continue;
                }
                // We can optimize this case (doing a bulk byte copy) since the field numbers are identical
                int32_t start = docNum;
                int32_t numDocs = 0;
                do
                {
                    ++docNum;
                    ++numDocs;
                    if (docNum >= maxDoc)
                        break;
                    if (reader->isDeleted(docNum))
                    {
                        ++docNum;
                        break;
                    }
                }
                while (numDocs < MAX_RAW_MERGE_DOCS);
                
                matchingVectorsReader->rawDocs(rawDocLengths, rawDocLengths2, start, numDocs);
                termVectorsWriter->addRawDocuments(matchingVectorsReader, rawDocLengths, rawDocLengths2, numDocs);
                checkAbort->work(300 * numDocs);
            }
        }
        else
        {
            for (int32_t docNum = 0; docNum < maxDoc; ++docNum)
            {
                if (reader->isDeleted(docNum))
                {
                    // skip deleted docs
                    continue;
                }
                
                // NOTE: it's very important to first assign to vectors then pass it to termVectorsWriter.addAllDocVectors
                termVectorsWriter->addAllDocVectors(reader->getTermFreqVectors(docNum));
                checkAbort->work(300);
            }
        }
    }
    
    void SegmentMerger::copyVectorsNoDeletions(TermVectorsWriterPtr termVectorsWriter, TermVectorsReaderPtr matchingVectorsReader, IndexReaderPtr reader)
    {
        int32_t maxDoc = reader->maxDoc();
        if (matchingVectorsReader)
        {
            // We can bulk-copy because the fieldInfos are "congruent"
            int32_t docCount = 0;
            while (docCount < maxDoc)
            {
                int32_t len = std::min(MAX_RAW_MERGE_DOCS, maxDoc - docCount);
                matchingVectorsReader->rawDocs(rawDocLengths, rawDocLengths2, docCount, len);
                termVectorsWriter->addRawDocuments(matchingVectorsReader, rawDocLengths, rawDocLengths2, len);
                docCount += len;
                checkAbort->work(300 * len);
            }
        }
        else
        {
            for (int32_t docNum = 0; docNum < maxDoc; ++docNum)
            {
                // NOTE: it's very important to first assign to vectors then pass it to termVectorsWriter.addAllDocVectors
                termVectorsWriter->addAllDocVectors(reader->getTermFreqVectors(docNum));
                checkAbort->work(300);
            }
        }
    }

    void SegmentMerger::mergeTerms()
    {
        TestScope testScope(L"SegmentMerger", L"mergeTerms");
        
        FormatPostingsFieldsConsumerPtr fieldsConsumer(newLucene<FormatPostingsFieldsWriter>(segmentWriteState, _fieldInfos));
        
        LuceneException finally;
        try
        {
            queue = newLucene<SegmentMergeQueue>(readers.size());
            mergeTermInfos(fieldsConsumer);
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        fieldsConsumer->finish();
        if (queue)
            queue->close();
        finally.throwException();
    }
    
    void SegmentMerger::mergeTermInfos(FormatPostingsFieldsConsumerPtr consumer)
    {
        int32_t base = 0;
        int32_t readerCount = readers.size();
        for (int32_t i = 0; i < readerCount; ++i)
        {
            IndexReaderPtr reader(readers[i]);
            TermEnumPtr termEnum(reader->terms());
            SegmentMergeInfoPtr smi(newLucene<SegmentMergeInfo>(base, termEnum, reader));
            if (payloadProcessorProvider)
                smi->dirPayloadProcessor = payloadProcessorProvider->getDirProcessor(reader->directory());
            Collection<int32_t> docMap(smi->getDocMap());
            if (docMap)
            {
                if (!docMaps)
                {
                    docMaps = Collection< Collection<int32_t> >::newInstance(readerCount);
                    delCounts = Collection<int32_t>::newInstance(readerCount);
                }
                docMaps[i] = docMap;
                IndexReaderPtr segmentMergeReader(smi->_reader);
                delCounts[i] = segmentMergeReader->maxDoc() - segmentMergeReader->numDocs();
            }
            
            base += reader->numDocs();
            
            BOOST_ASSERT(reader->numDocs() == reader->maxDoc() - smi->delCount);
            
            if (smi->next())
                queue->add(smi); // initialize queue
            else
                smi->close();
        }
        
        Collection<SegmentMergeInfoPtr> match(Collection<SegmentMergeInfoPtr>::newInstance(readers.size()));
        
        String currentField;
        FormatPostingsTermsConsumerPtr termsConsumer;
        
        while (!queue->empty())
        {
            int32_t matchSize = 0; // pop matching terms
            match[matchSize++] = queue->pop();
            TermPtr term(match[0]->term);
            SegmentMergeInfoPtr top(queue->empty() ? SegmentMergeInfoPtr() : queue->top());
            
            while (top && term->compareTo(top->term) == 0)
            {
                match[matchSize++] = queue->pop();
                top = queue->top();
            }
            
            if (currentField != term->_field)
            {
                currentField = term->_field;
                if (termsConsumer)
                    termsConsumer->finish();
                FieldInfoPtr fieldInfo(_fieldInfos->fieldInfo(currentField));
                termsConsumer = consumer->addField(fieldInfo);
                omitTermFreqAndPositions = fieldInfo->omitTermFreqAndPositions;
            }
            
            int32_t df = appendPostings(termsConsumer, match, matchSize); // add new TermInfo
            
            checkAbort->work(df / 3.0);
            
            while (matchSize > 0)
            {
                SegmentMergeInfoPtr smi(match[--matchSize]);
                if (smi->next())
                    queue->add(smi); // restore queue
                else
                    smi->close(); // done with a segment
            }
        }
    }
    
    Collection< Collection<int32_t> > SegmentMerger::getDocMaps()
    {
        return docMaps;
    }
    
    Collection<int32_t> SegmentMerger::getDelCounts()
    {
        return delCounts;
    }
    
    int32_t SegmentMerger::appendPostings(FormatPostingsTermsConsumerPtr termsConsumer, Collection<SegmentMergeInfoPtr> smis, int32_t n)
    {
        FormatPostingsDocsConsumerPtr docConsumer(termsConsumer->addTerm(smis[0]->term->_text));
        int32_t df = 0;
        for (int32_t i = 0; i < n; ++i)
        {
            SegmentMergeInfoPtr smi(smis[i]);
            TermPositionsPtr postings(smi->getPositions());
            BOOST_ASSERT(postings);
            int32_t base = smi->base;
            Collection<int32_t> docMap(smi->getDocMap());
            postings->seek(smi->termEnum);
            
            PayloadProcessorPtr payloadProcessor;
            if (smi->dirPayloadProcessor)
                payloadProcessor = smi->dirPayloadProcessor->getProcessor(smi->term);
            
            while (postings->next())
            {
                ++df;
                int32_t doc = postings->doc();
                if (docMap)
                    doc = docMap[doc]; // map around deletions
                doc += base; // convert to merged space
                
                int32_t freq = postings->freq();
                FormatPostingsPositionsConsumerPtr posConsumer(docConsumer->addDoc(doc, freq));
                
                if (!omitTermFreqAndPositions)
                {
                    for (int32_t j = 0; j < freq; ++j)
                    {
                        int32_t position = postings->nextPosition();
                        int32_t payloadLength = postings->getPayloadLength();
                        if (payloadLength > 0)
                        {
                            if (!payloadBuffer)
                                payloadBuffer = ByteArray::newInstance(payloadLength);
                            if (payloadBuffer.size() < payloadLength)
                                payloadBuffer.resize(payloadLength);
                            postings->getPayload(payloadBuffer, 0);
                            if (payloadProcessor)
                            {
                                payloadBuffer = payloadProcessor->processPayload(payloadBuffer, 0, payloadLength);
                                payloadLength = payloadProcessor->payloadLength();
                            }
                        }
                        posConsumer->addPosition(position, payloadBuffer, 0, payloadLength);
                    }
                    posConsumer->finish();
                }
            }
        }
        docConsumer->finish();
        
        return df;
    }

    void SegmentMerger::mergeNorms()
    {
        // get needed buffer size by finding the largest segment
        int32_t bufferSize = 0;
        for (Collection<IndexReaderPtr>::iterator reader = readers.begin(); reader != readers.end(); ++reader)
            bufferSize = std::max(bufferSize, (*reader)->maxDoc());
    
        ByteArray normBuffer;
        IndexOutputPtr output;
        LuceneException finally;
        try
        {
            int32_t numFieldInfos = _fieldInfos->size();
            for (int32_t i = 0; i < numFieldInfos; ++i)
            {
                FieldInfoPtr fi(_fieldInfos->fieldInfo(i));
                if (fi->isIndexed && !fi->omitNorms)
                {
                    if (!output)
                    {
                        output = directory->createOutput(IndexFileNames::segmentFileName(segment, IndexFileNames::NORMS_EXTENSION()));
                        output->writeBytes(NORMS_HEADER, SIZEOF_ARRAY(NORMS_HEADER));
                    }
                    if (!normBuffer)
                        normBuffer = ByteArray::newInstance(bufferSize);
                    for (Collection<IndexReaderPtr>::iterator reader = readers.begin(); reader != readers.end(); ++reader)
                    {
                        int32_t maxDoc = (*reader)->maxDoc();
                        (*reader)->norms(fi->name, normBuffer, 0);
                        if (!(*reader)->hasDeletions())
                        {
                            // optimized case for segments without deleted docs
                            output->writeBytes(normBuffer.get(), maxDoc);
                        }
                        else
                        {
                            // this segment has deleted docs, so we have to check for every doc if it is deleted or not
                            for (int32_t k = 0; k < maxDoc; ++k)
                            {
                                if (!(*reader)->isDeleted(k))
                                    output->writeByte(normBuffer[k]);
                            }
                        }
                        checkAbort->work(maxDoc);
                    }
                }
            }
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        if (output)
            output->close();
        finally.throwException();
    }

    CheckAbort::CheckAbort(OneMergePtr merge, DirectoryPtr dir)
    {
        workCount = 0;
        this->merge = merge;
        this->_dir = dir;
    }
    
    CheckAbort::~CheckAbort()
    {
    }
    
    void CheckAbort::work(double units)
    {
        workCount += units;
        if (workCount >= 10000.0)
        {
            merge->checkAborted(DirectoryPtr(_dir));
            workCount = 0;
        }
    }
    
    CheckAbortNull::CheckAbortNull() : CheckAbort(OneMergePtr(), DirectoryPtr())
    {
    }
    
    CheckAbortNull::~CheckAbortNull()
    {
    }
    
    void CheckAbortNull::work(double units)
    {
        // do nothing
    }
}
