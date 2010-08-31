/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneContrib.h"
#include "TokenFilter.h"

namespace Lucene
{
	/// A {@link TokenFilter} that applies {@link ArabicNormalizer} to normalize the orthography.
	class LPPAPI ArabicNormalizationFilter : public TokenFilter
	{
	public:
		ArabicNormalizationFilter(TokenStreamPtr input);
		virtual ~ArabicNormalizationFilter();
		
		LUCENE_CLASS(ArabicNormalizationFilter);
    
    protected:
        ArabicNormalizerPtr normalizer;
        TermAttributePtr termAtt;
    
    public:
        virtual bool incrementToken();
	};
}
