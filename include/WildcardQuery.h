/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef WILDCARDQUERY_H
#define WILDCARDQUERY_H

#include "MultiTermQuery.h"

namespace Lucene
{
    /// Implements the wildcard search query.  Supported wildcards are *, which matches any character sequence 
    /// (including the empty one), and ?, which matches any single character. Note this query can be slow, as 
    /// it needs to iterate over many terms.  In order to prevent extremely slow WildcardQueries, a Wildcard 
    /// term should not start with one of the wildcards * or ?.
    ///
    /// This query uses the {@link MultiTermQuery#CONSTANT_SCORE_AUTO_REWRITE_DEFAULT} rewrite method.
    /// @see WildcardTermEnum
    class LPPAPI WildcardQuery : public MultiTermQuery
    {
    public:
        WildcardQuery(TermPtr term);
        virtual ~WildcardQuery();
    
        LUCENE_CLASS(WildcardQuery);
    
    protected:
        bool termContainsWildcard;
        bool termIsPrefix;
        TermPtr term;
    
    public:
        using MultiTermQuery::toString;
        
        /// Returns the pattern term.
        TermPtr getTerm();
        
        virtual QueryPtr rewrite(IndexReaderPtr reader);
        
        /// Prints a user-readable version of this query.
        virtual String toString(const String& field);
        
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        virtual int32_t hashCode();
        virtual bool equals(LuceneObjectPtr other);
        
    protected:
        virtual FilteredTermEnumPtr getEnum(IndexReaderPtr reader);
    };
}

#endif
