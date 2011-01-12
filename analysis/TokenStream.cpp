/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "TokenStream.h"

namespace Lucene
{
    TokenStream::TokenStream()
    {
    }
    
    TokenStream::TokenStream(AttributeSourcePtr input) : AttributeSource(input)
    {
    }
    
    TokenStream::TokenStream(AttributeFactoryPtr factory) : AttributeSource(factory)
    {
    }
    
    TokenStream::~TokenStream()
    {
    }
    
    void TokenStream::end()
    {
        // do nothing by default
    }
    
    void TokenStream::reset()
    {
    }
    
    void TokenStream::close()
    {
    }
}
