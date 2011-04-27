/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _PAYLOADPROCESSORPROVIDER_H
#define _PAYLOADPROCESSORPROVIDER_H

#include "LuceneObject.h"

namespace Lucene
{
    /// Returns a {@link PayloadProcessor} for a given {@link Term} which allows processing 
    /// the payloads of different terms differently. If you intent to process all your payloads 
    /// the same way, then you can ignore the given term.
    ///
    /// NOTE: if you protect your {@link DirPayloadProcessor} from concurrency issues, then you 
    /// shouldn't worry about any such issues when {@link PayloadProcessor}s are requested for 
    /// different terms.
    class DirPayloadProcessor : public LuceneObject
    {
    public:
        virtual ~DirPayloadProcessor();        
        LUCENE_CLASS(DirPayloadProcessor);
    
    public:
        /// Returns a {@link PayloadProcessor} for the given term.
        virtual PayloadProcessorPtr getProcessor(TermPtr term) = 0;
    };
    
    class PayloadProcessor : public LuceneObject
    {
    public:
        virtual ~PayloadProcessor();        
        LUCENE_CLASS(PayloadProcessor);
    
    public:
        /// Returns the length of the payload that was returned by {@link #processPayload}.
        virtual int32_t payloadLength() = 0;
        
        /// Process the incoming payload and returns the resulting byte[]. Note that a new array 
        /// might be allocated if the given array is not big enough. The length of the new 
        /// payload data can be obtained via {@link #payloadLength()}.
        virtual ByteArray processPayload(ByteArray payload, int32_t start, int32_t length) = 0;
    };
}

#endif
