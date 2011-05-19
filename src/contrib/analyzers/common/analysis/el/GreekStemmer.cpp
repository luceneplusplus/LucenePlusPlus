/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "GreekStemmer.h"
#include "MiscUtils.h"

namespace Lucene
{
    GreekStemmer::~GreekStemmer()
    {
    }

    int32_t GreekStemmer::stem(wchar_t* s, int32_t len)
    {
        if (len < 4) // too short
            return len;

        int32_t origLen = len;

        // "short rules": if it hits one of these, it skips the "long list"
        len = rule0(s, len);
        len = rule1(s, len);
        len = rule2(s, len);
        len = rule3(s, len);
        len = rule4(s, len);
        len = rule5(s, len);
        len = rule6(s, len);
        len = rule7(s, len);
        len = rule8(s, len);
        len = rule9(s, len);
        len = rule10(s, len);
        len = rule11(s, len);
        len = rule12(s, len);
        len = rule13(s, len);
        len = rule14(s, len);
        len = rule15(s, len);
        len = rule16(s, len);
        len = rule17(s, len);
        len = rule18(s, len);
        len = rule19(s, len);
        len = rule20(s, len);

        // "long list"
        if (len == origLen)
            len = rule21(s, len);

        return rule22(s, len);
    }

    int32_t GreekStemmer::rule0(wchar_t* s, int32_t len)
    {
        if (len > 9 && (endsWith(s, len, L"\x03ba\x03b1\x03b8\x03b5\x03c3\x03c4\x03c9\x03c4\x03bf\x03c3") ||
            endsWith(s, len, L"\x03ba\x03b1\x03b8\x03b5\x03c3\x03c4\x03c9\x03c4\x03c9\x03bd")))
            return len - 4;

        if (len > 8 && (endsWith(s, len, L"\x03b3\x03b5\x03b3\x03bf\x03bd\x03bf\x03c4\x03bf\x03c3") ||
            endsWith(s, len, L"\x03b3\x03b5\x03b3\x03bf\x03bd\x03bf\x03c4\x03c9\x03bd")))
            return len - 4;

        if (len > 8 && endsWith(s, len, L"\x03ba\x03b1\x03b8\x03b5\x03c3\x03c4\x03c9\x03c4\x03b1"))
            return len - 3;

        if (len > 7 && (endsWith(s, len, L"\x03c4\x03b1\x03c4\x03bf\x03b3\x03b9\x03bf\x03c5") ||
            endsWith(s, len, L"\x03c4\x03b1\x03c4\x03bf\x03b3\x03b9\x03c9\x03bd")))
            return len - 4;

        if (len > 7 && endsWith(s, len, L"\x03b3\x03b5\x03b3\x03bf\x03bd\x03bf\x03c4\x03b1"))
            return len - 3;

        if (len > 7 && endsWith(s, len, L"\x03ba\x03b1\x03b8\x03b5\x03c3\x03c4\x03c9\x03c3"))
            return len - 2;

        if (len > 6 && (endsWith(s, len, L"\x03c3\x03ba\x03b1\x03b3\x03b9\x03bf\x03c5")) ||
            endsWith(s, len, L"\x03c3\x03ba\x03b1\x03b3\x03b9\x03c9\x03bd") ||
            endsWith(s, len, L"\x03bf\x03bb\x03bf\x03b3\x03b9\x03bf\x03c5") ||
            endsWith(s, len, L"\x03bf\x03bb\x03bf\x03b3\x03b9\x03c9\x03bd") ||
            endsWith(s, len, L"\x03ba\x03c1\x03b5\x03b1\x03c4\x03bf\x03c3") ||
            endsWith(s, len, L"\x03ba\x03c1\x03b5\x03b1\x03c4\x03c9\x03bd") ||
            endsWith(s, len, L"\x03c0\x03b5\x03c1\x03b1\x03c4\x03bf\x03c3") ||
            endsWith(s, len, L"\x03c0\x03b5\x03c1\x03b1\x03c4\x03c9\x03bd") ||
            endsWith(s, len, L"\x03c4\x03b5\x03c1\x03b1\x03c4\x03bf\x03c3") ||
            endsWith(s, len, L"\x03c4\x03b5\x03c1\x03b1\x03c4\x03c9\x03bd"))
            return len - 4;

        if (len > 6 && endsWith(s, len, L"\x03c4\x03b1\x03c4\x03bf\x03b3\x03b9\x03b1"))
            return len - 3;

        if (len > 6 && endsWith(s, len, L"\x03b3\x03b5\x03b3\x03bf\x03bd\x03bf\x03c3"))
            return len - 2;

        if (len > 5 && (endsWith(s, len, L"\x03c6\x03b1\x03b3\x03b9\x03bf\x03c5") ||
            endsWith(s, len, L"\x03c6\x03b1\x03b3\x03b9\x03c9\x03bd") ||
            endsWith(s, len, L"\x03c3\x03bf\x03b3\x03b9\x03bf\x03c5") ||
            endsWith(s, len, L"\x03c3\x03bf\x03b3\x03b9\x03bf\x03c5")))
            return len - 4;

        if (len > 5 && (endsWith(s, len, L"\x03c3\x03ba\x03b1\x03b3\x03b9\x03b1") ||
            endsWith(s, len, L"\x03bf\x03bb\x03bf\x03b3\x03b9\x03b1") ||
            endsWith(s, len, L"\x03ba\x03c1\x03b5\x03b1\x03c4\x03b1") ||
            endsWith(s, len, L"\x03c0\x03b5\x03c1\x03b1\x03c4\x03b1") ||
            endsWith(s, len, L"\x03c4\x03b5\x03c1\x03b1\x03c4\x03b1")))
            return len - 3;

        if (len > 4 && (endsWith(s, len, L"\x03c6\x03b1\x03b3\x03b9\x03b1") ||
            endsWith(s, len, L"\x03c3\x03bf\x03b3\x03b9\x03b1") ||
            endsWith(s, len, L"\x03c6\x03c9\x03c4\x03bf\x03c3") ||
            endsWith(s, len, L"\x03c6\x03c9\x03c4\x03c9\x03bd")))
            return len - 3;

        if (len > 4 && (endsWith(s, len, L"\x03ba\x03c1\x03b5\x03b1\x03c3") ||
            endsWith(s, len, L"\x03c0\x03b5\x03c1\x03b1\x03c3") ||
            endsWith(s, len, L"\x03c4\x03b5\x03c1\x03b1\x03c3")))
            return len - 2;

        if (len > 3 && endsWith(s, len, L"\x03c6\x03c9\x03c4\x03b1"))
            return len - 2;

        if (len > 2 && endsWith(s, len, L"\x03c6\x03c9\x03c3"))
            return len - 1;

        return len;
    }

    int32_t GreekStemmer::rule1(wchar_t* s, int32_t len)
    {
        if (len > 4 && (endsWith(s, len, L"\x03b1\x03b4\x03b5\x03c3") ||
            endsWith(s, len, L"\x03b1\x03b4\x03c9\x03bd")))
        {
            len -= 4;
            if (!(endsWith(s, len, L"\x03bf\x03ba") ||
                endsWith(s, len, L"\x03bc\x03b1\x03bc") ||
                endsWith(s, len, L"\x03bc\x03b1\x03bd") ||
                endsWith(s, len, L"\x03bc\x03c0\x03b1\x03bc\x03c0") ||
                endsWith(s, len, L"\x03c0\x03b1\x03c4\x03b5\x03c1") ||
                endsWith(s, len, L"\x03b3\x03b9\x03b1\x03b3\x03b9") ||
                endsWith(s, len, L"\x03bd\x03c4\x03b1\x03bd\x03c4") ||
                endsWith(s, len, L"\x03ba\x03c5\x03c1") ||
                endsWith(s, len, L"\x03b8\x03b5\x03b9") ||
                endsWith(s, len, L"\x03c0\x03b5\x03b8\x03b5\x03c1")))
                len += 2; // add back
        }
        return len;
    }

