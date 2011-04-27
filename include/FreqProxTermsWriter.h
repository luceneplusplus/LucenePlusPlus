/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef FREQPROXTERMSWRITER_H
#define FREQPROXTERMSWRITER_H

#include "TermsHashConsumer.h"

namespace Lucene
{
    class FreqProxTermsWriter : public TermsHashConsumer
    {
    public:
        virtual ~FreqProxTermsWriter();
        
        LUCENE_CLASS(FreqProxTermsWriter);
            
    protected:
        ByteArray payloadBuffer;
    
    public:
        virtual TermsHashConsumerPerThreadPtr addThread(TermsHashPerThreadPtr perThread);
        virtual void abort();
        virtual void flush(MapTermsHashConsumerPerThreadCollectionTermsHashConsumerPerField threadsAndFields, SegmentWriteStatePtr state);
        
        /// Walk through all unique text tokens (Posting instances) found in this field and serialize them
        /// into a single RAM segment.
        void appendPostings(Collection<FreqProxTermsWriterPerFieldPtr> fields, FormatPostingsFieldsConsumerPtr consumer);
    
    protected:
        static int32_t compareText(const wchar_t* text1, int32_t pos1, const wchar_t* text2, int32_t pos2);
    };
}

#endif
