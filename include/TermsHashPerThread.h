/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef TERMSHASHPERTHREAD_H
#define TERMSHASHPERTHREAD_H

#include "InvertedDocConsumerPerThread.h"

namespace Lucene
{
    class TermsHashPerThread : public InvertedDocConsumerPerThread
    {
    public:
        TermsHashPerThread(DocInverterPerThreadPtr docInverterPerThread, TermsHashPtr termsHash, TermsHashPtr nextTermsHash, TermsHashPerThreadPtr primaryPerThread);
        virtual ~TermsHashPerThread();
        
        LUCENE_CLASS(TermsHashPerThread);
            
    public:
        DocInverterPerThreadWeakPtr _docInverterPerThread;
        TermsHashWeakPtr _termsHash;
        TermsHashPtr nextTermsHash;
        TermsHashPerThreadWeakPtr _primaryPerThread;
        TermsHashConsumerPerThreadPtr consumer;
        TermsHashPerThreadPtr nextPerThread;
        
        CharBlockPoolPtr charPool;
        IntBlockPoolPtr intPool;
        ByteBlockPoolPtr bytePool;
        bool primary;
        DocStatePtr docState;
        
    public:
        virtual void initialize();
        
        virtual InvertedDocConsumerPerFieldPtr addField(DocInverterPerFieldPtr docInverterPerField, FieldInfoPtr fieldInfo);
        virtual void abort();
        
        virtual void startDocument();
        virtual DocWriterPtr finishDocument();
        
        /// Clear all state
        void reset(bool recyclePostings);        
    };
}

#endif
