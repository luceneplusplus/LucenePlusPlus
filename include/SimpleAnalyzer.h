/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Analyzer.h"

namespace Lucene
{
	/// An {@link Analyzer} that filters {@link LetterTokenizer} with {@link LowerCaseFilter}
	class LPPAPI SimpleAnalyzer : public Analyzer
	{
	public:
		virtual ~SimpleAnalyzer();
		
		LUCENE_CLASS(SimpleAnalyzer);
	
	public:
		virtual TokenStreamPtr tokenStream(const String& fieldName, ReaderPtr reader);
		virtual TokenStreamPtr reusableTokenStream(const String& fieldName, ReaderPtr reader);
	};
}
