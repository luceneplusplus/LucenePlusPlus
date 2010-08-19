/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "SpanQuery.h"
#include "PriorityQueue.h"
#include "Spans.h"

namespace Lucene
{
	/// Matches the union of its clauses.
	class LPPAPI SpanOrQuery : public SpanQuery
	{
	public:
		/// Construct a SpanOrQuery merging the provided clauses.
		SpanOrQuery(Collection<SpanQueryPtr> clauses);
		virtual ~SpanOrQuery();
		
		LUCENE_CLASS(SpanOrQuery);
	
	protected:
		Collection<SpanQueryPtr> clauses;
		String field;
	
	public:
		using SpanQuery::toString;
		
		/// Return the clauses whose spans are matched.
		Collection<SpanQueryPtr> getClauses();
	
		virtual String getField();
		virtual void extractTerms(SetTerm terms);
		virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
		virtual QueryPtr rewrite(IndexReaderPtr reader);
		virtual String toString(const String& field);
		virtual bool equals(LuceneObjectPtr other);
		virtual int32_t hashCode();		
		virtual SpansPtr getSpans(IndexReaderPtr reader);
		
		friend class OrSpans;
	};
	
	class LPPAPI SpanQueue : public PriorityQueue<SpansPtr>
	{
	public:
		SpanQueue(int32_t size);
		virtual ~SpanQueue();
		
		LUCENE_CLASS(SpanQueue);
	
	protected:
		virtual bool lessThan(const SpansPtr& first, const SpansPtr& second);
	};
	
	class LPPAPI OrSpans : public Spans
	{
	public:
		OrSpans(SpanOrQueryPtr query, IndexReaderPtr reader);
		virtual ~OrSpans();
		
		LUCENE_CLASS(OrSpans);
	
	protected:
		SpanOrQueryPtr query;
		IndexReaderPtr reader;
		SpanQueuePtr queue;
	
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
		bool initSpanQueue(int32_t target);
		SpansPtr top();
	};
}
