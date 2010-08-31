/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneContrib.h"
#include "LetterTokenizer.h"

namespace Lucene
{
    /// Tokenizer that breaks text into runs of letters and diacritics.
    ///
    /// The problem with the standard Letter tokenizer is that it fails on diacritics.
    /// Handling similar to this is necessary for Indic Scripts, Hebrew, Thaana, etc.
    ///
 	class LPPAPI ArabicLetterTokenizer : public LetterTokenizer
	{
	public:
		/// Construct a new ArabicLetterTokenizer.
		ArabicLetterTokenizer(ReaderPtr input);
		
		/// Construct a new ArabicLetterTokenizer using a given {@link AttributeSource}.
		ArabicLetterTokenizer(AttributeSourcePtr source, ReaderPtr input);
		
		/// Construct a new ArabicLetterTokenizer using a given {@link AttributeFactory}.
		ArabicLetterTokenizer(AttributeFactoryPtr factory, ReaderPtr input);
		
		virtual ~ArabicLetterTokenizer();
		
		LUCENE_CLASS(ArabicLetterTokenizer);
    
    public:
		/// Allows for Letter category or NonspacingMark category
		virtual bool isTokenChar(wchar_t c);
	};
}
