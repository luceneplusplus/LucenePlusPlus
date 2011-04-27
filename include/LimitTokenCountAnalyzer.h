/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef LIMITTOKENCOUNTANALYZER_H
#define LIMITTOKENCOUNTANALYZER_H

#include "Analyzer.h"

namespace Lucene
{
    /// This Analyzer limits the number of tokens while indexing. It is a replacement 
    /// for the maximum field length setting inside {@link IndexWriter}.
    class LPPAPI LimitTokenCountAnalyzer : public Analyzer
    {
    public:
        /// Build an analyzer that limits the maximum number of tokens per field.
        LimitTokenCountAnalyzer(AnalyzerPtr delegate, int32_t maxTokenCount);
        
        virtual ~LimitTokenCountAnalyzer();
        
        LUCENE_CLASS(LimitTokenCountAnalyzer);
    
    private:
        AnalyzerPtr delegate;
        int32_t maxTokenCount;
    
    public:
        virtual TokenStreamPtr tokenStream(const String& fieldName, ReaderPtr reader);
        virtual TokenStreamPtr reusableTokenStream(const String& fieldName, ReaderPtr reader);
        virtual int32_t getPositionIncrementGap(const String& fieldName);
        virtual int32_t getOffsetGap(FieldablePtr field);
        
        virtual String toString();
    };
}

#endif
