/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene
{
    // todo
    class LPPAPI MemoryIndex : public LuceneObject
    {
    public:
        virtual ~MemoryIndex();
        LUCENE_CLASS(MemoryIndex);

    public:
        IndexSearcherPtr createSearcher();
        void addField(const String& fieldName, TokenStreamPtr stream);
    };
}
