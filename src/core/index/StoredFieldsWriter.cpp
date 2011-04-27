/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "StoredFieldsWriter.h"
#include "_StoredFieldsWriter.h"
#include "StoredFieldsWriterPerThread.h"
#include "RAMOutputStream.h"
#include "SegmentWriteState.h"
#include "FieldsWriter.h"
#include "IndexFileNames.h"
#include "IndexWriter.h"
#include "Directory.h"
#include "MiscUtils.h"
#include "StringUtils.h"

namespace Lucene
{
    StoredFieldsWriter::StoredFieldsWriter(DocumentsWriterPtr docWriter, FieldInfosPtr fieldInfos)
    {
        lastDocID = 0;
        docFreeList = Collection<StoredFieldsWriterPerDocPtr>::newInstance(1);
        freeCount = 0;
        allocCount = 0;
        
        this->_docWriter = docWriter;
        this->fieldInfos = fieldInfos;
    }
    
    StoredFieldsWriter::~StoredFieldsWriter()
    {
    }
    
    StoredFieldsWriterPerThreadPtr StoredFieldsWriter::addThread(DocStatePtr docState)
    {
        return newLucene<StoredFieldsWriterPerThread>(docState, shared_from_this());
    }
    
    void StoredFieldsWriter::flush(SegmentWriteStatePtr state)
    {
        SyncLock syncLock(this);
        if (state->numDocs > lastDocID)
        {
            initFieldsWriter();
            fill(state->numDocs);
        }

        if (fieldsWriter)
        {
            fieldsWriter->close();
            fieldsWriter.reset();
            lastDocID = 0;

            String fieldsIdxName(IndexFileNames::segmentFileName(state->segmentName, IndexFileNames::FIELDS_INDEX_EXTENSION()));
            if (4 + ((int64_t)state->numDocs) * 8 != state->directory->fileLength(fieldsIdxName))
            {
                boost::throw_exception(RuntimeException(L"after flush: fdx size mismatch: " + StringUtils::toString(state->numDocs) + 
                                                        L" docs vs " + StringUtils::toString(state->directory->fileLength(fieldsIdxName)) + 
                                                        L" length in bytes of " + fieldsIdxName + L" file exists?=" + 
                                                        StringUtils::toString(state->directory->fileExists(fieldsIdxName))));
            }
        }
    }
    
    void StoredFieldsWriter::initFieldsWriter()
    {
        SyncLock syncLock(this);
        if (!fieldsWriter)
        {
            DocumentsWriterPtr docWriter(_docWriter);
            fieldsWriter = newLucene<FieldsWriter>(docWriter->directory, docWriter->getSegment(), fieldInfos);
            lastDocID = 0;
        }
    }
    
    StoredFieldsWriterPerDocPtr StoredFieldsWriter::getPerDoc()
    {
        SyncLock syncLock(this);
        if (freeCount == 0)
        {
            ++allocCount;
            if (allocCount > docFreeList.size())
            {
                // Grow our free list up front to make sure we have enough space to recycle all 
                // outstanding StoredFieldsWriterPerDoc instances
                BOOST_ASSERT(allocCount == docFreeList.size() + 1);
                MiscUtils::grow(docFreeList, allocCount);
            }
            return newLucene<StoredFieldsWriterPerDoc>(shared_from_this());
        }
        else
            return docFreeList[--freeCount];
    }
    
    void StoredFieldsWriter::abort()
    {
        SyncLock syncLock(this);
        fieldsWriter->abort();
        fieldsWriter.reset();
        lastDocID = 0;
    }
    
    void StoredFieldsWriter::fill(int32_t docID)
    {
        // We must "catch up" for all docs before us that had no stored fields
        while (lastDocID < docID)
        {
            fieldsWriter->skipDocument();
            ++lastDocID;
        }
    }
    
    void StoredFieldsWriter::finishDocument(StoredFieldsWriterPerDocPtr perDoc)
    {
        SyncLock syncLock(this);
        IndexWriterPtr writer(DocumentsWriterPtr(_docWriter)->_writer);
        BOOST_ASSERT(writer->testPoint(L"StoredFieldsWriter.finishDocument start"));
        initFieldsWriter();
        
        fill(perDoc->docID);
        
        // Append stored fields to the real FieldsWriter
        fieldsWriter->flushDocument(perDoc->numStoredFields, perDoc->fdt);
        ++lastDocID;
        perDoc->reset();
        free(perDoc);
        BOOST_ASSERT(writer->testPoint(L"StoredFieldsWriter.finishDocument end"));
    }
    
    void StoredFieldsWriter::free(StoredFieldsWriterPerDocPtr perDoc)
    {
        SyncLock syncLock(this);
        BOOST_ASSERT(freeCount < docFreeList.size());
        BOOST_ASSERT(perDoc->numStoredFields == 0);
        BOOST_ASSERT(perDoc->fdt->length() == 0);
        BOOST_ASSERT(perDoc->fdt->getFilePointer() == 0);
        docFreeList[freeCount++] = perDoc;
    }
    
    StoredFieldsWriterPerDoc::StoredFieldsWriterPerDoc(StoredFieldsWriterPtr fieldsWriter)
    {
        this->_fieldsWriter = fieldsWriter;
        buffer = DocumentsWriterPtr(fieldsWriter->_docWriter)->newPerDocBuffer();
        fdt = newLucene<RAMOutputStream>(buffer);
        numStoredFields = 0;
    }
    
    StoredFieldsWriterPerDoc::~StoredFieldsWriterPerDoc()
    {
    }
    
    void StoredFieldsWriterPerDoc::reset()
    {
        fdt->reset();
        buffer->recycle();
        numStoredFields = 0;
    }
    
    void StoredFieldsWriterPerDoc::abort()
    {
        reset();
        StoredFieldsWriterPtr(_fieldsWriter)->free(shared_from_this());
    }
    
    int64_t StoredFieldsWriterPerDoc::sizeInBytes()
    {
        return buffer->getSizeInBytes();
    }
    
    void StoredFieldsWriterPerDoc::finish()
    {
        StoredFieldsWriterPtr(_fieldsWriter)->finishDocument(shared_from_this());
    }
}
