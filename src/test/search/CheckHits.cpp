/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <boost/algorithm/string.hpp>
#include "CheckHits.h"
#include "Searcher.h"
#include "Explanation.h"
#include "QueryUtils.h"
#include "Collector.h"
#include "Query.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "MultiSearcher.h"
#include "IndexSearcher.h"
#include "Scorer.h"
#include "MiscUtils.h"

namespace Lucene {

/// Some explains methods calculate their values though a slightly different order of operations
/// from the actual scoring method - this allows for a small amount of variation
const double CheckHits::EXPLAIN_SCORE_TOLERANCE_DELTA = 0.00005;

class SetCollector : public Collector {
public:
    SetCollector(Set<int32_t> bag) {
        this->bag = bag;
        this->base = 0;
    }

    virtual ~SetCollector() {
    }

public:
    Set<int32_t> bag;

protected:
    int32_t base;

public:
    virtual void setScorer(const ScorerPtr& scorer) {
    }

    virtual void collect(int32_t doc) {
        bag.add(doc + base);
    }

    virtual void setNextReader(const IndexReaderPtr& reader, int32_t docBase) {
        base = docBase;
    }

    virtual bool acceptsDocsOutOfOrder() {
        return true;
    }
};

/// Asserts that the score explanation for every document matching a query corresponds with the true score.
///
/// NOTE: this HitCollector should only be used with the Query and Searcher specified at when it is constructed.
class ExplanationAsserter : public Collector {
public:
    ExplanationAsserter(const QueryPtr& q, const String& defaultFieldName, const SearcherPtr& s, bool deep = false) {
        this->q=q;
        this->s=s;
        this->d = q->toString(defaultFieldName);
        this->deep=deep;
        this->base = 0;
    }

    virtual ~ExplanationAsserter() {
    }

public:
    QueryPtr q;
    SearcherPtr s;
    String d;
    bool deep;
    ScorerPtr scorer;

protected:
    int32_t base;

public:
    virtual void setScorer(const ScorerPtr& scorer) {
        this->scorer = scorer;
    }

    virtual void collect(int32_t doc) {
        doc = doc + base;
        ExplanationPtr exp = s->explain(q, doc);

        EXPECT_TRUE(exp);
        CheckHits::verifyExplanation(d, doc, scorer->score(), deep, exp);
    }

    virtual void setNextReader(const IndexReaderPtr& reader, int32_t docBase) {
        base = docBase;
    }

