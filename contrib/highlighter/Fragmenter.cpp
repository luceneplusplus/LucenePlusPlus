/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Fragmenter.h"

namespace Lucene
{
    Fragmenter::~Fragmenter()
    {
    }
    
    void Fragmenter::start(const String& originalText, TokenStreamPtr tokenStream)
    {
        BOOST_ASSERT(false);
        // override
    }
    
    bool Fragmenter::isNewFragment()
    {
        BOOST_ASSERT(false);
        return false; // override
    }
}
