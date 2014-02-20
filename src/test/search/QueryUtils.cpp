/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "QueryUtils.h"
#include "Query.h"
#include "CheckHits.h"
#include "IndexSearcher.h"
#include "IndexReader.h"
#include "MultiReader.h"
#include "MultiSearcher.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "MatchAllDocsQuery.h"
#include "Scorer.h"
#include "Weight.h"
#include "DocIdSetIterator.h"
#include "ReaderUtil.h"
#include "MiscUtils.h"

namespace Lucene {

QueryUtils::~QueryUtils() {
}

void QueryUtils::check(const QueryPtr& q) {
    checkHashEquals(q);
}

class WhackyQuery : public Query {
public:
    virtual ~WhackyQuery() {
    }

public:
    virtual String toString(const String& field) {
        return L"My Whacky Query";
    }

    virtual bool equals(const LuceneObjectPtr& other) {
        if (!MiscUtils::typeOf<WhackyQuery>(other)) {
            return false;
        }
        return Query::equals(other);
    }
};

void QueryUtils::checkHashEquals(const QueryPtr& q) {
    QueryPtr q2 = boost::dynamic_pointer_cast<Query>(q->clone());
    checkEqual(q, q2);

    QueryPtr q3 = boost::dynamic_pointer_cast<Query>(q->clone());
    q3->setBoost(7.21792348);
    checkUnequal(q, q3);

    // test that a class check is done so that no exception is thrown in the implementation of equals()
    QueryPtr whacky = newLucene<WhackyQuery>();
    whacky->setBoost(q->getBoost());
    checkUnequal(q, whacky);
}

void QueryUtils::checkEqual(const QueryPtr& q1, const QueryPtr& q2) {
    EXPECT_TRUE(q1->equals(q2));
    EXPECT_EQ(q1->hashCode(), q2->hashCode());
}

void QueryUtils::checkUnequal(const QueryPtr& q1, const QueryPtr& q2) {
    EXPECT_TRUE(!q1->equals(q2));
    EXPECT_TRUE(!q2->equals(q1));

    // possible this test can fail on a hash collision... if that happens, please change
    // test to use a different example.
    EXPECT_NE(q1->hashCode(), q2->hashCode());
}

void QueryUtils::checkExplanations(const QueryPtr& q, const SearcherPtr& s) {
    CheckHits::checkExplanations(q, L"", s, true);
}

void QueryUtils::check(const QueryPtr& q1, const SearcherPtr& s) {
    check(q1, s, true);
}

void QueryUtils::check(const QueryPtr& q1, const SearcherPtr& s, bool wrap) {
    check(q1);
    if (s) {
        IndexSearcherPtr is = boost::dynamic_pointer_cast<IndexSearcher>(s);
        if (is) {
            checkFirstSkipTo(q1, is);
            checkSkipTo(q1, is);
            if (wrap) {
                check(q1, wrapUnderlyingReader(is, -1), false);
                check(q1, wrapUnderlyingReader(is,  0), false);
                check(q1, wrapUnderlyingReader(is, +1), false);
            }
        }
        if (wrap) {
            check(q1, wrapSearcher(s, -1), false);
            check(q1, wrapSearcher(s,  0), false);
            check(q1, wrapSearcher(s, +1), false);
        }
        checkExplanations(q1, s);

        QueryPtr q2 = boost::dynamic_pointer_cast<Query>(q1->clone());
        checkEqual(s->rewrite(q1), s->rewrite(q2));
    }
}

IndexSearcherPtr QueryUtils::wrapUnderlyingReader(const IndexSearcherPtr& s, int32_t edge) {
    IndexReaderPtr r = s->getIndexReader();

    // we can't put deleted docs before the nested reader, because it will throw off the docIds
    Collection<IndexReaderPtr> readers = newCollection<IndexReaderPtr>(
            edge < 0 ? r : IndexReader::open(makeEmptyIndex(0), true),
            IndexReader::open(makeEmptyIndex(0), true),
            newLucene<MultiReader>(newCollection<IndexReaderPtr>(
                                       IndexReader::open(makeEmptyIndex(edge < 0 ? 4 : 0), true),
                                       IndexReader::open(makeEmptyIndex(0), true),
                                       0 == edge ? r : IndexReader::open(makeEmptyIndex(0), true)
                                   )),
            IndexReader::open(makeEmptyIndex(0 < edge ? 0 : 7), true),
            IndexReader::open(makeEmptyIndex(0), true),
            newLucene<MultiReader>(newCollection<IndexReaderPtr>(
                                       IndexReader::open(makeEmptyIndex(0 < edge ? 0 : 5), true),
                                       IndexReader::open(makeEmptyIndex(0), true),
                                       0 < edge ? r : IndexReader::open(makeEmptyIndex(0), true)
                                   ))
                                         );
    IndexSearcherPtr out = newLucene<IndexSearcher>(newLucene<MultiReader>(readers));
    out->setSimilarity(s->getSimilarity());
    return out;
}

MultiSearcherPtr QueryUtils::wrapSearcher(const SearcherPtr& s, int32_t edge) {
    // we can't put deleted docs before the nested reader, because it will through off the docIds
    Collection<SearchablePtr> searchers = newCollection<SearchablePtr>(
            edge < 0 ? s : newLucene<IndexSearcher>(makeEmptyIndex(0), true),
            newLucene<MultiSearcher>(newCollection<SearchablePtr>(
                                         newLucene<IndexSearcher>(makeEmptyIndex(edge < 0 ? 65 : 0), true),
                                         newLucene<IndexSearcher>(makeEmptyIndex(0), true),
                                         0 == edge ? s : newLucene<IndexSearcher>(makeEmptyIndex(0), true)
                                     )),
            newLucene<IndexSearcher>(makeEmptyIndex(0 < edge ? 0 : 3), true),
            newLucene<IndexSearcher>(makeEmptyIndex(0), true),
            newLucene<MultiSearcher>(newCollection<SearchablePtr>(
                                         newLucene<IndexSearcher>(makeEmptyIndex(0 < edge ? 0 : 5), true),
                                         newLucene<IndexSearcher>(makeEmptyIndex(0), true),
                                         0 < edge ? s : newLucene<IndexSearcher>(makeEmptyIndex(0), true)
                                     ))
                                          );
    MultiSearcherPtr out = newLucene<MultiSearcher>(searchers);
    out->setSimilarity(s->getSimilarity());
    return out;
}

RAMDirectoryPtr QueryUtils::makeEmptyIndex(int32_t numDeletedDocs) {
    RAMDirectoryPtr d = newLucene<RAMDirectory>();
    IndexWriterPtr w = newLucene<IndexWriter>(d, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < numDeletedDocs; ++i) {
        w->addDocument(newLucene<Document>());
    }
    w->commit();
    w->deleteDocuments(newLucene<MatchAllDocsQuery>());
    w->commit();

    if (0 < numDeletedDocs) {
        EXPECT_TRUE(w->hasDeletions());
    }

    EXPECT_EQ(numDeletedDocs, w->maxDoc());
    EXPECT_EQ(0, w->numDocs());
    w->close();
    IndexReaderPtr r = IndexReader::open(d, true);
    EXPECT_EQ(numDeletedDocs, r->numDeletedDocs());
    r->close();
    return d;
}

namespace CheckSkipTo {

class SkipCollector : public Collector {
public:
    SkipCollector(const QueryPtr& q, const IndexSearcherPtr& s, Collection<int32_t> lastDoc, Collection<int32_t> order, Collection<int32_t> opidx, Collection<IndexReaderPtr> lastReader) {
        this->q = q;
        this->s = s;
        this->lastDoc = lastDoc;
        this->order = order;
        this->opidx = opidx;
        this->lastReader = lastReader;
    }