    int32_t GreekStemmer::rule2(wchar_t* s, int32_t len)
    {
        if (len > 4 && (endsWith(s, len, L"\x03b5\x03b4\x03b5\x03c3") ||
            endsWith(s, len, L"\x03b5\x03b4\x03c9\x03bd")))
        {
            len -= 4;
            if (endsWith(s, len, L"\x03bf\x03c0") ||
                endsWith(s, len, L"\x03b9\x03c0") ||
                endsWith(s, len, L"\x03b5\x03bc\x03c0") ||
                endsWith(s, len, L"\x03c5\x03c0") ||
                endsWith(s, len, L"\x03b3\x03b7\x03c0") ||
                endsWith(s, len, L"\x03b4\x03b1\x03c0") ||
                endsWith(s, len, L"\x03ba\x03c1\x03b1\x03c3\x03c0") ||
                endsWith(s, len, L"\x03bc\x03b9\x03bb"))
                len += 2; // add back
        }
        return len;
    }

    int32_t GreekStemmer::rule3(wchar_t* s, int32_t len)
    {
        if (len > 5 && (endsWith(s, len, L"\x03bf\x03c5\x03b4\x03b5\x03c3") ||
            endsWith(s, len, L"\x03bf\x03c5\x03b4\x03c9\x03bd")))
        {
            len -= 5;
            if (endsWith(s, len, L"\x03b1\x03c1\x03ba") ||
                endsWith(s, len, L"\x03ba\x03b1\x03bb\x03b9\x03b1\x03ba") ||
                endsWith(s, len, L"\x03c0\x03b5\x03c4\x03b1\x03bb") ||
                endsWith(s, len, L"\x03bb\x03b9\x03c7") ||
                endsWith(s, len, L"\x03c0\x03bb\x03b5\x03be") ||
                endsWith(s, len, L"\x03c3\x03ba") ||
                endsWith(s, len, L"\x03c3") ||
                endsWith(s, len, L"\x03c6\x03bb") ||
                endsWith(s, len, L"\x03c6\x03c1") ||
                endsWith(s, len, L"\x03b2\x03b5\x03bb") ||
                endsWith(s, len, L"\x03bb\x03bf\x03c5\x03bb") ||
                endsWith(s, len, L"\x03c7\x03bd") ||
                endsWith(s, len, L"\x03c3\x03c0") ||
                endsWith(s, len, L"\x03c4\x03c1\x03b1\x03b3") ||
                endsWith(s, len, L"\x03c6\x03b5"))
                len += 3; // add back
        }
        return len;
    }

    int32_t GreekStemmer::rule4(wchar_t* s, int32_t len)
    {
        static HashSet<String> exc4;
        if (!exc4)
        {
            exc4 = HashSet<String>::newInstance();
            exc4.add(L"\x03b8");
            exc4.add(L"\x03b4");
            exc4.add(L"\x03b5\x03bb");
            exc4.add(L"\x03b3\x03b1\x03bb");
            exc4.add(L"\x03bd");
            exc4.add(L"\x03c0");
            exc4.add(L"\x03b9\x03b4");
            exc4.add(L"\x03c0\x03b1\x03c1");
        }

        if (len > 3 && (endsWith(s, len, L"\x03b5\x03c9\x03c3") ||
            endsWith(s, len, L"\x03b5\x03c9\x03bd")))
        {
            len -= 3;
            if (exc4.contains(String(s, len)))
                ++len; // add back
        }
        return len;
    }

    int32_t GreekStemmer::rule5(wchar_t* s, int32_t len)
    {
        if (len > 2 && endsWith(s, len, L"\x03b9\x03b1"))
        {
            len -= 2;
            if (endsWithVowel(s, len))
                ++len; // add back
        }
        else if (len > 3 && (endsWith(s, len, L"\x03b9\x03bf\x03c5") ||
                 endsWith(s, len, L"\x03b9\x03c9\x03bd")))
        {
            len -= 3;
            if (endsWithVowel(s, len))
                ++len; // add back
        }
        return len;
    }

    int32_t GreekStemmer::rule6(wchar_t* s, int32_t len)
    {
        static HashSet<String> exc6;
        if (!exc6)
        {
            exc6 = HashSet<String>::newInstance();
            exc6.add(L"\x03b1\x03bb");
            exc6.add(L"\x03b1\x03b4");
            exc6.add(L"\x03b5\x03bd\x03b4");
            exc6.add(L"\x03b1\x03bc\x03b1\x03bd");
            exc6.add(L"\x03b1\x03bc\x03bc\x03bf\x03c7\x03b1\x03bb");
            exc6.add(L"\x03b7\x03b8");
            exc6.add(L"\x03b1\x03bd\x03b7\x03b8");
            exc6.add(L"\x03b1\x03bd\x03c4\x03b9\x03b4");
            exc6.add(L"\x03c6\x03c5\x03c3");
            exc6.add(L"\x03b2\x03c1\x03c9\x03bc");
            exc6.add(L"\x03b3\x03b5\x03c1");
            exc6.add(L"\x03b5\x03be\x03c9\x03b4");
            exc6.add(L"\x03ba\x03b1\x03bb\x03c0");
            exc6.add(L"\x03ba\x03b1\x03bb\x03bb\x03b9\x03bd");
            exc6.add(L"\x03ba\x03b1\x03c4\x03b1\x03b4");
            exc6.add(L"\x03bc\x03bf\x03c5\x03bb");
            exc6.add(L"\x03bc\x03c0\x03b1\x03bd");
            exc6.add(L"\x03bc\x03c0\x03b1\x03b3\x03b9\x03b1\x03c4");
            exc6.add(L"\x03bc\x03c0\x03bf\x03bb");
            exc6.add(L"\x03bc\x03c0\x03bf\x03c3");
            exc6.add(L"\x03bd\x03b9\x03c4");
            exc6.add(L"\x03be\x03b9\x03ba");
            exc6.add(L"\x03c3\x03c5\x03bd\x03bf\x03bc\x03b7\x03bb");
            exc6.add(L"\x03c0\x03b5\x03c4\x03c3");
            exc6.add(L"\x03c0\x03b9\x03c4\x03c3");
            exc6.add(L"\x03c0\x03b9\x03ba\x03b1\x03bd\x03c4");
            exc6.add(L"\x03c0\x03bb\x03b9\x03b1\x03c4\x03c3");
            exc6.add(L"\x03c0\x03bf\x03c3\x03c4\x03b5\x03bb\x03bd");
            exc6.add(L"\x03c0\x03c1\x03c9\x03c4\x03bf\x03b4");
            exc6.add(L"\x03c3\x03b5\x03c1\x03c4");
            exc6.add(L"\x03c3\x03c5\x03bd\x03b1\x03b4");
            exc6.add(L"\x03c4\x03c3\x03b1\x03bc");
            exc6.add(L"\x03c5\x03c0\x03bf\x03b4");
            exc6.add(L"\x03c6\x03b9\x03bb\x03bf\x03bd");
            exc6.add(L"\x03c6\x03c5\x03bb\x03bf\x03b4");
            exc6.add(L"\x03c7\x03b1\x03c3");
        }

        bool removed = false;
        if (len > 3 && (endsWith(s, len, L"\x03b9\x03ba\x03b1") ||
            endsWith(s, len, L"\x03b9\x03ba\x03bf")))
        {
            len -= 3;
            removed = true;
        }
        else if (len > 4 && (endsWith(s, len, L"\x03b9\x03ba\x03bf\x03c5") ||
                 endsWith(s, len, L"\x03b9\x03ba\x03c9\x03bd")))
        {
            len -= 4;
            removed = true;
        }

        if (removed)
        {
            if (endsWithVowel(s, len) || exc6.contains(String(s, len)))
            len += 2; // add back
        }
        return len;
    }

