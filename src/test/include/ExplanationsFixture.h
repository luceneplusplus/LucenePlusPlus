/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef EXPLANATIONSFIXTURE_H
#define EXPLANATIONSFIXTURE_H

#include "LuceneTestFixture.h"

namespace Lucene {

class ExplanationsFixture : public LuceneTestFixture {
public:
    ExplanationsFixture();
    virtual ~ExplanationsFixture();

public:
    static const String KEY;
    static const String FIELD;

protected:
    IndexSearcherPtr searcher;
    QueryParserPtr qp;
    Collection<String> docFields;

public:
    virtual SpanTermQueryPtr st(const String& s);
    virtual SpanFirstQueryPtr sf(const String& s, int32_t b);
    virtual SpanNotQueryPtr snot(const SpanQueryPtr& i, const SpanQueryPtr& e);
    virtual SpanOrQueryPtr sor(const String& s, const String& e);
    virtual SpanOrQueryPtr sor(const SpanQueryPtr& s, const SpanQueryPtr& e);
    virtual SpanOrQueryPtr sor(const String& s, const String& m, const String& e);
    virtual SpanOrQueryPtr sor(const SpanQueryPtr& s, const SpanQueryPtr& m, const SpanQueryPtr& e);
    virtual SpanNearQueryPtr snear(const String& s, const String& e, int32_t slop, bool inOrder);
    virtual SpanNearQueryPtr snear(const SpanQueryPtr& s, const SpanQueryPtr& e, int32_t slop, bool inOrder);
    virtual SpanNearQueryPtr snear(const String& s, const String& m, const String& e, int32_t slop, bool inOrder);
    virtual SpanNearQueryPtr snear(const SpanQueryPtr& s, const SpanQueryPtr& m, const SpanQueryPtr& e, int32_t slop, bool inOrder);
    virtual QueryPtr optB(const String& q);
    virtual QueryPtr optB(const QueryPtr& q);
    virtual QueryPtr reqB(const String& q);
    virtual QueryPtr reqB(const QueryPtr& q);
    virtual Collection<TermPtr> ta(Collection<String> s);

    /// Check the expDocNrs first, then check the query (and the explanations)
    virtual void qtest(const String& queryText, Collection<int32_t> expDocNrs);
    virtual void qtest(const QueryPtr& q, Collection<int32_t> expDocNrs);

    /// Tests a query using qtest after wrapping it with both optB and reqB
    virtual void bqtest(const QueryPtr& q, Collection<int32_t> expDocNrs);
    virtual void bqtest(const String& queryText, Collection<int32_t> expDocNrs);

    virtual QueryPtr makeQuery(const String& queryText);
};

}

#endif
