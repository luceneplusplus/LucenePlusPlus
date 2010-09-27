/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TextFragment.h"

namespace Lucene
{
    TextFragment::TextFragment(const String& markedUpText, int32_t textStartPos, int32_t fragNum)
    {
        this->markedUpText = markedUpText;
        this->textStartPos = textStartPos;
        this->textEndPos = 0;
        this->fragNum = fragNum;
        this->score = 0;
    }
    
    TextFragment::~TextFragment()
    {
    }
    
    void TextFragment::setScore(double score)
    {
        this->score = score;
    }
    
    double TextFragment::getScore()
    {
        return score;
    }
    
    void TextFragment::merge(TextFragmentPtr frag2)
    {
        textEndPos = frag2->textEndPos;
        score = std::max(score, frag2->score);
    }
    
    bool TextFragment::follows(TextFragmentPtr fragment)
    {
        return (textStartPos == fragment->textEndPos);
    }
    
    int32_t TextFragment::getFragNum()
    {
        return fragNum;
    }
    
    String TextFragment::toString()
    {
        return markedUpText.substr(textStartPos, textEndPos - textStartPos);
    }
}
