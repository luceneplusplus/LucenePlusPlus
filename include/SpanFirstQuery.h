/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "SpanQuery.h"
#include "Spans.h"

namespace Lucene
{
	/// Matches spans near the beginning of a field.
	class LPPAPI SpanFirstQuery : public SpanQuery
	{
	public:
		/// Construct a SpanFirstQuery matching spans in match whose end position is less than or equal to end.
		SpanFirstQuery(SpanQueryPtr match, int32_t end);
		virtual ~SpanFirstQuery();
		
		LUCENE_CLASS(SpanFirstQuery);
	
	protected:
		SpanQueryPtr match;
		int32_t end;
	
	public:
		using SpanQuery::toString;
		
		/// Return the SpanQuery whose matches are filtered.
		SpanQueryPtr getMatch();
		
		/// Return the maximum end position permitted in a match.
		int32_t getEnd();
		
		virtual String getField();
		virtual String toString(const String& field);
		virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
		virtual void extractTerms(SetTerm terms);
		virtual SpansPtr getSpans(IndexReaderPtr reader);
		virtual QueryPtr rewrite(IndexReaderPtr reader);
		
		virtual bool equals(LuceneObjectPtr other);
		virtual int32_t hashCode();
		
		friend class FirstSpans;
	};
	
	class LPPAPI FirstSpans : public Spans
	{
	public:
		FirstSpans(SpanFirstQueryPtr query, SpansPtr spans);
		virtual ~FirstSpans();
		
		LUCENE_CLASS(FirstSpans);
	
	protected:
		SpanFirstQueryPtr query;
		SpansPtr spans;
	
	public:
		virtual bool next();
		virtual bool skipTo(int32_t target);
		virtual int32_t doc();
		virtual int32_t start();
		virtual int32_t end();
		virtual Collection<ByteArray> getPayload();
		virtual bool isPayloadAvailable();
	};
}