    virtual ~SkipCollector() {
    }

protected:
    QueryPtr q;
    IndexSearcherPtr s;
    Collection<int32_t> lastDoc;
    Collection<int32_t> order;
    Collection<int32_t> opidx;

    ScorerPtr sc;
    IndexReaderPtr reader;
    ScorerPtr scorer;
    Collection<IndexReaderPtr> lastReader;

public:
    virtual void setScorer(const ScorerPtr& scorer) {
        this->sc = scorer;
    }

    virtual void collect(int32_t doc) {
        double score = sc->score();
        lastDoc[0] = doc;
        if (!scorer) {
            WeightPtr w = q->weight(s);
            scorer = w->scorer(reader, true, false);
        }

        int32_t skip_op = 0;
        int32_t next_op = 1;
        double maxDiff = 1e-5;

        int32_t op = order[(opidx[0]++) % order.size()];
        bool more = op == skip_op ?
                    (scorer->advance(scorer->docID() + 1) != DocIdSetIterator::NO_MORE_DOCS) :
                    (scorer->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
        int32_t scorerDoc = scorer->docID();
        double scorerScore = scorer->score();
        double scorerScore2 = scorer->score();
        double scoreDiff = std::abs(score - scorerScore);
        double scorerDiff = std::abs(scorerScore2 - scorerScore);
        if (!more || doc != scorerDoc || scoreDiff > maxDiff || scorerDiff > maxDiff) {
            StringStream sbord;
            for (int32_t i = 0; i < order.size(); ++i) {
                sbord << (order[i] == skip_op ? L" skip()" : L" next()");
            }
            StringStream message;
            message << L"ERROR matching docs:\n\t"
                    << (doc != scorerDoc ? L"--> " : L"")
                    << L"doc=" << doc << L", scorerDoc=" << scorerDoc
                    << L"\n\t" << (!more ? L"--> " : L"") << L"tscorer.more="
                    << more << L"\n\t" << (scoreDiff > maxDiff ? L"--> " : L"")
                    << L"scorerScore=" << scorerScore << L" scoreDiff="
                    << scoreDiff << L" maxDiff=" << maxDiff << L"\n\t"
                    << (scorerDiff > maxDiff ? L"--> " : L"") << L"scorerScore2="
                    << scorerScore2 << L" scorerDiff=" << scorerDiff
                    << L"\n\thitCollector.doc=" << doc << L" score="
                    << score << L"\n\t Scorer=" << scorer << L"\n\t Query="
                    << q->toString() + L"  " << L"\n\t Searcher=" + s->toString()
                    << L"\n\t Order=" << sbord.str() << L"\n\t Op="
                    << (op == skip_op ? L" skip()" : L" next()");
            FAIL() << StringUtils::toUTF8(message.str());
        }
    }

    virtual void setNextReader(const IndexReaderPtr& reader, int32_t docBase) {
        // confirm that skipping beyond the last doc, on the previous reader, hits NO_MORE_DOCS
        if (lastReader[0]) {
            IndexReaderPtr previousReader = lastReader[0];
            WeightPtr w = q->weight(newLucene<IndexSearcher>(previousReader));
            ScorerPtr scorer = w->scorer(previousReader, true, false);
            if (scorer) {
                bool more = (scorer->advance(lastDoc[0] + 1) != DocIdSetIterator::NO_MORE_DOCS);
                EXPECT_TRUE(!more);
            }
        }
        this->reader = reader;
        this->scorer.reset();
        lastDoc[0] = -1;
    }

    virtual bool acceptsDocsOutOfOrder() {
        return true;
    }
};

}

void QueryUtils::checkSkipTo(const QueryPtr& q, const IndexSearcherPtr& s) {
    if (q->weight(s)->scoresDocsOutOfOrder()) {
        return;    // in this case order of skipTo() might differ from that of next().
    }

    int32_t skip_op = 0;
    int32_t next_op = 1;
    Collection< Collection<int32_t> > orders = newCollection< Collection<int32_t> >(
                newCollection<int32_t>(next_op),
                newCollection<int32_t>(skip_op),
                newCollection<int32_t>(skip_op, next_op),
                newCollection<int32_t>(next_op, skip_op),
                newCollection<int32_t>(skip_op, skip_op, next_op, next_op),
                newCollection<int32_t>(next_op, next_op, skip_op, skip_op),
                newCollection<int32_t>(skip_op, skip_op, skip_op, next_op, next_op)
            );

    Collection<IndexReaderPtr> lastReader = Collection<IndexReaderPtr>::newInstance(1);

    for (int32_t k = 0; k < orders.size(); ++k) {
        Collection<int32_t> order = orders[k];
        Collection<int32_t> opidx = newCollection<int32_t>(0);
        Collection<int32_t> lastDoc = newCollection<int32_t>(-1);

        s->search(q, newLucene<CheckSkipTo::SkipCollector>(q, s, lastDoc, order, opidx, lastReader));

        if (lastReader[0]) {
            // confirm that skipping beyond the last doc, on the previous reader, hits NO_MORE_DOCS
            IndexReaderPtr previousReader = lastReader[0];
            WeightPtr w = q->weight(newLucene<IndexSearcher>(previousReader));
            ScorerPtr scorer = w->scorer(previousReader, true, false);
            if (scorer) {
                bool more = (scorer->advance(lastDoc[0] + 1) != DocIdSetIterator::NO_MORE_DOCS);
                EXPECT_TRUE(!more);
            }
        }
    }
}

namespace CheckFirstSkipTo {

class SkipCollector : public Collector {
public:
    SkipCollector(const QueryPtr& q, const IndexSearcherPtr& s, Collection<int32_t> lastDoc, Collection<IndexReaderPtr> lastReader) {
        this->q = q;
        this->s = s;
        this->lastDoc = lastDoc;
        this->lastReader = lastReader;
    }

