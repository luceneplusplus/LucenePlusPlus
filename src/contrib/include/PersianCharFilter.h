/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef PERSIANCHARFILTER_H
#define PERSIANCHARFILTER_H

#include "CharFilter.h"

namespace Lucene
{
    /// CharFilter that replaces instances of Zero-width non-joiner with an
    /// ordinary space.
    class LPPCONTRIBAPI PersianCharFilter : public CharFilter
    {
    public:
        PersianCharFilter(CharStreamPtr in);
        virtual ~PersianCharFilter();

        LUCENE_CLASS(PersianCharFilter);

    public:
        virtual int32_t read(wchar_t* buffer, int32_t offset, int32_t length);
    };
}

#endif

