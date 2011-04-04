/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _SCORERDOCQUEUE_H
#define _SCORERDOCQUEUE_H

#include "LuceneObject.h"

namespace Lucene
{
    class HeapedScorerDoc : public LuceneObject
    {
    public:
        HeapedScorerDoc(ScorerPtr scorer);
        HeapedScorerDoc(ScorerPtr scorer, int32_t doc);
        virtual ~HeapedScorerDoc();
    
        LUCENE_CLASS(HeapedScorerDoc);
    
    public:
        ScorerPtr scorer;
        int32_t doc;
    
    public:
        void adjust();
    };
}

#endif
