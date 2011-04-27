/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "ReusableAnalyzerBase.h"
#include "Tokenizer.h"

namespace Lucene
{
    ReusableAnalyzerBase::~ReusableAnalyzerBase()
    {
    }
    
    TokenStreamPtr ReusableAnalyzerBase::reusableTokenStream(const String& fieldName, ReaderPtr reader)
    {
        TokenStreamComponentsPtr streamChain(boost::dynamic_pointer_cast<TokenStreamComponents>(getPreviousTokenStream()));
        ReaderPtr r(initReader(reader));
        if (!streamChain || !streamChain->reset(r))
        {
            streamChain = createComponents(fieldName, r);
            setPreviousTokenStream(streamChain);
        }
        return streamChain->getTokenStream();
    }
    
    TokenStreamPtr ReusableAnalyzerBase::tokenStream(const String& fieldName, ReaderPtr reader)
    {
        return createComponents(fieldName, initReader(reader))->getTokenStream();
    }
    
    ReaderPtr ReusableAnalyzerBase::initReader(ReaderPtr reader)
    {
        return reader;
    }
    
    TokenStreamComponents::TokenStreamComponents(TokenizerPtr source, TokenStreamPtr result)
    {
        this->source = source;
        this->sink = result;
    }
    
    TokenStreamComponents::TokenStreamComponents(TokenizerPtr source)
    {
        this->source = source;
        this->sink = source;
    }
    
    TokenStreamComponents::~TokenStreamComponents()
    {
    }
    
    bool TokenStreamComponents::reset(ReaderPtr reader)
    {
        source->reset(reader);
        if (sink != source)
            sink->reset(); // only reset if the sink reference is different from source
        return true;
    }
    
    TokenStreamPtr TokenStreamComponents::getTokenStream()
    {
        return sink;
    }
}
