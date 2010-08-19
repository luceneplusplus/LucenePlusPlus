/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "PhraseScorer.h"

namespace Lucene
{
	class LPPAPI ExactPhraseScorer : public PhraseScorer
	{
	public:
		ExactPhraseScorer(WeightPtr weight, Collection<TermPositionsPtr> tps, Collection<int32_t> offsets, SimilarityPtr similarity, ByteArray norms);
		virtual ~ExactPhraseScorer();
	
		LUCENE_CLASS(ExactPhraseScorer);
	
	protected:
		virtual double phraseFreq();
	};
}
