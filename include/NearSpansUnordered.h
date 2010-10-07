/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Spans.h"
#include "PriorityQueue.h"

namespace Lucene
{
	/// Similar to {@link NearSpansOrdered}, but for the unordered case.
	///
	/// Only public for subclassing.  Most implementations should not need this class
	class LPPAPI NearSpansUnordered : public Spans
	{
	public:
		NearSpansUnordered(SpanNearQueryPtr query, IndexReaderPtr reader);
		virtual ~NearSpansUnordered();
		
		LUCENE_CLASS(NearSpansUnordered);
	
	protected:
		SpanNearQueryPtr query;
		IndexReaderPtr reader;
		
		Collection<SpansCellPtr> ordered; // spans in query order
		Collection<SpansPtr> subSpans;
		int32_t slop; // from query
		
		SpansCellPtr first; // linked list of spans
		SpansCellPtr last; // sorted by doc only
		
		int32_t totalLength; // sum of current lengths
		
		CellQueuePtr queue; // sorted queue of spans
		SpansCellPtr max; // max element in queue
		
		bool more; // true if not done
		bool firstTime; // true before first next()
	
	public:
		virtual void initialize();
		
		Collection<SpansPtr> getSubSpans();
		
		virtual bool next();
		virtual bool skipTo(int32_t target);
		virtual int32_t doc();
		virtual int32_t start();
		virtual int32_t end();
		virtual Collection<ByteArray> getPayload();
		virtual bool isPayloadAvailable();
		virtual String toString();
	
	protected:
		SpansCellPtr min();
		void initList(bool next);
		void addToList(SpansCellPtr cell);
		void firstToLast();
		void queueToList();
		void listToQueue();
		bool atMatch();
		
		friend class SpansCell;
	};
	
	/// Wraps a Spans, and can be used to form a linked list.
	class LPPAPI SpansCell : public Spans
	{
	public:
		SpansCell(NearSpansUnorderedPtr unordered, SpansPtr spans, int32_t index);
		virtual ~SpansCell();
		
		LUCENE_CLASS(SpansCell);
	
	protected:
		NearSpansUnorderedWeakPtr _unordered;
		SpansPtr spans;
		SpansCellPtr _next;
		int32_t length;
		int32_t index;
	
	public:
		virtual bool next();
		virtual bool skipTo(int32_t target);
		virtual int32_t doc();
		virtual int32_t start();
		virtual int32_t end();
		virtual Collection<ByteArray> getPayload();
		virtual bool isPayloadAvailable();
		virtual String toString();
	
	protected:
		bool adjust(bool condition);
		
		friend class NearSpansUnordered;
	};
	
	class LPPAPI CellQueue : public PriorityQueue<SpansCellPtr>
	{
	public:
		CellQueue(int32_t size);
		virtual ~CellQueue();
		
		LUCENE_CLASS(CellQueue);
	
	protected:
		virtual bool lessThan(const SpansCellPtr& first, const SpansCellPtr& second);
	};
}
