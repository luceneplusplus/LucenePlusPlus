/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "ElisionFilter.h"
#include "CharArraySet.h"
#include "CharTermAttribute.h"

namespace Lucene
{
    const wchar_t ElisionFilter::apostrophes[] = {L'\'', L'\x2019'};

    ElisionFilter::ElisionFilter(LuceneVersion::Version matchVersion, TokenStreamPtr input) : TokenFilter(input)
    {
        articles = newLucene<CharArraySet>(newCollection<String>(L"l", L"m", L"t", L"qu", L"n", L"s", L"j"), true);
        termAtt = addAttribute<CharTermAttribute>();
    }

    ElisionFilter::ElisionFilter(TokenStreamPtr input) : TokenFilter(input)
    {
        articles = newLucene<CharArraySet>(newCollection<String>(L"l", L"m", L"t", L"qu", L"n", L"s", L"j"), true);
        termAtt = addAttribute<CharTermAttribute>();
    }

    ElisionFilter::ElisionFilter(TokenStreamPtr input, HashSet<String> articles) : TokenFilter(input)
    {
        setArticles(LuceneVersion::LUCENE_CURRENT, articles);
        termAtt = addAttribute<CharTermAttribute>();
    }

    ElisionFilter::ElisionFilter(LuceneVersion::Version matchVersion, TokenStreamPtr input, HashSet<String> articles) : TokenFilter(input)
    {
        setArticles(matchVersion, articles);
        termAtt = addAttribute<CharTermAttribute>();
    }

    ElisionFilter::~ElisionFilter()
    {
    }

    void ElisionFilter::setArticles(LuceneVersion::Version matchVersion, HashSet<String> articles)
    {
        this->articles = newLucene<CharArraySet>(matchVersion, articles, true);
    }

    void ElisionFilter::setArticles(HashSet<String> articles)
    {
        setArticles(LuceneVersion::LUCENE_CURRENT, articles);
    }

    bool ElisionFilter::incrementToken()
    {
        if (input->incrementToken())
        {
            wchar_t* termBuffer = termAtt->bufferArray();
            int32_t termLength = termAtt->length();

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

