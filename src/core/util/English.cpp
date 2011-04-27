/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include <boost/algorithm/string.hpp>
#include "English.h"

namespace Lucene
{
    English::English()
    {
    }
    
    English::~English()
    {
    }
    
    String English::longToEnglish(int64_t i)
    {
        String english;
        if (i == 0)
            return L"zero";
        if (i < 0) 
        {
            english += L"minus ";
            i = -i;
        }
        if (i >= 1000000000000000000LL) // quintillion
        {
            english += longToEnglish(i / 1000000000000000000LL);
            english += L"quintillion, ";
            i = i % 1000000000000000000LL;
        }
        if (i >= 1000000000000000LL) // quadrillion
        {
            english += longToEnglish(i / 1000000000000000LL);
            english += L"quadrillion, ";
            i = i % 1000000000000000LL;
        }
        if (i >= 1000000000000LL) // trillions
        {
            english += longToEnglish(i / 1000000000000LL);
            english += L"trillion, ";
            i = i % 1000000000000LL;
        }
        if (i >= 1000000000) // billions
        {
            english += longToEnglish(i / 1000000000);
            english += L"billion, ";
            i = i % 1000000000;
        }
        if (i >= 1000000) // millions
        {
            english += longToEnglish(i / 1000000);
            english += L"million, ";
            i = i % 1000000;
        }
        if (i >= 1000) // thousands
        {
            english += longToEnglish(i / 1000);
            english += L"thousand, ";
            i = i % 1000;
        }
        if (i >= 100) // hundreds
        {
            english += longToEnglish(i / 100);
            english += L"hundred ";
            i = i % 100;
        }
        if (i >= 20)
        {
            switch ((int32_t)i / 10)
            {
                case 9:
                    english += L"ninety";
                    break;
                case 8:
                    english += L"eighty";
                    break;
                case 7:
                    english += L"seventy";
                    break;
                case 6:
                    english += L"sixty";
                    break;
                case 5:
                    english += L"fifty";
                    break;
                case 4:
                    english += L"forty";
                    break;
                case 3:
                    english += L"thirty";
                    break;
                case 2:
                    english += L"twenty";
                    break;
            }
            i = i % 10;
            english += i == 0 ? L" " : L"-";
        }
        switch ((int32_t)i)
        {
            case 19:
                english += L"nineteen ";
                break;
            case 18:
                english += L"eighteen ";
                break;
            case 17:
                english += L"seventeen ";
                break;
            case 16:
                english += L"sixteen ";
                break;
            case 15:
                english += L"fifteen ";
                break;
            case 14:
                english += L"fourteen ";
                break;
            case 13:
                english += L"thirteen ";
                break;
            case 12:
                english += L"twelve ";
                break;
            case 11:
                english += L"eleven ";
                break;
            case 10:
                english += L"ten ";
                break;
            case 9:
                english += L"nine ";
                break;
            case 8:
                english += L"eight ";
                break;
            case 7:
                english += L"seven ";
                break;
            case 6:
                english += L"six ";
                break;
            case 5:
                english += L"five ";
                break;
            case 4:
                english += L"four ";
                break;
            case 3:
                english += L"three ";
                break;
            case 2:
                english += L"two ";
                break;
            case 1:
                english += L"one ";
                break;
        }
        return boost::trim_copy(english);
    }
    
    String English::intToEnglish(int32_t i)
    {
        return longToEnglish(i);
    }
}
