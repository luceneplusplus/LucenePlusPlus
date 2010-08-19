/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "test_lucene.h"
#include "RAMInputStream.h"

namespace Lucene
{
	/// Used by MockRAMDirectory to create an input stream that keeps track of when it's been closed.
	class MockRAMInputStream : public RAMInputStream
	{
	public:
		/// Construct an empty output buffer. 
		MockRAMInputStream();
		MockRAMInputStream(MockRAMDirectoryPtr dir, const String& name, RAMFilePtr f);
		virtual ~MockRAMInputStream();
		
		LUCENE_CLASS(MockRAMInputStream);
	
	protected:
		MockRAMDirectoryWeakPtr _dir;
		String name;
		bool isClone;
	
	public:
		virtual void close();
		
		virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
	};
}
