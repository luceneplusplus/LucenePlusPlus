/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "PorterStemFilter.h"
#include "PorterStemmer.h"
#include "CharTermAttribute.h"
#include "KeywordAttribute.h"

namespace Lucene
{
    PorterStemFilter::PorterStemFilter(TokenStreamPtr input) : TokenFilter(input)
    {
        stemmer = newLucene<PorterStemmer>();
        termAtt = addAttribute<CharTermAttribute>();
        keywordAttr = addAttribute<KeywordAttribute>();
    }
    
    PorterStemFilter::~PorterStemFilter()
    {
    }
    
    bool PorterStemFilter::incrementToken()
    {
        if (!input->incrementToken())
            return false;
        if (!keywordAttr->isKeyword() && stemmer->stem(termAtt->buffer()))
            termAtt->copyBuffer(stemmer->getResultBuffer(), 0, stemmer->getResultLength());
        return true;
    }
}
