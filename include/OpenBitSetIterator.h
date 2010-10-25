/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef OPENBITSETITERATOR_H
#define OPENBITSETITERATOR_H

#include "DocIdBitSet.h"

namespace Lucene
{
    /// An iterator to iterate over set bits in an OpenBitSet.
    class LPPAPI OpenBitSetIterator : public DocIdBitSetIterator
    {
    public:
        OpenBitSetIterator(OpenBitSetPtr bitSet);
        virtual ~OpenBitSetIterator();
        
        LUCENE_CLASS(OpenBitSetIterator);
    };
}

#endif
