/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MultiTermQueryWrapperFilter.h"

namespace Lucene
{
	/// A Filter that restricts search results to values that have a matching prefix in a given field.
	class LPPAPI PrefixFilter : public MultiTermQueryWrapperFilter
	{
	public:
		PrefixFilter(TermPtr prefix);
		virtual ~PrefixFilter();
	
		LUCENE_CLASS(PrefixFilter);
	
	public:
		TermPtr getPrefix();
		
		virtual String toString();
	};
}