    int32_t GreekStemmer::rule7(wchar_t* s, int32_t len)
    {
        static HashSet<String> exc7;
        if (!exc7)
        {
            exc7 = HashSet<String>::newInstance();
            exc7.add(L"\x03b1\x03bd\x03b1\x03c0");
            exc7.add(L"\x03b1\x03c0\x03bf\x03b8");
            exc7.add(L"\x03b1\x03c0\x03bf\x03ba");
            exc7.add(L"\x03b1\x03c0\x03bf\x03c3\x03c4");
            exc7.add(L"\x03b2\x03bf\x03c5\x03b2");
            exc7.add(L"\x03be\x03b5\x03b8");
            exc7.add(L"\x03bf\x03c5\x03bb");
            exc7.add(L"\x03c0\x03b5\x03b8");
            exc7.add(L"\x03c0\x03b9\x03ba\x03c1");
            exc7.add(L"\x03c0\x03bf\x03c4");
            exc7.add(L"\x03c3\x03b9\x03c7");
            exc7.add(L"\x03c7");
        }

        if (len == 5 && endsWith(s, len, L"\x03b1\x03b3\x03b1\x03bc\x03b5"))
            return len - 1;

        if (len > 7 && endsWith(s, len, L"\x03b7\x03b8\x03b7\x03ba\x03b1\x03bc\x03b5"))
            len -= 7;
        else if (len > 6 && endsWith(s, len, L"\x03bf\x03c5\x03c3\x03b1\x03bc\x03b5"))
            len -= 6;
        else if (len > 5 && (endsWith(s, len, L"\x03b1\x03b3\x03b1\x03bc\x03b5") ||
                 endsWith(s, len, L"\x03b7\x03c3\x03b1\x03bc\x03b5") ||
                 endsWith(s, len, L"\x03b7\x03ba\x03b1\x03bc\x03b5")))
        len -= 5;

        if (len > 3 && endsWith(s, len, L"\x03b1\x03bc\x03b5"))
        {
            len -= 3;
            if (exc7.contains(String(s, len)))
                len += 2; // add back
        }

        return len;
    }

    int32_t GreekStemmer::rule8(wchar_t* s, int32_t len)
    {
        static HashSet<String> exc8a;
        if (!exc8a)
        {
            exc8a = HashSet<String>::newInstance();
            exc8a.add(L"\x03c4\x03c1");
            exc8a.add(L"\x03c4\x03c3");
        }

        static HashSet<String> exc8b;
        if (!exc8b)
        {
            exc8b = HashSet<String>::newInstance();
            exc8b.add(L"\x03b2\x03b5\x03c4\x03b5\x03c1");
            exc8b.add(L"\x03b2\x03bf\x03c5\x03bb\x03ba");
            exc8b.add(L"\x03b2\x03c1\x03b1\x03c7\x03bc");
            exc8b.add(L"\x03b3");
            exc8b.add(L"\x03b4\x03c1\x03b1\x03b4\x03bf\x03c5\x03bc");
            exc8b.add(L"\x03b8");
            exc8b.add(L"\x03ba\x03b1\x03bb\x03c0\x03bf\x03c5\x03b6");
            exc8b.add(L"\x03ba\x03b1\x03c3\x03c4\x03b5\x03bb");
            exc8b.add(L"\x03ba\x03bf\x03c1\x03bc\x03bf\x03c1");
            exc8b.add(L"\x03bb\x03b1\x03bf\x03c0\x03bb");
            exc8b.add(L"\x03bc\x03c9\x03b1\x03bc\x03b5\x03b8");
            exc8b.add(L"\x03bc");
            exc8b.add(L"\x03bc\x03bf\x03c5\x03c3\x03bf\x03c5\x03bb\x03bc");
            exc8b.add(L"\x03bd");
            exc8b.add(L"\x03bf\x03c5\x03bb");
            exc8b.add(L"\x03c0");
            exc8b.add(L"\x03c0\x03b5\x03bb\x03b5\x03ba");
            exc8b.add(L"\x03c0\x03bb");
            exc8b.add(L"\x03c0\x03bf\x03bb\x03b9\x03c3");
            exc8b.add(L"\x03c0\x03bf\x03c1\x03c4\x03bf\x03bb");
            exc8b.add(L"\x03c3\x03b1\x03c1\x03b1\x03ba\x03b1\x03c4\x03c3");
            exc8b.add(L"\x03c3\x03bf\x03c5\x03bb\x03c4");
            exc8b.add(L"\x03c4\x03c3\x03b1\x03c1\x03bb\x03b1\x03c4");
            exc8b.add(L"\x03bf\x03c1\x03c6");
            exc8b.add(L"\x03c4\x03c3\x03b9\x03b3\x03b3");
            exc8b.add(L"\x03c4\x03c3\x03bf\x03c0");
            exc8b.add(L"\x03c6\x03c9\x03c4\x03bf\x03c3\x03c4\x03b5\x03c6");
            exc8b.add(L"\x03c7");
            exc8b.add(L"\x03c8\x03c5\x03c7\x03bf\x03c0\x03bb");
            exc8b.add(L"\x03b1\x03b3");
            exc8b.add(L"\x03bf\x03c1\x03c6");
            exc8b.add(L"\x03b3\x03b1\x03bb");
            exc8b.add(L"\x03b3\x03b5\x03c1");
            exc8b.add(L"\x03b4\x03b5\x03ba");
            exc8b.add(L"\x03b4\x03b9\x03c0\x03bb");
            exc8b.add(L"\x03b1\x03bc\x03b5\x03c1\x03b9\x03ba\x03b1\x03bd");
            exc8b.add(L"\x03bf\x03c5\x03c1");
            exc8b.add(L"\x03c0\x03b9\x03b8");
            exc8b.add(L"\x03c0\x03bf\x03c5\x03c1\x03b9\x03c4");
            exc8b.add(L"\x03c3");
            exc8b.add(L"\x03b6\x03c9\x03bd\x03c4");
            exc8b.add(L"\x03b9\x03ba");
            exc8b.add(L"\x03ba\x03b1\x03c3\x03c4");
            exc8b.add(L"\x03ba\x03bf\x03c0");
            exc8b.add(L"\x03bb\x03b9\x03c7");
            exc8b.add(L"\x03bb\x03bf\x03c5\x03b8\x03b7\x03c1");
            exc8b.add(L"\x03bc\x03b1\x03b9\x03bd\x03c4");
            exc8b.add(L"\x03bc\x03b5\x03bb");
            exc8b.add(L"\x03c3\x03b9\x03b3");
            exc8b.add(L"\x03c3\x03c0");
            exc8b.add(L"\x03c3\x03c4\x03b5\x03b3");
            exc8b.add(L"\x03c4\x03c1\x03b1\x03b3");
            exc8b.add(L"\x03c4\x03c3\x03b1\x03b3");
            exc8b.add(L"\x03c6");
            exc8b.add(L"\x03b5\x03c1");
            exc8b.add(L"\x03b1\x03b4\x03b1\x03c0");
            exc8b.add(L"\x03b1\x03b8\x03b9\x03b3\x03b3");
            exc8b.add(L"\x03b1\x03bc\x03b7\x03c7");
            exc8b.add(L"\x03b1\x03bd\x03b9\x03ba");
            exc8b.add(L"\x03b1\x03bd\x03bf\x03c1\x03b3");
            exc8b.add(L"\x03b1\x03c0\x03b7\x03b3");
            exc8b.add(L"\x03b1\x03c0\x03b9\x03b8");
            exc8b.add(L"\x03b1\x03c4\x03c3\x03b9\x03b3\x03b3");
            exc8b.add(L"\x03b2\x03b1\x03c3");
            exc8b.add(L"\x03b2\x03b1\x03c3\x03ba");
            exc8b.add(L"\x03b2\x03b1\x03b8\x03c5\x03b3\x03b1\x03bb");
            exc8b.add(L"\x03b2\x03b9\x03bf\x03bc\x03b7\x03c7");
            exc8b.add(L"\x03b2\x03c1\x03b1\x03c7\x03c5\x03ba");
            exc8b.add(L"\x03b4\x03b9\x03b1\x03c4");
            exc8b.add(L"\x03b4\x03b9\x03b1\x03c6");
            exc8b.add(L"\x03b5\x03bd\x03bf\x03c1\x03b3");
            exc8b.add(L"\x03b8\x03c5\x03c3");
            exc8b.add(L"\x03ba\x03b1\x03c0\x03bd\x03bf\x03b2\x03b9\x03bf\x03bc\x03b7\x03c7");
            exc8b.add(L"\x03ba\x03b1\x03c4\x03b1\x03b3\x03b1\x03bb");
            exc8b.add(L"\x03ba\x03bb\x03b9\x03b2");
            exc8b.add(L"\x03ba\x03bf\x03b9\x03bb\x03b1\x03c1\x03c6");
            exc8b.add(L"\x03bb\x03b9\x03b2");
            exc8b.add(L"\x03bc\x03b5\x03b3\x03bb\x03bf\x03b2\x03b9\x03bf\x03bc\x03b7\x03c7");
            exc8b.add(L"\x03bc\x03b9\x03ba\x03c1\x03bf\x03b2\x03b9\x03bf\x03bc\x03b7\x03c7");
            exc8b.add(L"\x03bd\x03c4\x03b1\x03b2");
            exc8b.add(L"\x03be\x03b7\x03c1\x03bf\x03ba\x03bb\x03b9\x03b2");
            exc8b.add(L"\x03bf\x03bb\x03b9\x03b3\x03bf\x03b4\x03b1\x03bc");
            exc8b.add(L"\x03bf\x03bb\x03bf\x03b3\x03b1\x03bb");
            exc8b.add(L"\x03c0\x03b5\x03bd\x03c4\x03b1\x03c1\x03c6");
            exc8b.add(L"\x03c0\x03b5\x03c1\x03b7\x03c6");
            exc8b.add(L"\x03c0\x03b5\x03c1\x03b9\x03c4\x03c1");
            exc8b.add(L"\x03c0\x03bb\x03b1\x03c4");
            exc8b.add(L"\x03c0\x03bf\x03bb\x03c5\x03b4\x03b1\x03c0");
            exc8b.add(L"\x03c0\x03bf\x03bb\x03c5\x03bc\x03b7\x03c7");
            exc8b.add(L"\x03c3\x03c4\x03b5\x03c6");
            exc8b.add(L"\x03c4\x03b1\x03b2");
            exc8b.add(L"\x03c4\x03b5\x03c4");
            exc8b.add(L"\x03c5\x03c0\x03b5\x03c1\x03b7\x03c6");
            exc8b.add(L"\x03c5\x03c0\x03bf\x03ba\x03bf\x03c0");
            exc8b.add(L"\x03c7\x03b1\x03bc\x03b7\x03bb\x03bf\x03b4\x03b1\x03c0");
            exc8b.add(L"\x03c8\x03b7\x03bb\x03bf\x03c4\x03b1\x03b2");
        }

        bool removed = false;

        if (len > 8 && endsWith(s, len, L"\x03b9\x03bf\x03c5\x03bd\x03c4\x03b1\x03bd\x03b5"))
        {
            len -= 8;
            removed = true;
        }
        else if (len > 7 && endsWith(s, len, L"\x03b9\x03bf\x03bd\x03c4\x03b1\x03bd\x03b5") ||
                 endsWith(s, len, L"\x03bf\x03c5\x03bd\x03c4\x03b1\x03bd\x03b5") ||
                 endsWith(s, len, L"\x03b7\x03b8\x03b7\x03ba\x03b1\x03bd\x03b5"))
        {
            len -= 7;
            removed = true;
        }
        else if (len > 6 && endsWith(s, len, L"\x03b9\x03bf\x03c4\x03b1\x03bd\x03b5") ||
                 endsWith(s, len, L"\x03bf\x03bd\x03c4\x03b1\x03bd\x03b5") ||
                 endsWith(s, len, L"\x03bf\x03c5\x03c3\x03b1\x03bd\x03b5"))
        {
            len -= 6;
            removed = true;
        }
        else if (len > 5 && endsWith(s, len, L"\x03b1\x03b3\x03b1\x03bd\x03b5") ||
                 endsWith(s, len, L"\x03b7\x03c3\x03b1\x03bd\x03b5") ||
                 endsWith(s, len, L"\x03bf\x03c4\x03b1\x03bd\x03b5") ||
                 endsWith(s, len, L"\x03b7\x03ba\x03b1\x03bd\x03b5"))
        {
            len -= 5;
            removed = true;
        }

        if (removed && exc8a.contains(String(s, len)))
        {
            len += 4;
            s[len - 4] = L'\x03b1';
            s[len - 3] = L'\x03b3';
            s[len - 2] = L'\x03b1';
            s[len - 1] = L'\x03bd';
        }

        if (len > 3 && endsWith(s, len, L"\x03b1\x03bd\x03b5"))
        {
            len -= 3;
            if (endsWithVowelNoY(s, len) || exc8b.contains(String(s, len)))
                len += 2; // add back
        }

        return len;
    }

