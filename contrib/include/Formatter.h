/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene
{
	/// Processes terms found in the original text, typically by applying some form of mark-up to highlight 
	/// terms in HTML search results pages.
	class LPPAPI Formatter
	{
	public:
		virtual ~Formatter();
		LUCENE_INTERFACE(Formatter);
	
	public:
	    /// @param originalText The section of text being considered for markup
	    /// @param tokenGroup contains one or several overlapping Tokens along with their scores and positions.
		virtual String highlightTerm(const String& originalText, TokenGroupPtr tokenGroup);
	};
}
