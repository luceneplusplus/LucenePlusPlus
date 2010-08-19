/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "test_lucene.h"
#include "Filter.h"

namespace Lucene
{
	class MockFilter : public Filter
	{
	public:
		MockFilter();
		virtual ~MockFilter();
		
		LUCENE_CLASS(MockFilter);
	
	protected:
		bool _wasCalled;
	
	public:
		virtual DocIdSetPtr getDocIdSet(IndexReaderPtr reader);
		
		void clear();
		bool wasCalled();
	};
}