    int32_t GreekStemmer::rule9(wchar_t* s, int32_t len)
    {
        static HashSet<String> exc9;
        if (!exc9)
        {
            exc9 = HashSet<String>::newInstance();
            exc9.add(L"\x03b1\x03b2\x03b1\x03c1");
            exc9.add(L"\x03b2\x03b5\x03bd");
            exc9.add(L"\x03b5\x03bd\x03b1\x03c1");
            exc9.add(L"\x03b1\x03b2\x03c1");
            exc9.add(L"\x03b1\x03b4");
            exc9.add(L"\x03b1\x03b8");
            exc9.add(L"\x03b1\x03bd");
            exc9.add(L"\x03b1\x03c0\x03bb");
            exc9.add(L"\x03b2\x03b1\x03c1\x03bf\x03bd");
            exc9.add(L"\x03bd\x03c4\x03c1");
            exc9.add(L"\x03c3\x03ba");
            exc9.add(L"\x03ba\x03bf\x03c0");
            exc9.add(L"\x03bc\x03c0\x03bf\x03c1");
            exc9.add(L"\x03bd\x03b9\x03c6");
            exc9.add(L"\x03c0\x03b1\x03b3");
            exc9.add(L"\x03c0\x03b1\x03c1\x03b1\x03ba\x03b1\x03bb");
            exc9.add(L"\x03c3\x03b5\x03c1\x03c0");
            exc9.add(L"\x03c3\x03ba\x03b5\x03bb");
            exc9.add(L"\x03c3\x03c5\x03c1\x03c6");
            exc9.add(L"\x03c4\x03bf\x03ba");
            exc9.add(L"\x03c5");
            exc9.add(L"\x03b4");
            exc9.add(L"\x03b5\x03bc");
            exc9.add(L"\x03b8\x03b1\x03c1\x03c1");
            exc9.add(L"\x03b8");
        }

        if (len > 5 && endsWith(s, len, L"\x03b7\x03c3\x03b5\x03c4\x03b5"))
            len -= 5;

        if (len > 3 && endsWith(s, len, L"\x03b5\x03c4\x03b5"))
        {
            len -= 3;
            if (exc9.contains(String(s, len)) ||
                endsWithVowelNoY(s, len) ||
                endsWith(s, len, L"\x03bf\x03b4") ||
                endsWith(s, len, L"\x03b1\x03b9\x03c1") ||
                endsWith(s, len, L"\x03c6\x03bf\x03c1") ||
                endsWith(s, len, L"\x03c4\x03b1\x03b8") ||
                endsWith(s, len, L"\x03b4\x03b9\x03b1\x03b8") ||
                endsWith(s, len, L"\x03c3\x03c7") ||
                endsWith(s, len, L"\x03b5\x03bd\x03b4") ||
                endsWith(s, len, L"\x03b5\x03c5\x03c1") ||
                endsWith(s, len, L"\x03c4\x03b9\x03b8") ||
                endsWith(s, len, L"\x03c5\x03c0\x03b5\x03c1\x03b8") ||
                endsWith(s, len, L"\x03c1\x03b1\x03b8") ||
                endsWith(s, len, L"\x03b5\x03bd\x03b8") ||
                endsWith(s, len, L"\x03c1\x03bf\x03b8") ||
                endsWith(s, len, L"\x03c3\x03b8") ||
                endsWith(s, len, L"\x03c0\x03c5\x03c1") ||
                endsWith(s, len, L"\x03b1\x03b9\x03bd") ||
                endsWith(s, len, L"\x03c3\x03c5\x03bd\x03b4") ||
                endsWith(s, len, L"\x03c3\x03c5\x03bd") ||
                endsWith(s, len, L"\x03c3\x03c5\x03bd\x03b8") ||
                endsWith(s, len, L"\x03c7\x03c9\x03c1") ||
                endsWith(s, len, L"\x03c0\x03bf\x03bd") ||
                endsWith(s, len, L"\x03b2\x03c1") ||
                endsWith(s, len, L"\x03ba\x03b1\x03b8") ||
                endsWith(s, len, L"\x03b5\x03c5\x03b8") ||
                endsWith(s, len, L"\x03b5\x03ba\x03b8") ||
                endsWith(s, len, L"\x03bd\x03b5\x03c4") ||
                endsWith(s, len, L"\x03c1\x03bf\x03bd") ||
                endsWith(s, len, L"\x03b1\x03c1\x03ba") ||
                endsWith(s, len, L"\x03b2\x03b1\x03c1") ||
                endsWith(s, len, L"\x03b2\x03bf\x03bb") ||
                endsWith(s, len, L"\x03c9\x03c6\x03b5\x03bb"))
                len += 2; // add back
        }

        return len;
    }

