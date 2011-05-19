/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "PersianNormalizationFilter.h"
#include "PersianNormalizer.h"
#include "CharTermAttribute.h"

namespace Lucene
{
    PersianNormalizationFilter::PersianNormalizationFilter(TokenStreamPtr input) : TokenFilter(input)
    {
        normalizer = newLucene<PersianNormalizer>();
        termAtt = addAttribute<CharTermAttribute>();
    }

    PersianNormalizationFilter::~PersianNormalizationFilter()
    {
    }

    bool PersianNormalizationFilter::incrementToken()
    {
        if (input->incrementToken())
        {
            int32_t newlen = normalizer->normalize(termAtt->buffer().get(), termAtt->length());
            termAtt->setLength(newlen);
            return true;
        }
        else
            return false;
    }
}