    virtual ~SkipCollector() {
    }

protected:
    QueryPtr q;
    IndexSearcherPtr s;
    Collection<int32_t> lastDoc;
    Collection<IndexReaderPtr> lastReader;

    ScorerPtr scorer;
    IndexReaderPtr reader;

public:
    virtual void setScorer(const ScorerPtr& scorer) {
        this->scorer = scorer;
    }

    virtual void collect(int32_t doc) {
        double score = scorer->score();
        lastDoc[0] = doc;
        for (int32_t i = lastDoc[0] + 1; i <= doc; ++i) {
            WeightPtr w = q->weight(s);
            ScorerPtr scorer = w->scorer(reader, true, false);
            EXPECT_TRUE(scorer->advance(i) != DocIdSetIterator::NO_MORE_DOCS);
            EXPECT_EQ(doc, scorer->docID());
            double skipToScore = scorer->score();
            EXPECT_NEAR(skipToScore, scorer->score(), 1e-5);
            EXPECT_NEAR(score, skipToScore, 1e-5);
        }
        lastDoc[0] = doc;
    }

    virtual void setNextReader(const IndexReaderPtr& reader, int32_t docBase) {
        // confirm that skipping beyond the last doc, on the previous reader, hits NO_MORE_DOCS
        if (lastReader[0]) {
            IndexReaderPtr previousReader = lastReader[0];
            WeightPtr w = q->weight(newLucene<IndexSearcher>(previousReader));
            ScorerPtr scorer = w->scorer(previousReader, true, false);
            if (scorer) {
                bool more = (scorer->advance(lastDoc[0] + 1) != DocIdSetIterator::NO_MORE_DOCS);
                EXPECT_TRUE(!more);
            }
        }

        lastReader[0] = reader;
        this->reader = reader;
        lastDoc[0] = -1;
    }

    virtual bool acceptsDocsOutOfOrder() {
        return false;
    }
};

}

void QueryUtils::checkFirstSkipTo(const QueryPtr& q, const IndexSearcherPtr& s) {
    Collection<int32_t> lastDoc = newCollection<int32_t>(-1);
    Collection<IndexReaderPtr> lastReader = Collection<IndexReaderPtr>::newInstance(1);

    s->search(q, newLucene<CheckFirstSkipTo::SkipCollector>(q, s, lastDoc, lastReader));

    if (lastReader[0]) {
        // confirm that skipping beyond the last doc, on the previous reader, hits NO_MORE_DOCS
        IndexReaderPtr previousReader = lastReader[0];
        WeightPtr w = q->weight(newLucene<IndexSearcher>(previousReader));
        ScorerPtr scorer = w->scorer(previousReader, true, false);
        if (scorer) {
            bool more = (scorer->advance(lastDoc[0] + 1) != DocIdSetIterator::NO_MORE_DOCS);
            EXPECT_TRUE(!more);
        }
    }
}

}
