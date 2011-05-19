/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "PersianCharFilter.h"

namespace Lucene
{
    PersianCharFilter::PersianCharFilter(CharStreamPtr in) : CharFilter(in)
    {
    }

    PersianCharFilter::~PersianCharFilter()
    {
    }

    int32_t PersianCharFilter::read(wchar_t* buffer, int32_t offset, int32_t length)
    {
        int32_t charsRead = CharFilter::read(buffer, offset, length);
        if (charsRead > 0)
        {
            int32_t end = offset + charsRead;
            while (offset < end)
            {
                if (buffer[offset] == L'\x200c')
                    buffer[offset] = L' ';
                ++offset;
            }
        }
        return charsRead;
    }
}