    virtual bool acceptsDocsOutOfOrder() {
        return true;
    }
};

CheckHits::~CheckHits() {
}

void CheckHits::checkNoMatchExplanations(const QueryPtr& q, const String& defaultFieldName, const SearcherPtr& searcher, Collection<int32_t> results) {
    String d = q->toString(defaultFieldName);
    Set<int32_t> ignore = Set<int32_t>::newInstance();
    for (int32_t i = 0; i < results.size(); ++i) {
        ignore.add(results[i]);
    }

    int32_t maxDoc = searcher->maxDoc();
    for (int32_t doc = 0; doc < maxDoc; ++doc) {
        if (ignore.contains(doc)) {
            continue;
        }

        ExplanationPtr exp = searcher->explain(q, doc);
        EXPECT_TRUE(exp);
        EXPECT_EQ(0.0, exp->getValue());
    }
}

void CheckHits::checkHitCollector(const QueryPtr& query, const String& defaultFieldName, const SearcherPtr& searcher, Collection<int32_t> results) {
    QueryUtils::check(query, searcher);
    Set<int32_t> correct = Set<int32_t>::newInstance();
    for (int32_t i = 0; i < results.size(); ++i) {
        correct.add(results[i]);
    }

    Set<int32_t> actual = Set<int32_t>::newInstance();
    CollectorPtr c = newLucene<SetCollector>(actual);

    searcher->search(query, c);
    EXPECT_TRUE(correct.equals(actual));

    for (int32_t i = -1; i < 2; ++i) {
        actual.clear();
        QueryUtils::wrapSearcher(searcher, i)->search(query, c);
        EXPECT_TRUE(correct.equals(actual));
    }

    if (!MiscUtils::typeOf<IndexSearcher>(searcher)) {
        return;
    }

    for (int32_t i = -1; i < 2; ++i) {
        actual.clear();
        QueryUtils::wrapUnderlyingReader(boost::dynamic_pointer_cast<IndexSearcher>(searcher), i)->search(query, c);
        EXPECT_TRUE(correct.equals(actual));
    }
}

void CheckHits::checkHits(const QueryPtr& query, const String& defaultFieldName, const SearcherPtr& searcher, Collection<int32_t> results) {
    if (!MiscUtils::typeOf<IndexSearcher>(searcher)) {
        QueryUtils::check(query, searcher);
    }

    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    Set<int32_t> correct = Set<int32_t>::newInstance();
    for (int32_t i = 0; i < results.size(); ++i) {
        correct.add(results[i]);
    }

    Set<int32_t> actual = Set<int32_t>::newInstance();
    for (int32_t i = 0; i < hits.size(); ++i) {
        actual.add(hits[i]->doc);
    }

    EXPECT_TRUE(correct.equals(actual));

    QueryUtils::check(query, searcher);
}

void CheckHits::checkDocIds(Collection<int32_t> results, Collection<ScoreDocPtr> hits) {
    EXPECT_EQ(hits.size(), results.size());
    for (int32_t i = 0; i < results.size(); ++i) {
        EXPECT_EQ(results[i], hits[i]->doc);
    }
}

void CheckHits::checkHitsQuery(const QueryPtr& query, Collection<ScoreDocPtr> hits1, Collection<ScoreDocPtr> hits2, Collection<int32_t> results) {
    checkDocIds(results, hits1);
    checkDocIds(results, hits2);
    checkEqual(query, hits1, hits2);
}

void CheckHits::checkEqual(const QueryPtr& query, Collection<ScoreDocPtr> hits1, Collection<ScoreDocPtr> hits2) {
    double scoreTolerance = 1.0e-6;
    EXPECT_EQ(hits1.size(), hits2.size());
    for (int32_t i = 0; i < hits1.size(); ++i) {
        EXPECT_EQ(hits1[i]->doc, hits2[i]->doc);
        EXPECT_NEAR(hits1[i]->score, hits2[i]->score, scoreTolerance);
    }
}

void CheckHits::checkExplanations(const QueryPtr& query, const String& defaultFieldName, const SearcherPtr& searcher, bool deep) {
    searcher->search(query, newLucene<ExplanationAsserter>(query, defaultFieldName, searcher, deep));
}

void CheckHits::verifyExplanation(const String& q, int32_t doc, double score, bool deep, const ExplanationPtr& expl) {
    double value = expl->getValue();
    EXPECT_NEAR(score, value, EXPLAIN_SCORE_TOLERANCE_DELTA);

    if (!deep) {
        return;
    }

    Collection<ExplanationPtr> detail = expl->getDetails();
    if (detail) {
        if (detail.size() == 1) {
            // simple containment, no matter what the description says, just verify contained expl has same score
            verifyExplanation(q, doc, score, deep, detail[0]);
        } else {
            // explanation must either:
            // - end with one of: "product of:", "sum of:", "max of:", or
            // - have "max plus <x> times others" (where <x> is float).
            double x = 0;
            String descr = StringUtils::toLower(expl->getDescription());
            bool productOf = boost::ends_with(descr, L"product of:");
            bool sumOf = boost::ends_with(descr, L"sum of:");
            bool maxOf = boost::ends_with(descr, L"max of:");
            bool maxTimesOthers = false;
            if (!(productOf || sumOf || maxOf)) {
                // maybe 'max plus x times others'
                String::size_type k1 = descr.find(L"max plus ");
                if (k1 != String::npos) {
                    k1 += String(L"max plus ").length();
                    String::size_type k2 = descr.find(L" ", k1);
                    x = StringUtils::toDouble(descr.substr(k1));
                    String max(descr.substr(k2));
                    boost::trim(max);
                    if (max == L"times others of:") {
                        maxTimesOthers = true;
                    }
                }
            }
            EXPECT_TRUE(productOf || sumOf || maxOf || maxTimesOthers);
            double sum = 0.0;
            double product = 1.0;
            double max = 0.0;
            for (int32_t i = 0; i < detail.size(); ++i) {
                double dval = detail[i]->getValue();
                verifyExplanation(q, doc, dval, deep, detail[i]);
                product *= dval;
                sum += dval;
                max = std::max(max, dval);
            }
            double combined = 0.0;
            if (productOf) {
                combined = product;
            } else if (sumOf) {
                combined = sum;
            } else if (maxOf) {
                combined = max;
            } else if (maxTimesOthers) {
                combined = max + x * (sum - max);
            } else {
                FAIL() << "should never get here!";
            }

            EXPECT_NEAR(combined, value, EXPLAIN_SCORE_TOLERANCE_DELTA);
        }
    }
}

}
