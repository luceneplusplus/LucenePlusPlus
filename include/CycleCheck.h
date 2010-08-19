/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Lucene.h"
#include "Synchronize.h"

namespace Lucene
{
	/// Debug utility to track shared_ptr utilization.
	class LPPAPI CycleCheck
	{
	public:
		virtual ~CycleCheck();
	
	protected:
		static MapStringInt cycleMap;
		static Set<LuceneObjectPtr*> staticRefs;
	
	protected:
		void addRef(const String& className, int32_t ref);
	
	public:
		template <class TYPE>
		static void addStatic(TYPE& staticRef)
		{
			#ifdef _DEBUG
			if (!staticRefs)
				staticRefs = Set<LuceneObjectPtr*>::newInstance();
			staticRefs.add(reinterpret_cast<LuceneObjectPtr*>(&staticRef));
			#endif
		}
		
		static void dumpRefs();
	};
	
	template <class TYPE>
	class CycleCheckT : public CycleCheck
	{
	public:
		CycleCheckT()
		{
			addRef(TYPE::_getClassName(), 1);
		}
		
		virtual ~CycleCheckT()
		{
			addRef(TYPE::_getClassName(), -1);
		}
	};
}
