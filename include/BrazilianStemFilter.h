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
	/// A {@link TokenFilter} that applies {@link BrazilianStemmer}.
	class LPPAPI BrazilianStemFilter : public TokenFilter
	{
	public:
		BrazilianStemFilter(TokenStreamPtr input);
		BrazilianStemFilter(TokenStreamPtr input, HashSet<String> exclusiontable);
		
		virtual ~BrazilianStemFilter();
		
		LUCENE_CLASS(BrazilianStemFilter);
    
    protected:
        /// {@link BrazilianStemmer} in use by this filter.
        BrazilianStemmerPtr stemmer;
        
        HashSet<String> exclusions;
        TermAttributePtr termAtt;
    
    public:
        virtual bool incrementToken();
	};
}
