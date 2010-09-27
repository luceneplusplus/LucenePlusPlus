/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MemoryIndex.h"

namespace Lucene
{
    MemoryIndex::~MemoryIndex()
    {
    }
    
    IndexSearcherPtr MemoryIndex::createSearcher()
    {
        return IndexSearcherPtr(); // todo
    }
    
    void MemoryIndex::addField(const String& fieldName, TokenStreamPtr stream)
    {
        // todo
    }
}
