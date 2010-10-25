/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "NormsWriterPerThread.h"
#include "NormsWriterPerField.h"
#include "DocInverterPerThread.h"

namespace Lucene
{
    NormsWriterPerThread::NormsWriterPerThread(DocInverterPerThreadPtr docInverterPerThread, NormsWriterPtr normsWriter)
    {
        this->_normsWriter = normsWriter;
        docState = docInverterPerThread->docState;
    }
    
    NormsWriterPerThread::~NormsWriterPerThread()
    {
    }
    
    InvertedDocEndConsumerPerFieldPtr NormsWriterPerThread::addField(DocInverterPerFieldPtr docInverterPerField, FieldInfoPtr fieldInfo)
    {
        return newLucene<NormsWriterPerField>(docInverterPerField, shared_from_this(), fieldInfo);
    }
    
    void NormsWriterPerThread::abort()
    {
    }
    
    void NormsWriterPerThread::startDocument()
    {
    }
    
    void NormsWriterPerThread::finishDocument()
    {
    }
    
    bool NormsWriterPerThread::freeRAM()
    {
        return false;
    }
}
