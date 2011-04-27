/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SPANPOSITIONCHECKQUERY_H
#define SPANPOSITIONCHECKQUERY_H

#include "SpanQuery.h"

namespace Lucene
{
    class LPPAPI SpanPositionCheckQuery : public SpanQuery
    {
    public:
        /// Construct a SpanPositionCheckQuery matching spans in match whose end position is less 
        /// than or equal to end.
        SpanPositionCheckQuery(SpanQueryPtr match);
        virtual ~SpanPositionCheckQuery();
        
        LUCENE_CLASS(SpanPositionCheckQuery);
    
    protected:
        SpanQueryPtr match;
    
        /// Return value if the match should be accepted {@code YES}, rejected {@code NO}, or 
        /// rejected and enumeration should advance to the next document {@code NO_AND_ADVANCE}.
        /// @see #acceptPosition(Spans)
        enum AcceptStatus { YES, NO, NO_AND_ADVANCE };
        
    public:
        /// Return the SpanQuery whose matches are filtered.
        virtual SpanQueryPtr getMatch();
        
        virtual String getField();
        virtual void extractTerms(SetTerm terms);
        
        virtual SpansPtr getSpans(IndexReaderPtr reader);
        virtual QueryPtr rewrite(IndexReaderPtr reader);
    
    protected:
        /// Implementing classes are required to return whether the current position is a match 
        /// for the passed in "match" {@link SpanQuery}.
        ///
        /// This is only called if the underlying {@link Spans#next()} for the match is successful
        ///
        /// @param spans The {@link Spans} instance, positioned at the spot to check
        /// @return whether the match is accepted, rejected, or rejected and should move to the next doc.
        /// @see Spans#next()
        virtual AcceptStatus acceptPosition(SpansPtr spans) = 0;
        
        friend class PositionCheckSpan;
    };
}

#endif
