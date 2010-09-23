/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneContrib.h"
#include "TokenFilter.h"

struct sb_stemmer;

namespace Lucene
{
	/// A filter that stems words using a Snowball-generated stemmer.
	class LPPAPI SnowballFilter : public TokenFilter
	{
	public:
		SnowballFilter(TokenStreamPtr input, const String& name);
		virtual ~SnowballFilter();
		
		LUCENE_CLASS(SnowballFilter);
    
    protected:
        struct sb_stemmer* stemmer;
        UTF8ResultPtr utf8Result;
        TermAttributePtr termAtt;
    
    public:
        virtual bool incrementToken();
	};
}
