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
	class LPPAPI MapWeightedSpanTerm : public LuceneObject
	{
	public:
	    virtual ~MapWeightedSpanTerm();
		LUCENE_CLASS(MapWeightedSpanTerm);
	
	protected:
	    MapStringWeightedSpanTerm map;
	
	public:
	    virtual MapStringWeightedSpanTerm::iterator begin();
	    virtual MapStringWeightedSpanTerm::iterator end();
	    virtual void put(const String& key, WeightedSpanTermPtr val);
	    virtual WeightedSpanTermPtr get(const String& key) const;
	    virtual void clear();
	};
}
