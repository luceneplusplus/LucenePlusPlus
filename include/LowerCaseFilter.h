/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "TokenFilter.h"

namespace Lucene
{
	/// Normalizes token text to lower case.
	class LPPAPI LowerCaseFilter : public TokenFilter
	{
	public:
		LowerCaseFilter(TokenStreamPtr input);
		virtual ~LowerCaseFilter();
		
		LUCENE_CLASS(LowerCaseFilter);
	
	protected:
		TermAttributePtr termAtt;
	
	public:
		virtual bool incrementToken();
	};
}
