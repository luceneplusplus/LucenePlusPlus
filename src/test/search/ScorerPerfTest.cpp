/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "Random.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "IndexSearcher.h"
#include "BitSet.h"
#include "BooleanQuery.h"
#include "Collector.h"
#include "ConstantScoreQuery.h"
#include "Filter.h"
#include "DocIdBitSet.h"

using namespace Lucene;

DECLARE_SHARED_PTR(CountingHitCollector)
DECLARE_SHARED_PTR(MatchingHitCollector)

class CountingHitCollector : public Collector {
public:
    CountingHitCollector() {
        count = 0;
        sum = 0;
        docBase = 0;
    }

    virtual ~CountingHitCollector() {
    }

public:
    int32_t count;
    int32_t sum;
    int32_t docBase;

public:
    virtual void setScorer(const ScorerPtr& scorer) {
    }

    virtual void collect(int32_t doc) {
        ++count;
        sum += docBase + doc; // use it to avoid any possibility of being optimized away
    }

    int32_t getCount() {
        return count;
    }

    int32_t getSum() {
        return sum;
    }

    virtual void setNextReader(const IndexReaderPtr& reader, int32_t docBase) {
        this->docBase = docBase;
    }

    virtual bool acceptsDocsOutOfOrder() {
        return true;
    }
};

class MatchingHitCollector : public CountingHitCollector {
public:
    MatchingHitCollector(const BitSetPtr& answer) {
        this->answer = answer;
        this->pos = -1;
    }

    virtual ~MatchingHitCollector() {
    }

public:
    BitSetPtr answer;
    int32_t pos;

public:
    virtual void collect(int32_t doc) {
        pos = answer->nextSetBit(pos + 1);
        if (pos != doc + docBase) {
            boost::throw_exception(RuntimeException(L"Expected doc " + StringUtils::toString(pos) + L" but got " + StringUtils::toString(doc + docBase)));
        }
        CountingHitCollector::collect(doc);
    }
};

class AddClauseFilter : public Filter {
public:
    AddClauseFilter(const BitSetPtr& rnd) {
        this->rnd = rnd;
    }

    virtual ~AddClauseFilter() {
    }

protected:
    BitSetPtr rnd;

public:
    virtual DocIdSetPtr getDocIdSet(const IndexReaderPtr& reader) {
        return newLucene<DocIdBitSet>(rnd);
    }
};

class ScorerPerfTest : public LuceneTestFixture {
public:
    ScorerPerfTest() {
        r = newLucene<Random>();
        createDummySearcher();
    }

    virtual ~ScorerPerfTest() {
        s->close();
    }

public:
    RandomPtr r;
    Collection<BitSetPtr> sets;
    Collection<TermPtr> terms;
    IndexSearcherPtr s;

public:
    void createDummySearcher() {
        // Create a dummy index with nothing in it.
        RAMDirectoryPtr rd = newLucene<RAMDirectory>();
        IndexWriterPtr iw = newLucene<IndexWriter>(rd, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        iw->addDocument(newLucene<Document>());
        iw->close();
        s = newLucene<IndexSearcher>(rd, true);
    }

    BitSetPtr randBitSet(int32_t sz, int32_t numBitsToSet) {
        BitSetPtr set = newLucene<BitSet>(sz);
        for (int32_t i = 0; i < numBitsToSet; ++i) {
            set->set(r->nextInt(sz));
        }
        return set;
    }

    Collection<BitSetPtr> randBitSets(int32_t numSets, int32_t setSize) {
        Collection<BitSetPtr> sets = Collection<BitSetPtr>::newInstance(numSets);
        for (int32_t i = 0; i < sets.size(); ++i) {
            sets[i] = randBitSet(setSize, r->nextInt(setSize));
        }
        return sets;
    }

    void doConjunctions(int32_t iter, int32_t maxClauses) {
        for (int32_t i = 0; i < iter; ++i) {
            int32_t numClauses = r->nextInt(maxClauses - 1) + 2; // min 2 clauses
            BooleanQueryPtr bq = newLucene<BooleanQuery>();
            BitSetPtr result;
            for (int32_t j = 0; j < numClauses; ++j) {
                result = addClause(bq, result);
            }

            CountingHitCollectorPtr hc = newLucene<MatchingHitCollector>(result);
            s->search(bq, hc);

            EXPECT_EQ(result->cardinality(), hc->getCount());
        }
    }

    void doNestedConjunctions(int32_t iter, int32_t maxOuterClauses, int32_t maxClauses) {
        for (int32_t i = 0; i < iter; ++i) {
            int32_t oClauses = r->nextInt(maxOuterClauses - 1) + 2;
            BooleanQueryPtr oq = newLucene<BooleanQuery>();
            BitSetPtr result;

            for (int32_t o = 0; o < oClauses; ++o) {
                int32_t numClauses = r->nextInt(maxClauses - 1) + 2; // min 2 clauses
                BooleanQueryPtr bq = newLucene<BooleanQuery>();
                for (int32_t j = 0; j < numClauses; ++j) {
                    result = addClause(bq, result);
                }
                oq->add(bq, BooleanClause::MUST);
            }

            CountingHitCollectorPtr hc = newLucene<MatchingHitCollector>(result);
            s->search(oq, hc);

            EXPECT_EQ(result->cardinality(), hc->getCount());
        }
    }

    BitSetPtr addClause(const BooleanQueryPtr& bq, const BitSetPtr& result) {
        BitSetPtr rnd = sets[r->nextInt(sets.size())];
        QueryPtr q = newLucene<ConstantScoreQuery>(newLucene<AddClauseFilter>(rnd));
        bq->add(q, BooleanClause::MUST);
        BitSetPtr _result(result);
        if (!_result) {
            _result = boost::dynamic_pointer_cast<BitSet>(rnd->clone());
        } else {
            _result->_and(rnd);
        }
        return _result;
    }
};

TEST_F(ScorerPerfTest, testConjunctions) {
    // test many small sets... the bugs will be found on boundary conditions
    sets = randBitSets(1000, 10);
    doConjunctions(10000, 5);
    doNestedConjunctions(10000, 3, 3);
}
