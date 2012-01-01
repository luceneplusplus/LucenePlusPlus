/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CHARBLOCKPOOL_H
#define CHARBLOCKPOOL_H

#include "LuceneObject.h"

namespace Lucene
{
    class CharBlockPool : public LuceneObject
    {
    public:
        CharBlockPool(DocumentsWriterPtr docWriter);
        virtual ~CharBlockPool();

        LUCENE_CLASS(CharBlockPool);

    public:
        CollectionCollection<CharArray> buffers;
        int32_t numBuffer;
        int32_t bufferUpto; // Which buffer we are up to
        int32_t charUpto; // Where we are in head buffer

        Collection<CharArray> buffer; // Current head buffer
        int32_t charOffset; // Current head offset

    protected:
        DocumentsWriterPtr docWriter;

    public:
        void reset();
        void nextBuffer();
    };
}

#endif