    int32_t GreekStemmer::rule10(wchar_t* s, int32_t len)
    {
        if (len > 5 && (endsWith(s, len, L"\x03bf\x03bd\x03c4\x03b1\x03c3") ||
            endsWith(s, len, L"\x03c9\x03bd\x03c4\x03b1\x03c3")))
        {
            len -= 5;
            if (len == 3 && endsWith(s, len, L"\x03b1\x03c1\x03c7"))
            {
                len += 3; // add back
                s[len - 3] = L'\x03bf';
            }
            if (endsWith(s, len, L"\x03ba\x03c1\x03b5"))
            {
                len += 3; // add back
                s[len - 3] = L'\x03c9';
            }
        }

        return len;
    }

    int32_t GreekStemmer::rule11(wchar_t* s, int32_t len)
    {
        if (len > 6 && endsWith(s, len, L"\x03bf\x03bc\x03b1\x03c3\x03c4\x03b5"))
        {
            len -= 6;
            if (len == 2 && endsWith(s, len, L"\x03bf\x03bd"))
                len += 5; // add back
        }
        else if (len > 7 && endsWith(s, len, L"\x03b9\x03bf\x03bc\x03b1\x03c3\x03c4\x03b5"))
        {
            len -= 7;
            if (len == 2 && endsWith(s, len, L"\x03bf\x03bd"))
            {
                len += 5;
                s[len - 5] = L'\x03bf';
                s[len - 4] = L'\x03bc';
                s[len - 3] = L'\x03b1';
                s[len - 2] = L'\x03c3';
                s[len - 1] = L'\x03c4';
            }
        }
        return len;
    }

    int32_t GreekStemmer::rule12(wchar_t* s, int32_t len)
    {
        static HashSet<String> exc12a;
        if (!exc12a)
        {
            exc12a = HashSet<String>::newInstance();
            exc12a.add(L"\x03c0");
            exc12a.add(L"\x03b1\x03c0");
            exc12a.add(L"\x03c3\x03c5\x03bc\x03c0");
            exc12a.add(L"\x03b1\x03c3\x03c5\x03bc\x03c0");
            exc12a.add(L"\x03b1\x03ba\x03b1\x03c4\x03b1\x03c0");
            exc12a.add(L"\x03b1\x03bc\x03b5\x03c4\x03b1\x03bc\x03c6");
        }

        static HashSet<String> exc12b;
        if (!exc12b)
        {
            exc12b = HashSet<String>::newInstance();
            exc12b.add(L"\x03b1\x03bb");
            exc12b.add(L"\x03b1\x03c1");
            exc12b.add(L"\x03b5\x03ba\x03c4\x03b5\x03bb");
            exc12b.add(L"\x03b6");
            exc12b.add(L"\x03bc");
            exc12b.add(L"\x03be");
            exc12b.add(L"\x03c0\x03b1\x03c1\x03b1\x03ba\x03b1\x03bb");
            exc12b.add(L"\x03b1\x03c1");
            exc12b.add(L"\x03c0\x03c1\x03bf");
            exc12b.add(L"\x03bd\x03b9\x03c3");
        }

        if (len > 5 && endsWith(s, len, L"\x03b9\x03b5\x03c3\x03c4\x03b5"))
        {
            len -= 5;
            if (exc12a.contains(String(s, len)))
                len += 4; // add back
        }

        if (len > 4 && endsWith(s, len, L"\x03b5\x03c3\x03c4\x03b5"))
        {
            len -= 4;
            if (exc12b.contains(String(s, len)))
                len += 3; // add back
        }

        return len;
    }

    int32_t GreekStemmer::rule13(wchar_t* s, int32_t len)
    {
        static HashSet<String> exc13;
        if (!exc13)
        {
            exc13 = HashSet<String>::newInstance();
            exc13.add(L"\x03b4\x03b9\x03b1\x03b8");
            exc13.add(L"\x03b8");
            exc13.add(L"\x03c0\x03b1\x03c1\x03b1\x03ba\x03b1\x03c4\x03b1\x03b8");
            exc13.add(L"\x03c0\x03c1\x03bf\x03c3\x03b8");
            exc13.add(L"\x03c3\x03c5\x03bd\x03b8");
        }

        if (len > 6 && endsWith(s, len, L"\x03b7\x03b8\x03b7\x03ba\x03b5\x03c3"))
            len -= 6;
        else if (len > 5 && (endsWith(s, len, L"\x03b7\x03b8\x03b7\x03ba\x03b1") ||
                 endsWith(s, len, L"\x03b7\x03b8\x03b7\x03ba\x03b5")))
            len -= 5;

        bool removed = false;

        if (len > 4 && endsWith(s, len, L"\x03b7\x03ba\x03b5\x03c3"))
        {
            len -= 4;
            removed = true;
        }
        else if (len > 3 && (endsWith(s, len, L"\x03b7\x03ba\x03b1") ||
                 endsWith(s, len, L"\x03b7\x03ba\x03b5")))
        {
            len -= 3;
            removed = true;
        }

        if (removed && (exc13.contains(String(s, len)) ||
            endsWith(s, len, L"\x03c3\x03ba\x03c9\x03bb") ||
            endsWith(s, len, L"\x03c3\x03ba\x03bf\x03c5\x03bb") ||
            endsWith(s, len, L"\x03bd\x03b1\x03c1\x03b8") ||
            endsWith(s, len, L"\x03c3\x03c6") ||
            endsWith(s, len, L"\x03bf\x03b8") ||
            endsWith(s, len, L"\x03c0\x03b9\x03b8")))
            len += 2;

        return len;
    }

