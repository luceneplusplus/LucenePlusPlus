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
	/// A {@link TokenFilter} that stems French words. 
	///
	/// It supports a table of words that should not be stemmed at all.  The stemmer used can 
	/// be changed at runtime after the filter object is created (as long as it is a 
	/// {@link FrenchStemmer}).
	class LPPAPI FrenchStemFilter : public TokenFilter
	{
	public:
		FrenchStemFilter(TokenStreamPtr input);
		
		/// Builds a FrenchStemFilter that uses an exclusion table.
		FrenchStemFilter(TokenStreamPtr input, HashSet<String> exclusiontable);
		
		virtual ~FrenchStemFilter();
		
		LUCENE_CLASS(FrenchStemFilter);
    
    protected:
        /// {@link FrenchStemmer} in use by this filter.
        FrenchStemmerPtr stemmer;
        
        HashSet<String> exclusions;
        TermAttributePtr termAtt;
    
    public:
        virtual bool incrementToken();
        
        /// Set a alternative/custom {@link FrenchStemmer} for this filter.
        void setStemmer(FrenchStemmerPtr stemmer);
        
        /// Set an alternative exclusion list for this filter.
        void setExclusionSet(HashSet<String> exclusiontable);
	};
}
