/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "FreqProxTermsWriterPerThread.h"
#include "FreqProxTermsWriterPerField.h"
#include "TermsHashPerThread.h"

namespace Lucene
{
    FreqProxTermsWriterPerThread::FreqProxTermsWriterPerThread(TermsHashPerThreadPtr perThread)
    {
        docState = perThread->docState;
        _termsHashPerThread = perThread;
    }
    
    FreqProxTermsWriterPerThread::~FreqProxTermsWriterPerThread()
    {
    }
    
    TermsHashConsumerPerFieldPtr FreqProxTermsWriterPerThread::addField(TermsHashPerFieldPtr termsHashPerField, FieldInfoPtr fieldInfo)
    {
        return newLucene<FreqProxTermsWriterPerField>(termsHashPerField, shared_from_this(), fieldInfo);
    }
    
    void FreqProxTermsWriterPerThread::startDocument()
    {
    }
    
    DocWriterPtr FreqProxTermsWriterPerThread::finishDocument()
    {
        return DocWriterPtr();
    }
    
    void FreqProxTermsWriterPerThread::abort()
    {
    }
}
