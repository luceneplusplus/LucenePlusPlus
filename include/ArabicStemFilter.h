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
	/// A {@link TokenFilter} that applies {@link ArabicStemmer} to stem Arabic words.
	class LPPAPI ArabicStemFilter : public TokenFilter
	{
	public:
		ArabicStemFilter(TokenStreamPtr input);
		virtual ~ArabicStemFilter();
		
		LUCENE_CLASS(ArabicStemFilter);
    
    protected:
        ArabicStemmerPtr stemmer;
        TermAttributePtr termAtt;
    
    public:
        virtual bool incrementToken();
	};
}