    int32_t GreekStemmer::rule14(wchar_t* s, int32_t len)
    {
        static HashSet<String> exc14;
        if (!exc14)
        {
            exc14 = HashSet<String>::newInstance();
            exc14.add(L"\x03c6\x03b1\x03c1\x03bc\x03b1\x03ba");
            exc14.add(L"\x03c7\x03b1\x03b4");
            exc14.add(L"\x03b1\x03b3\x03ba");
            exc14.add(L"\x03b1\x03bd\x03b1\x03c1\x03c1");
            exc14.add(L"\x03b2\x03c1\x03bf\x03bc");
            exc14.add(L"\x03b5\x03ba\x03bb\x03b9\x03c0");
            exc14.add(L"\x03bb\x03b1\x03bc\x03c0\x03b9\x03b4");
            exc14.add(L"\x03bb\x03b5\x03c7");
            exc14.add(L"\x03bc");
            exc14.add(L"\x03c0\x03b1\x03c4");
            exc14.add(L"\x03c1");
            exc14.add(L"\x03bb");
            exc14.add(L"\x03bc\x03b5\x03b4");
            exc14.add(L"\x03bc\x03b5\x03c3\x03b1\x03b6");
            exc14.add(L"\x03c5\x03c0\x03bf\x03c4\x03b5\x03b9\x03bd");
            exc14.add(L"\x03b1\x03bc");
            exc14.add(L"\x03b1\x03b9\x03b8");
            exc14.add(L"\x03b1\x03bd\x03b7\x03ba");
            exc14.add(L"\x03b4\x03b5\x03c3\x03c0\x03bf\x03b6");
            exc14.add(L"\x03b5\x03bd\x03b4\x03b9\x03b1\x03c6\x03b5\x03c1");
            exc14.add(L"\x03b4\x03b5");
            exc14.add(L"\x03b4\x03b5\x03c5\x03c4\x03b5\x03c1\x03b5\x03c5");
            exc14.add(L"\x03ba\x03b1\x03b8\x03b1\x03c1\x03b5\x03c5");
            exc14.add(L"\x03c0\x03bb\x03b5");
            exc14.add(L"\x03c4\x03c3\x03b1");
        }

        bool removed = false;

        if (len > 5 && endsWith(s, len, L"\x03bf\x03c5\x03c3\x03b5\x03c3"))
        {
            len -= 5;
            removed = true;
        }
        else if (len > 4 && (endsWith(s, len, L"\x03bf\x03c5\x03c3\x03b1") ||
                 endsWith(s, len, L"\x03bf\x03c5\x03c3\x03b5")))
        {
            len -= 4;
            removed = true;
        }

        if (removed && (exc14.contains(String(s, len)) || endsWithVowel(s, len) ||
                        endsWith(s, len, L"\x03c0\x03bf\x03b4\x03b1\x03c1") ||
                        endsWith(s, len, L"\x03b2\x03bb\x03b5\x03c0") ||
                        endsWith(s, len, L"\x03c0\x03b1\x03bd\x03c4\x03b1\x03c7") ||
                        endsWith(s, len, L"\x03c6\x03c1\x03c5\x03b4") ||
                        endsWith(s, len, L"\x03bc\x03b1\x03bd\x03c4\x03b9\x03bb") ||
                        endsWith(s, len, L"\x03bc\x03b1\x03bb\x03bb") ||
                        endsWith(s, len, L"\x03ba\x03c5\x03bc\x03b1\x03c4") ||
                        endsWith(s, len, L"\x03bb\x03b1\x03c7") ||
                        endsWith(s, len, L"\x03bb\x03b7\x03b3") ||
                        endsWith(s, len, L"\x03c6\x03b1\x03b3") ||
                        endsWith(s, len, L"\x03bf\x03bc") ||
                        endsWith(s, len, L"\x03c0\x03c1\x03c9\x03c4")))
            len += 3; // add back

        return len;
    }

    int32_t GreekStemmer::rule15(wchar_t* s, int32_t len)
    {
        static HashSet<String> exc15a;
        if (!exc15a)
        {
            exc15a = HashSet<String>::newInstance();
            exc15a.add(L"\x03b1\x03b2\x03b1\x03c3\x03c4");
            exc15a.add(L"\x03c0\x03bf\x03bb\x03c5\x03c6");
            exc15a.add(L"\x03b1\x03b4\x03b7\x03c6");
            exc15a.add(L"\x03c0\x03b1\x03bc\x03c6");
            exc15a.add(L"\x03c1");
            exc15a.add(L"\x03b1\x03c3\x03c0");
            exc15a.add(L"\x03b1\x03c6");
            exc15a.add(L"\x03b1\x03bc\x03b1\x03bb");
            exc15a.add(L"\x03b1\x03bc\x03b1\x03bb\x03bb\x03b9");
            exc15a.add(L"\x03b1\x03bd\x03c5\x03c3\x03c4");
            exc15a.add(L"\x03b1\x03c0\x03b5\x03c1");
            exc15a.add(L"\x03b1\x03c3\x03c0\x03b1\x03c1");
            exc15a.add(L"\x03b1\x03c7\x03b1\x03c1");
            exc15a.add(L"\x03b4\x03b5\x03c1\x03b2\x03b5\x03bd");
            exc15a.add(L"\x03b4\x03c1\x03bf\x03c3\x03bf\x03c0");
            exc15a.add(L"\x03be\x03b5\x03c6");
            exc15a.add(L"\x03bd\x03b5\x03bf\x03c0");
            exc15a.add(L"\x03bd\x03bf\x03bc\x03bf\x03c4");
            exc15a.add(L"\x03bf\x03bb\x03bf\x03c0");
            exc15a.add(L"\x03bf\x03bc\x03bf\x03c4");
            exc15a.add(L"\x03c0\x03c1\x03bf\x03c3\x03c4");
            exc15a.add(L"\x03c0\x03c1\x03bf\x03c3\x03c9\x03c0\x03bf\x03c0");
            exc15a.add(L"\x03c3\x03c5\x03bc\x03c0");
            exc15a.add(L"\x03c3\x03c5\x03bd\x03c4");
            exc15a.add(L"\x03c4");
            exc15a.add(L"\x03c5\x03c0\x03bf\x03c4");
            exc15a.add(L"\x03c7\x03b1\x03c1");
            exc15a.add(L"\x03b1\x03b5\x03b9\x03c0");
            exc15a.add(L"\x03b1\x03b9\x03bc\x03bf\x03c3\x03c4");
            exc15a.add(L"\x03b1\x03bd\x03c5\x03c0");
            exc15a.add(L"\x03b1\x03c0\x03bf\x03c4");
            exc15a.add(L"\x03b1\x03c1\x03c4\x03b9\x03c0");
            exc15a.add(L"\x03b4\x03b9\x03b1\x03c4");
            exc15a.add(L"\x03b5\x03bd");
            exc15a.add(L"\x03b5\x03c0\x03b9\x03c4");
            exc15a.add(L"\x03ba\x03c1\x03bf\x03ba\x03b1\x03bb\x03bf\x03c0");
            exc15a.add(L"\x03c3\x03b9\x03b4\x03b7\x03c1\x03bf\x03c0");
            exc15a.add(L"\x03bb");
            exc15a.add(L"\x03bd\x03b1\x03c5");
            exc15a.add(L"\x03bf\x03c5\x03bb\x03b1\x03bc");
            exc15a.add(L"\x03bf\x03c5\x03c1");
            exc15a.add(L"\x03c0");
            exc15a.add(L"\x03c4\x03c1");
            exc15a.add(L"\x03bc");
        }

        static HashSet<String> exc15b;
        if (!exc15b)
        {
            exc15b = HashSet<String>::newInstance();
            exc15b.add(L"\x03c8\x03bf\x03c6");
            exc15b.add(L"\x03bd\x03b1\x03c5\x03bb\x03bf\x03c7");
        }

        bool removed = false;
        if (len > 4 && endsWith(s, len, L"\x03b1\x03b3\x03b5\x03c3"))
        {
            len -= 4;
            removed = true;
        }
        else if (len > 3 && (endsWith(s, len, L"\x03b1\x03b3\x03b1") ||
                 endsWith(s, len, L"\x03b1\x03b3\x03b5")))
        {
            len -= 3;
            removed = true;
        }

        if (removed)
        {
            bool cond1 = exc15a.contains(String(s, len)) ||
                         endsWith(s, len, L"\x03bf\x03c6") ||
                         endsWith(s, len, L"\x03c0\x03b5\x03bb") ||
                         endsWith(s, len, L"\x03c7\x03bf\x03c1\x03c4") ||
                         endsWith(s, len, L"\x03bb\x03bb") ||
                         endsWith(s, len, L"\x03c3\x03c6") ||
                         endsWith(s, len, L"\x03c1\x03c0") ||
                         endsWith(s, len, L"\x03c6\x03c1") ||
                         endsWith(s, len, L"\x03c0\x03c1") ||
                         endsWith(s, len, L"\x03bb\x03bf\x03c7") ||
                         endsWith(s, len, L"\x03c3\x03bc\x03b7\x03bd");

            bool cond2 = exc15b.contains(String(s, len)) ||
                         endsWith(s, len, L"\x03ba\x03bf\x03bb\x03bb");

            if (cond1 && !cond2)
                len += 2; // add back
        }

        return len;
    }

