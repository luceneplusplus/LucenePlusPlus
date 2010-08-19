/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Lucene.h"

#ifdef _DEBUG
#define LUCENE_INTERFACE(Name) \
	static String _getClassName() { return L###Name; } \
	virtual String getClassName() { return L###Name; } \
	CycleCheckT<Name> cycleCheck;
#else
#define LUCENE_INTERFACE(Name) \
	static String _getClassName() { return L###Name; } \
	virtual String getClassName() { return L###Name; }
#endif

#define LUCENE_CLASS(Name) \
	LUCENE_INTERFACE(Name); \
	boost::shared_ptr<Name> shared_from_this() { return boost::static_pointer_cast<Name>(LuceneObject::shared_from_this()); } \

namespace Lucene
{
	/// Base class for all Lucene classes
	class LPPAPI LuceneObject : public boost::enable_shared_from_this<LuceneObject>
	{
	public:
		virtual ~LuceneObject();
	
	protected:
		static boost::mutex lockMutex;
		
		SynchronizePtr objectLock;
		LuceneSignalPtr objectSignal;
	
	public:
		/// Called directly after instantiation to create objects that depend on this object being 
		/// fully constructed.
		virtual void initialize();
		
		/// Return clone of this object
		/// @param other clone reference - null when called initially, then set in top virtual override.
		virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
		
		/// Return hash code for this object.
		virtual int32_t hashCode();
		
		/// Return whether two objects are equal
		virtual bool equals(LuceneObjectPtr other);
		
		/// Compare two objects
		virtual int32_t compareTo(LuceneObjectPtr other);
		
		/// Returns a string representation of the object
		virtual String toString();
		
		/// Return this object synchronize lock.
		virtual SynchronizePtr getSync();
		
		/// Return this object signal.
		virtual LuceneSignalPtr getSignal();
		
		/// Lock this object using an optional timeout.
		virtual void lock(int32_t timeout = 0);
		
		/// Unlock this object.
		virtual void unlock();
		
		/// Returns true if this object is currently locked by current thread.
		virtual bool holdsLock();
		
		/// Wait for signal using an optional timeout.
		virtual void wait(int32_t timeout = 0);
		
		/// Notify all threads waiting for signal.
		virtual void notifyAll();
	};
}
