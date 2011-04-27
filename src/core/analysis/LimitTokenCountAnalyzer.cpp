/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "LimitTokenCountAnalyzer.h"
#include "LimitTokenCountFilter.h"
#include "StringUtils.h"

namespace Lucene
{
    LimitTokenCountAnalyzer::LimitTokenCountAnalyzer(AnalyzerPtr delegate, int32_t maxTokenCount)
    {
        this->delegate = delegate;
        this->maxTokenCount = maxTokenCount;
    }
    
    LimitTokenCountAnalyzer::~LimitTokenCountAnalyzer()
    {
    }
    
    TokenStreamPtr LimitTokenCountAnalyzer::tokenStream(const String& fieldName, ReaderPtr reader)
    {
        return newLucene<LimitTokenCountFilter>(delegate->tokenStream(fieldName, reader), maxTokenCount);
    }
    
    TokenStreamPtr LimitTokenCountAnalyzer::reusableTokenStream(const String& fieldName, ReaderPtr reader)
    {
        return newLucene<LimitTokenCountFilter>(delegate->reusableTokenStream(fieldName, reader), maxTokenCount);
    }
    
    int32_t LimitTokenCountAnalyzer::getPositionIncrementGap(const String& fieldName)
    {
        return delegate->getPositionIncrementGap(fieldName);
    }
    
    int32_t LimitTokenCountAnalyzer::getOffsetGap(FieldablePtr field)
    {
        return delegate->getOffsetGap(field);
    }
    
    String LimitTokenCountAnalyzer::toString()
    {
        return L"LimitTokenCountAnalyzer(" + delegate->toString() + L", maxTokenCount=" + StringUtils::toString(maxTokenCount) + L")";
    }
}