    int32_t GreekStemmer::rule16(wchar_t* s, int32_t len)
    {
        static HashSet<String> exc16;
        if (!exc16)
        {
            exc16 = HashSet<String>::newInstance();
            exc16.add(L"\x03bd");
            exc16.add(L"\x03c7\x03b5\x03c1\x03c3\x03bf\x03bd");
            exc16.add(L"\x03b4\x03c9\x03b4\x03b5\x03ba\x03b1\x03bd");
            exc16.add(L"\x03b5\x03c1\x03b7\x03bc\x03bf\x03bd");
            exc16.add(L"\x03bc\x03b5\x03b3\x03b1\x03bb\x03bf\x03bd");
            exc16.add(L"\x03b5\x03c0\x03c4\x03b1\x03bd");
        }

        bool removed = false;
        if (len > 4 && endsWith(s, len, L"\x03b7\x03c3\x03bf\x03c5"))
        {
            len -= 4;
            removed = true;
        }
        else if (len > 3 && (endsWith(s, len, L"\x03b7\x03c3\x03b5") ||
                 endsWith(s, len, L"\x03b7\x03c3\x03b1")))
        {
            len -= 3;
            removed = true;
        }

        if (removed && exc16.contains(String(s, len)))
            len += 2; // add back

        return len;
    }

    int32_t GreekStemmer::rule17(wchar_t* s, int32_t len)
    {
        static HashSet<String> exc17;
        if (!exc17)
        {
            exc17 = HashSet<String>::newInstance();
            exc17.add(L"\x03b1\x03c3\x03b2");
            exc17.add(L"\x03c3\x03b2");
            exc17.add(L"\x03b1\x03c7\x03c1");
            exc17.add(L"\x03c7\x03c1");
            exc17.add(L"\x03b1\x03c0\x03bb");
            exc17.add(L"\x03b1\x03b5\x03b9\x03bc\x03bd");
            exc17.add(L"\x03b4\x03c5\x03c3\x03c7\x03c1");
            exc17.add(L"\x03b5\x03c5\x03c7\x03c1");
            exc17.add(L"\x03ba\x03bf\x03b9\x03bd\x03bf\x03c7\x03c1");
            exc17.add(L"\x03c0\x03b1\x03bb\x03b9\x03bc\x03c8");
        }

        if (len > 4 && endsWith(s, len, L"\x03b7\x03c3\x03c4\x03b5"))
        {
            len -= 4;
            if (exc17.contains(String(s, len)))
                len += 3; // add back
        }

        return len;
    }

    int32_t GreekStemmer::rule18(wchar_t* s, int32_t len)
    {
        static HashSet<String> exc18;
        if (!exc18)
        {
            exc18 = HashSet<String>::newInstance();
            exc18.add(L"\x03bd");
            exc18.add(L"\x03c1");
            exc18.add(L"\x03c3\x03c0\x03b9");
            exc18.add(L"\x03c3\x03c4\x03c1\x03b1\x03b2\x03bf\x03bc\x03bf\x03c5\x03c4\x03c3");
            exc18.add(L"\x03ba\x03b1\x03ba\x03bf\x03bc\x03bf\x03c5\x03c4\x03c3");
            exc18.add(L"\x03b5\x03be\x03c9\x03bd");
        }

        bool removed = false;

        if (len > 6 && (endsWith(s, len, L"\x03b7\x03c3\x03bf\x03c5\x03bd\x03b5") ||
                        endsWith(s, len, L"\x03b7\x03b8\x03bf\x03c5\x03bd\x03b5")))
        {
            len -= 6;
            removed = true;
        }
        else if (len > 4 && endsWith(s, len, L"\x03bf\x03c5\x03bd\x03b5"))
        {
            len -= 4;
            removed = true;
        }

        if (removed && exc18.contains(String(s, len)))
        {
            len += 3;
            s[len - 3] = L'\x03bf';
            s[len - 2] = L'\x03c5';
            s[len - 1] = L'\x03bd';
        }

        return len;
    }

    int32_t GreekStemmer::rule19(wchar_t* s, int32_t len)
    {
        static HashSet<String> exc19;
        if (!exc19)
        {
            exc19 = HashSet<String>::newInstance();
            exc19.add(L"\x03c0\x03b1\x03c1\x03b1\x03c3\x03bf\x03c5\x03c3");
            exc19.add(L"\x03c6");
            exc19.add(L"\x03c7");
            exc19.add(L"\x03c9\x03c1\x03b9\x03bf\x03c0\x03bb");
            exc19.add(L"\x03b1\x03b6");
            exc19.add(L"\x03b1\x03bb\x03bb\x03bf\x03c3\x03bf\x03c5\x03c3");
            exc19.add(L"\x03b1\x03c3\x03bf\x03c5\x03c3");
        }

        bool removed = false;

        if (len > 6 && (endsWith(s, len, L"\x03b7\x03c3\x03bf\x03c5\x03bc\x03b5") ||
                        endsWith(s, len, L"\x03b7\x03b8\x03bf\x03c5\x03bc\x03b5")))
        {
            len -= 6;
            removed = true;
        }
        else if (len > 4 && endsWith(s, len, L"\x03bf\x03c5\x03bc\x03b5"))
        {
            len -= 4;
            removed = true;
        }

        if (removed && exc19.contains(String(s, len)))
        {
            len += 3;
            s[len - 3] = L'\x03bf';
            s[len - 2] = L'\x03c5';
            s[len - 1] = L'\x03bc';
        }

        return len;
    }

    int32_t GreekStemmer::rule20(wchar_t* s, int32_t len)
    {
        if (len > 5 && (endsWith(s, len, L"\x03bc\x03b1\x03c4\x03c9\x03bd") ||
                        endsWith(s, len, L"\x03bc\x03b1\x03c4\x03bf\x03c3")))
            len -= 3;
        else if (len > 4 && endsWith(s, len, L"\x03bc\x03b1\x03c4\x03b1"))
            len -= 2;
        return len;
    }

