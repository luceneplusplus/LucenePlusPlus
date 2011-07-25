/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef TERMCOLLECTINGREWRITE_H
#define TERMCOLLECTINGREWRITE_H

#include "MultiTermQuery.h"

namespace Lucene
{
    class LPPAPI TermCollectingRewrite : public RewriteMethod
    {
    public:
        virtual ~TermCollectingRewrite();

        LUCENE_CLASS(TermCollectingRewrite);

    protected:
        /// Return a suitable top-level Query for holding all expanded terms.
        virtual QueryPtr getTopLevelQuery() = 0;

        /// Add a MultiTermQuery term to the top-level query
        virtual void addClause(QueryPtr topLevel, TermPtr term, double boost) = 0;

        virtual void collectTerms(IndexReaderPtr reader, MultiTermQueryPtr query, TermCollectorPtr collector);

        friend class ScoringRewriteTermCollector;
    };

    class LPPAPI TermCollector : public LuceneObject
    {
    public:
        virtual ~TermCollector();
        LUCENE_CLASS(TermCollector);

    public:
        /// return false to stop collecting
        virtual bool collect(TermPtr t, double boost) = 0;
    };
}

#endif
