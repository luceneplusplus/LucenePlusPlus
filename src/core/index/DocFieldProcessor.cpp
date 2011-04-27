/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "DocFieldProcessor.h"
#include "DocFieldProcessorPerThread.h"
#include "DocFieldConsumerPerThread.h"
#include "DocFieldConsumer.h"
#include "StoredFieldsWriter.h"
#include "SegmentWriteState.h"
#include "IndexFileNames.h"
#include "FieldInfos.h"
#include "TestPoint.h"

namespace Lucene
{
    DocFieldProcessor::DocFieldProcessor(DocumentsWriterPtr docWriter, DocFieldConsumerPtr consumer)
    {
        this->_docWriter = docWriter;
        this->consumer = consumer;
        this->fieldInfos = docWriter->getFieldInfos();
        consumer->setFieldInfos(fieldInfos);
        fieldsWriter = newLucene<StoredFieldsWriter>(docWriter, fieldInfos);
    }
    
    DocFieldProcessor::~DocFieldProcessor()
    {
    }
    
    void DocFieldProcessor::flush(Collection<DocConsumerPerThreadPtr> threads, SegmentWriteStatePtr state)
    {
        TestScope testScope(L"DocFieldProcessor", L"flush");
        MapDocFieldConsumerPerThreadCollectionDocFieldConsumerPerField childThreadsAndFields(MapDocFieldConsumerPerThreadCollectionDocFieldConsumerPerField::newInstance());
        
        for (Collection<DocConsumerPerThreadPtr>::iterator thread = threads.begin(); thread != threads.end(); ++thread)
        {
            DocFieldProcessorPerThreadPtr perThread(boost::static_pointer_cast<DocFieldProcessorPerThread>(*thread));
            childThreadsAndFields.put(perThread->consumer, perThread->fields());
            perThread->trimFields(state);
        }
        fieldsWriter->flush(state);
        consumer->flush(childThreadsAndFields, state);
        
        // Important to save after asking consumer to flush so consumer can alter the FieldInfo* if necessary.
        // eg FreqProxTermsWriter does this with FieldInfo.storePayload.
        String fileName(IndexFileNames::segmentFileName(state->segmentName, IndexFileNames::FIELD_INFOS_EXTENSION()));
        fieldInfos->write(state->directory, fileName);
    }
    
    void DocFieldProcessor::abort()
    {
        fieldsWriter->abort();
        consumer->abort();
    }
    
    bool DocFieldProcessor::freeRAM()
    {
        return consumer->freeRAM();
    }
    
    DocConsumerPerThreadPtr DocFieldProcessor::addThread(DocumentsWriterThreadStatePtr perThread)
    {
        return newLucene<DocFieldProcessorPerThread>(perThread, shared_from_this());
    }
}
