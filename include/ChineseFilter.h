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
	/// A {@link TokenFilter} with a stop word table.  
	/// <ul>
	/// <li> Numeric tokens are removed.
	/// <li> English tokens must be larger than 1 character.
	/// <li> One Chinese character as one Chinese word.
	/// </ul>
	class LPPAPI ChineseFilter : public TokenFilter
	{
	public:
		ChineseFilter(TokenStreamPtr input);
		virtual ~ChineseFilter();
		
		LUCENE_CLASS(ChineseFilter);
    
    public:
        /// Only English now, Chinese to be added later.
        static const wchar_t* STOP_WORDS[];
        
    protected:
        HashSet<String> stopTable;
        TermAttributePtr termAtt;
            
    public:
        virtual bool incrementToken();
	};
}
