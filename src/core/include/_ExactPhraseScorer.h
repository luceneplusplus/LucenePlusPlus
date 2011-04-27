/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _EXACTPHRASESCORER_H
#define _EXACTPHRASESCORER_H

#include "LuceneObject.h"

namespace Lucene
{
    class ChunkState : public LuceneObject
    {
    public:
        ChunkState(TermPositionsPtr posEnum, int32_t offset, bool useAdvance);
        virtual ~ChunkState();
    
        LUCENE_CLASS(ChunkState);
    
    public:
        TermPositionsPtr posEnum;
        int32_t offset;
        bool useAdvance;
        int32_t posUpto;
        int32_t posLimit;
        int32_t pos;
        int32_t lastPos;
    };
}

#endif
