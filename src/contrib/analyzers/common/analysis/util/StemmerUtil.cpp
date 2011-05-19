/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "StemmerUtil.h"
#include "MiscUtils.h"

namespace Lucene
{
    StemmerUtil::~StemmerUtil()
    {
    }

    bool StemmerUtil::startsWith(wchar_t* s, int32_t len, const String& prefix)
    {
        if (prefix.length() == 1 && len < 4) // wa- prefix requires at least 3 characters
            return false;
        else if (len < (int32_t)prefix.length() + 2) // other prefixes require only 2
            return false;
        else
        {
            for (int32_t i = 0; i < (int32_t)prefix.length(); ++i)
            {
                if (s[i] != prefix[i])
                    return false;
            }
            return true;
        }
    }

    bool StemmerUtil::endsWith(wchar_t* s, int32_t len, const String& suffix)
    {
        if (len < (int32_t)suffix.length() + 2) // all suffixes require at least 2 characters after stemming
            return false;
        else
        {
            for (int32_t i = 0; i < (int32_t)suffix.length(); ++i)
            {
                if (s[len - suffix.length() + i] != suffix[i])
                    return false;
            }
            return true;
        }
    }

    int32_t StemmerUtil::_delete(wchar_t* s, int32_t pos, int32_t len)
    {
        if (pos < len)
            MiscUtils::arrayCopy(s, pos + 1, s, pos, len - pos - 1);
        return len - 1;
    }

    int32_t StemmerUtil::_deleteN(wchar_t* s, int32_t pos, int32_t len, int32_t chars)
    {
        for (int32_t i = 0; i < chars; ++i)
            len = _delete(s, pos, len);
        return len;
    }
}

