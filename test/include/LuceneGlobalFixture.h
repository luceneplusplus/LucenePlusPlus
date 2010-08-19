/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "test_lucene.h"

namespace Lucene
{
	class LuceneGlobalFixture
	{
	public:
		/// setup
		LuceneGlobalFixture();
		
		/// teardown
		virtual ~LuceneGlobalFixture();
	};
}

