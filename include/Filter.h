/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneObject.h"

namespace Lucene
{
	/// Abstract base class providing a mechanism to use a subset of an index for restriction or permission of 
	/// index search results.
	class LPPAPI Filter : public LuceneObject
	{
	public:
		virtual ~Filter();	
		LUCENE_CLASS(Filter);
	
	public:
		/// @return a DocIdSet that provides the documents which should be permitted or prohibited in search results.
		/// NOTE: null can be returned if no documents will be accepted by this Filter.
		/// @see DocIdBitSet
		virtual DocIdSetPtr getDocIdSet(IndexReaderPtr reader) = 0;
	};
}
