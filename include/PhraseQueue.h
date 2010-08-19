/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "PriorityQueue.h"

namespace Lucene
{
	class LPPAPI PhraseQueue : public PriorityQueue<PhrasePositionsPtr>
	{
	public:
		PhraseQueue(int32_t size);
		virtual ~PhraseQueue();
	
		LUCENE_CLASS(PhraseQueue);
	
	protected:
		virtual bool lessThan(const PhrasePositionsPtr& first, const PhrasePositionsPtr& second);
	};
}
