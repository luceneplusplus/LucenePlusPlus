/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "TermPositions.h"
#include "PriorityQueue.h"

namespace Lucene
{
	/// Allows you to iterate over the {@link TermPositions} for multiple {@link Term}s as a single 
	/// {@link TermPositions}.
	class LPPAPI MultipleTermPositions : public TermPositions, public LuceneObject
	{
	public:
		MultipleTermPositions(IndexReaderPtr indexReader, Collection<TermPtr> terms);
		virtual ~MultipleTermPositions();
		
		LUCENE_CLASS(MultipleTermPositions);
			
	protected:
		int32_t _doc;
		int32_t _freq;
		TermPositionsQueuePtr termPositionsQueue;
		IntQueuePtr posList;
	
	public:
		virtual bool next();
		virtual int32_t nextPosition();
		virtual bool skipTo(int32_t target);
		virtual int32_t doc();
		virtual int32_t freq();
		virtual void close();
		
		/// Not implemented.
		virtual void seek(TermPtr term);
		
		/// Not implemented.
		virtual void seek(TermEnumPtr termEnum);
		
		/// Not implemented.
		virtual int32_t read(Collection<int32_t> docs, Collection<int32_t> freqs);
		
		/// Not implemented.
		virtual ByteArray getPayload(ByteArray data, int32_t offset);
		
		/// @return false
		virtual bool isPayloadAvailable();
	};
	
	class LPPAPI TermPositionsQueue : public PriorityQueue<TermPositionsPtr>
	{
	public:
		TermPositionsQueue(Collection<TermPositionsPtr> termPositions);
		virtual ~TermPositionsQueue();
		
		LUCENE_CLASS(TermPositionsQueue);
	
	protected:
		Collection<TermPositionsPtr> termPositions;
	
	public:
		virtual void initialize();
	
	protected:
		virtual bool lessThan(const TermPositionsPtr& first, const TermPositionsPtr& second);
	};
	
	class LPPAPI IntQueue : public LuceneObject
	{
	public:
		IntQueue();
		virtual ~IntQueue();
		
		LUCENE_CLASS(IntQueue);
			
	protected:
		int32_t arraySize;
		int32_t index;
		int32_t lastIndex;
		Collection<int32_t> array;
	
	public:
		void add(int32_t i);
		int32_t next();
		void sort();
		void clear();
		int32_t size();
	
	protected:
		void growArray();
	};
}
