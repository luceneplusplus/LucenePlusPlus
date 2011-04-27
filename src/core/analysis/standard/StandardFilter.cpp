/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "StandardFilter.h"
#include "StandardTokenizer.h"
#include "CharTermAttribute.h"
#include "TypeAttribute.h"

namespace Lucene
{
    StandardFilter::StandardFilter(TokenStreamPtr input) : TokenFilter(input)
    {
        this->matchVersion = LuceneVersion::LUCENE_30;
        termAtt = addAttribute<CharTermAttribute>();
        typeAtt = addAttribute<TypeAttribute>();
    }
    
    StandardFilter::StandardFilter(LuceneVersion::Version matchVersion, TokenStreamPtr input) : TokenFilter(input)
    {
        this->matchVersion = matchVersion;
        termAtt = addAttribute<CharTermAttribute>();
        typeAtt = addAttribute<TypeAttribute>();
    }
    
    StandardFilter::~StandardFilter()
    {
    }
    
    const String& StandardFilter::APOSTROPHE_TYPE()
    {
        static String _APOSTROPHE_TYPE;
        if (_APOSTROPHE_TYPE.empty())
            _APOSTROPHE_TYPE = StandardTokenizer::TOKEN_TYPES()[StandardTokenizer::APOSTROPHE];
        return _APOSTROPHE_TYPE;
    }
    
    const String& StandardFilter::ACRONYM_TYPE()
    {
        static String _ACRONYM_TYPE;
        if (_ACRONYM_TYPE.empty())
            _ACRONYM_TYPE = StandardTokenizer::TOKEN_TYPES()[StandardTokenizer::ACRONYM];
        return _ACRONYM_TYPE;
    }
    
    bool StandardFilter::incrementToken()
    {
        if (LuceneVersion::onOrAfter(matchVersion, LuceneVersion::LUCENE_31))
            return input->incrementToken();
        else
            return incrementTokenClassic();
    }
    
    bool StandardFilter::incrementTokenClassic()
    {    
        if (!input->incrementToken())
            return false;
        
        wchar_t* termBuffer = termAtt->bufferArray();
        int32_t bufferLength = termAtt->length();
        String type(typeAtt->type());
        
        if (type == APOSTROPHE_TYPE() && bufferLength >= 2 && termBuffer[bufferLength - 2] == L'\'' &&
            (termBuffer[bufferLength - 1] == L's' || termBuffer[bufferLength - 1] == L'S')) // remove 's
        {
            // Strip last 2 characters off
            termAtt->setLength(bufferLength - 2);
        }
        else if (type == ACRONYM_TYPE()) // remove dots
        {
            int32_t upto = 0;
            for (int32_t i = 0; i < bufferLength; ++i)
            {
                wchar_t c = termBuffer[i];
                if (c != L'.')
                    termBuffer[upto++] = c;
            }
            termAtt->setLength(upto);
        }
        
        return true;
    }
}