    int32_t GreekStemmer::rule21(wchar_t* s, int32_t len)
    {
        if (len > 9 && endsWith(s, len, L"\x03b9\x03bf\x03bd\x03c4\x03bf\x03c5\x03c3\x03b1\x03bd"))
            return len - 9;

        if (len > 8 && (endsWith(s, len, L"\x03b9\x03bf\x03bc\x03b1\x03c3\x03c4\x03b1\x03bd") ||
                        endsWith(s, len, L"\x03b9\x03bf\x03c3\x03b1\x03c3\x03c4\x03b1\x03bd") ||
                        endsWith(s, len, L"\x03b9\x03bf\x03c5\x03bc\x03b1\x03c3\x03c4\x03b5") ||
                        endsWith(s, len, L"\x03bf\x03bd\x03c4\x03bf\x03c5\x03c3\x03b1\x03bd")))
            return len - 8;

        if (len > 7 && (endsWith(s, len, L"\x03b9\x03b5\x03bc\x03b1\x03c3\x03c4\x03b5") ||
                        endsWith(s, len, L"\x03b9\x03b5\x03c3\x03b1\x03c3\x03c4\x03b5") ||
                        endsWith(s, len, L"\x03b9\x03bf\x03bc\x03bf\x03c5\x03bd\x03b1") ||
                        endsWith(s, len, L"\x03b9\x03bf\x03c3\x03b1\x03c3\x03c4\x03b5") ||
                        endsWith(s, len, L"\x03b9\x03bf\x03c3\x03bf\x03c5\x03bd\x03b1") ||
                        endsWith(s, len, L"\x03b9\x03bf\x03c5\x03bd\x03c4\x03b1\x03b9") ||
                        endsWith(s, len, L"\x03b9\x03bf\x03c5\x03bd\x03c4\x03b1\x03bd") ||
                        endsWith(s, len, L"\x03b7\x03b8\x03b7\x03ba\x03b1\x03c4\x03b5") ||
                        endsWith(s, len, L"\x03bf\x03bc\x03b1\x03c3\x03c4\x03b1\x03bd") ||
                        endsWith(s, len, L"\x03bf\x03c3\x03b1\x03c3\x03c4\x03b1\x03bd") ||
                        endsWith(s, len, L"\x03bf\x03c5\x03bc\x03b1\x03c3\x03c4\x03b5")))
            return len - 7;

        if (len > 6 && (endsWith(s, len, L"\x03b9\x03bf\x03bc\x03bf\x03c5\x03bd") ||
                        endsWith(s, len, L"\x03b9\x03bf\x03bd\x03c4\x03b1\x03bd") ||
                        endsWith(s, len, L"\x03b9\x03bf\x03c3\x03bf\x03c5\x03bd") ||
                        endsWith(s, len, L"\x03b7\x03b8\x03b5\x03b9\x03c4\x03b5") ||
                        endsWith(s, len, L"\x03b7\x03b8\x03b7\x03ba\x03b1\x03bd") ||
                        endsWith(s, len, L"\x03bf\x03bc\x03bf\x03c5\x03bd\x03b1") ||
                        endsWith(s, len, L"\x03bf\x03c3\x03b1\x03c3\x03c4\x03b5") ||
                        endsWith(s, len, L"\x03bf\x03c3\x03bf\x03c5\x03bd\x03b1") ||
                        endsWith(s, len, L"\x03bf\x03c5\x03bd\x03c4\x03b1\x03b9") ||
                        endsWith(s, len, L"\x03bf\x03c5\x03bd\x03c4\x03b1\x03bd") ||
                        endsWith(s, len, L"\x03bf\x03c5\x03c3\x03b1\x03c4\x03b5")))
            return len - 6;

        if (len > 5 && (endsWith(s, len, L"\x03b1\x03b3\x03b1\x03c4\x03b5") ||
                        endsWith(s, len, L"\x03b9\x03b5\x03bc\x03b1\x03b9") ||
                        endsWith(s, len, L"\x03b9\x03b5\x03c4\x03b1\x03b9") ||
                        endsWith(s, len, L"\x03b9\x03b5\x03c3\x03b1\x03b9") ||
                        endsWith(s, len, L"\x03b9\x03bf\x03c4\x03b1\x03bd") ||
                        endsWith(s, len, L"\x03b9\x03bf\x03c5\x03bc\x03b1") ||
                        endsWith(s, len, L"\x03b7\x03b8\x03b5\x03b9\x03c3") ||
                        endsWith(s, len, L"\x03b7\x03b8\x03bf\x03c5\x03bd") ||
                        endsWith(s, len, L"\x03b7\x03ba\x03b1\x03c4\x03b5") ||
                        endsWith(s, len, L"\x03b7\x03c3\x03b1\x03c4\x03b5") ||
                        endsWith(s, len, L"\x03b7\x03c3\x03bf\x03c5\x03bd") ||
                        endsWith(s, len, L"\x03bf\x03bc\x03bf\x03c5\x03bd") ||
                        endsWith(s, len, L"\x03bf\x03bd\x03c4\x03b1\x03b9") ||
                        endsWith(s, len, L"\x03bf\x03bd\x03c4\x03b1\x03bd") ||
                        endsWith(s, len, L"\x03bf\x03c3\x03bf\x03c5\x03bd") ||
                        endsWith(s, len, L"\x03bf\x03c5\x03bc\x03b1\x03b9") ||
                        endsWith(s, len, L"\x03bf\x03c5\x03c3\x03b1\x03bd")))
            return len - 5;

        if (len > 4 && (endsWith(s, len, L"\x03b1\x03b3\x03b1\x03bd") ||
                        endsWith(s, len, L"\x03b1\x03bc\x03b1\x03b9") ||
                        endsWith(s, len, L"\x03b1\x03c3\x03b1\x03b9") ||
                        endsWith(s, len, L"\x03b1\x03c4\x03b1\x03b9") ||
                        endsWith(s, len, L"\x03b5\x03b9\x03c4\x03b5") ||
                        endsWith(s, len, L"\x03b5\x03c3\x03b1\x03b9") ||
                        endsWith(s, len, L"\x03b5\x03c4\x03b1\x03b9") ||
                        endsWith(s, len, L"\x03b7\x03b4\x03b5\x03c3") ||
                        endsWith(s, len, L"\x03b7\x03b4\x03c9\x03bd") ||
                        endsWith(s, len, L"\x03b7\x03b8\x03b5\x03b9") ||
                        endsWith(s, len, L"\x03b7\x03ba\x03b1\x03bd") ||
                        endsWith(s, len, L"\x03b7\x03c3\x03b1\x03bd") ||
                        endsWith(s, len, L"\x03b7\x03c3\x03b5\x03b9") ||
                        endsWith(s, len, L"\x03b7\x03c3\x03b5\x03c3") ||
                        endsWith(s, len, L"\x03bf\x03bc\x03b1\x03b9") ||
                        endsWith(s, len, L"\x03bf\x03c4\x03b1\x03bd")))
            return len - 4;

        if (len > 3 && (endsWith(s, len, L"\x03b1\x03b5\x03b9") ||
                        endsWith(s, len, L"\x03b5\x03b9\x03c3") ||
                        endsWith(s, len, L"\x03b7\x03b8\x03c9") ||
                        endsWith(s, len, L"\x03b7\x03c3\x03c9") ||
                        endsWith(s, len, L"\x03bf\x03c5\x03bd") ||
                        endsWith(s, len, L"\x03bf\x03c5\x03c3")))
            return len - 3;

        if (len > 2 && (endsWith(s, len, L"\x03b1\x03bd") ||
                        endsWith(s, len, L"\x03b1\x03c3") ||
                        endsWith(s, len, L"\x03b1\x03c9") ||
                        endsWith(s, len, L"\x03b5\x03b9") ||
                        endsWith(s, len, L"\x03b5\x03c3") ||
                        endsWith(s, len, L"\x03b7\x03c3") ||
                        endsWith(s, len, L"\x03bf\x03b9") ||
                        endsWith(s, len, L"\x03bf\x03c3") ||
                        endsWith(s, len, L"\x03bf\x03c5") ||
                        endsWith(s, len, L"\x03c5\x03c3") ||
                        endsWith(s, len, L"\x03c9\x03bd")))
            return len - 2;

        if (len > 1 && endsWithVowel(s, len))
            return len - 1;

        return len;
    }

    int32_t GreekStemmer::rule22(wchar_t* s, int32_t len)
    {
        if (endsWith(s, len, L"\x03b5\x03c3\x03c4\x03b5\x03c1") ||
            endsWith(s, len, L"\x03b5\x03c3\x03c4\x03b1\x03c4"))
            return len - 5;

        if (endsWith(s, len, L"\x03bf\x03c4\x03b5\x03c1") ||
            endsWith(s, len, L"\x03bf\x03c4\x03b1\x03c4") ||
            endsWith(s, len, L"\x03c5\x03c4\x03b5\x03c1") ||
            endsWith(s, len, L"\x03c5\x03c4\x03b1\x03c4") ||
            endsWith(s, len, L"\x03c9\x03c4\x03b5\x03c1") ||
            endsWith(s, len, L"\x03c9\x03c4\x03b1\x03c4"))
            return len - 4;

        return len;
    }

    int32_t GreekStemmer::endsWith(wchar_t* s, int32_t len, const String& suffix)
    {
        int32_t suffixLen = suffix.length();
        if (suffixLen > len)
            return false;
        for (int32_t i = suffixLen - 1; i >= 0; --i)
        {
            if (s[len - (suffixLen - i)] != suffix[i])
                return false;
        }
        return true;
    }

    int32_t GreekStemmer::endsWithVowel(wchar_t* s, int32_t len)
    {
        if (len == 0)
            return false;
        switch (s[len - 1])
        {
            case L'\x03b1':
            case L'\x03b5':
            case L'\x03b7':
            case L'\x03b9':
            case L'\x03bf':
            case L'\x03c5':
            case L'\x03c9':
                return true;
            default:
                return false;
        }
    }

    int32_t GreekStemmer::endsWithVowelNoY(wchar_t* s, int32_t len)
    {
        if (len == 0)
            return false;
        switch (s[len - 1])
        {
            case L'\x03b1':
            case L'\x03b5':
            case L'\x03b7':
            case L'\x03b9':
            case L'\x03bf':
            case L'\x03c9':
                return true;
            default:
                return false;
        }
    }
}

