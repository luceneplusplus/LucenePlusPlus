/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "ElisionFilter.h"
#include "Collection<CharArray>Set.h"
#include "TermAttribute.h"

namespace Lucene
{
    const wchar_t ElisionFilter::apostrophes[] = {L'\'', L'\x2019'};

    ElisionFilter::ElisionFilter(TokenStreamPtr input) : TokenFilter(input)
    {
        articles = newLucene<Collection<CharArray>Set>(newCollection<String>(L"l", L"m", L"t", L"qu", L"n", L"s", L"j"), true);
        termAtt = addAttribute<TermAttribute>();
    }

    ElisionFilter::ElisionFilter(TokenStreamPtr input, SetString articles) : TokenFilter(input)
    {
        setArticles(articles);
        termAtt = addAttribute<TermAttribute>();
    }

    ElisionFilter::~ElisionFilter()
    {
    }

    void ElisionFilter::setArticles(SetString articles)
    {
        this->articles = newLucene<Collection<CharArray>Set>(articles, true);
    }

    bool ElisionFilter::incrementToken()
    {
        if (input->incrementToken())
        {
            wchar_t* termBuffer = termAtt->termBufferArray();
            int32_t termLength = termAtt->termLength();

            int32_t minPoz = INT_MAX;
            for (int32_t i = 0; i < SIZEOF_ARRAY(apostrophes); ++i)
            {
                wchar_t apos = apostrophes[i];
                for (int32_t poz = 0; poz < termLength; ++poz)
                {
                    if (termBuffer[poz] == apos)
                    {
                        minPoz = std::min(poz, minPoz);
                        break;
                    }
                }
            }

            // An apostrophe has been found. If the prefix is an article strip it off.
            if (minPoz != INT_MAX && articles->contains(termBuffer, 0, minPoz))
                termAtt->setTermBuffer(termBuffer, minPoz + 1, termLength - (minPoz + 1));

            return true;
        }
        else
            return false;
    }
}
