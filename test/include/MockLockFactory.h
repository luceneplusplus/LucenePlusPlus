/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "test_lucene.h"
#include "LockFactory.h"

namespace Lucene
{
	class MockLockFactory : public LockFactory
	{
	public:
		MockLockFactory();
		virtual ~MockLockFactory();
		
		LUCENE_CLASS(MockLockFactory);
		
	public:
		bool lockPrefixSet;
		int32_t makeLockCount;
		MapStringLockPtr locksCreated;
	
	public:
		virtual void setLockPrefix(const String& lockPrefix);
		virtual LockPtr makeLock(const String& lockName);
		virtual void clearLock(const String& lockName);
	};
}
