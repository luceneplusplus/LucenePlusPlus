/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef PAYLOADPROCESSORPROVIDER_H
#define PAYLOADPROCESSORPROVIDER_H

#include "LuceneObject.h"

namespace Lucene
{
    /// Provides a {@link DirPayloadProcessor} to be used for a {@link Directory}. This allows 
    /// using different {@link DirPayloadProcessor}s for different directories, for eg. to 
    /// perform different processing of payloads of different directories.
    ///
    /// NOTE: to avoid processing payloads of certain directories, you can return null in 
    /// {@link #getDirProcessor}.
    ///
    /// NOTE: it is possible that the same {@link DirPayloadProcessor} will be requested for 
    /// the same {@link Directory} concurrently. Therefore, to avoid concurrency issues you 
    /// should return different instances for different threads. Usually, if your {@link 
    /// DirPayloadProcessor} does not maintain state this is not a problem. The merge code 
    /// ensures that the {@link DirPayloadProcessor} instance you return will be accessed by 
    /// one thread to obtain the {@link PayloadProcessor}s for different terms.
    class LPPAPI PayloadProcessorProvider : public LuceneObject
    {
    public:
        virtual ~PayloadProcessorProvider();
        
        LUCENE_CLASS(PayloadProcessorProvider);
    
    public:
        /// Returns a {@link DirPayloadProcessor} for the given {@link Directory}, through 
        /// which {@link PayloadProcessor}s can be obtained for each {@link Term}, or null
        /// if none should be used.
        virtual DirPayloadProcessorPtr getDirProcessor(DirectoryPtr dir) = 0;
    };
}

#endif
