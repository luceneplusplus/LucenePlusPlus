/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Lucene.h"

namespace Lucene
{
	// analyzers
	DECLARE_SHARED_PTR(ArabicAnalyzer)
	DECLARE_SHARED_PTR(ArabicAnalyzerSavedStreams)
	DECLARE_SHARED_PTR(ArabicLetterTokenizer)
	DECLARE_SHARED_PTR(ArabicNormalizationFilter)
	DECLARE_SHARED_PTR(ArabicNormalizer)
	DECLARE_SHARED_PTR(ArabicStemFilter)
	DECLARE_SHARED_PTR(ArabicStemmer)
	DECLARE_SHARED_PTR(ReverseStringFilter)
}
