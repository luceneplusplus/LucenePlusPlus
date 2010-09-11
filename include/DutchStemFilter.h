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
	/// A {@link TokenFilter} that stems Dutch words. 
	///
	/// It supports a table of words that should not be stemmed at all.  The stemmer used can 
	/// be changed at runtime after the filter object is created (as long as it is a 
	/// {@link DutchStemmer}).
	class LPPAPI DutchStemFilter : public TokenFilter
	{
	public:
		DutchStemFilter(TokenStreamPtr input);
		
		/// Builds a DutchStemFilter that uses an exclusion table.
		DutchStemFilter(TokenStreamPtr input, HashSet<String> exclusiontable);
		
		/// Builds a DutchStemFilter that uses an exclusion table and dictionary of word stem 
		/// pairs, that overrule the algorithm.
		DutchStemFilter(TokenStreamPtr input, HashSet<String> exclusiontable, MapStringString stemdictionary);
		
		virtual ~DutchStemFilter();
		
		LUCENE_CLASS(DutchStemFilter);
    
    protected:
        /// {@link DutchStemmer} in use by this filter.
        DutchStemmerPtr stemmer;
        
        HashSet<String> exclusions;
        TermAttributePtr termAtt;
    
    public:
        virtual bool incrementToken();
        
        /// Set a alternative/custom {@link DutchStemmer} for this filter.
        void setStemmer(DutchStemmerPtr stemmer);
        
        /// Set an alternative exclusion list for this filter.
        void setExclusionSet(HashSet<String> exclusiontable);
        
        /// Set dictionary for stemming, this dictionary overrules the algorithm, so you can 
        /// correct for a particular unwanted word-stem pair.
        void setStemDictionary(MapStringString dict);
	};
}
