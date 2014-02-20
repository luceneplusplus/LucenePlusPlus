/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "Explanation.h"
#include "TestUtils.h"
#include "IndexSearcher.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "SimpleAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "TermQuery.h"
#include "Term.h"
#include "CheckHits.h"
#include "PhraseQuery.h"
#include "BooleanQuery.h"
#include "SpanTermQuery.h"
#include "SpanNearQuery.h"
#include "QueryUtils.h"
#include "SpanNotQuery.h"
#include "SpanOrQuery.h"
#include "SpanFirstQuery.h"

using namespace Lucene;

/// Tests basic search capabilities.
///
/// Uses a collection of 1000 documents, each the english rendition of their document number.
/// For example, the document numbered 333 has text "three hundred thirty three".
///
/// Tests are each a single query, and its hits are checked to ensure that all and only the
/// correct documents are returned, thus providing end-to-end testing of the indexing and
/// search code.
class BasicSpansTest : public LuceneTestFixture {
public:
    BasicSpansTest() {
        RAMDirectoryPtr directory = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        for (int32_t i = 0; i < 1000; ++i) {
            DocumentPtr doc = newLucene<Document>();
            doc->add(newLucene<Field>(L"field", intToEnglish(i), Field::STORE_YES, Field::INDEX_ANALYZED));
            writer->addDocument(doc);
        }

        writer->close();
        searcher = newLucene<IndexSearcher>(directory, true);
    }

    virtual ~BasicSpansTest() {
    }

protected:
    IndexSearcherPtr searcher;

public:
    void checkHits(const QueryPtr& query, Collection<int32_t> results) {
        CheckHits::checkHits(query, L"field", searcher, results);
    }

    bool skipTo(const SpansPtr& s, int32_t target) {
        do {
            if (!s->next()) {
                return false;
            }
        } while (target > s->doc());
        return true;
    }
};

TEST_F(BasicSpansTest, testTerm) {
    QueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"field", L"seventy"));

    static const int32_t results[] = {
        70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 170, 171, 172, 173, 174, 175,
        176, 177, 178, 179, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279,
        370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 470, 471, 472, 473,
        474, 475, 476, 477, 478, 479, 570, 571, 572, 573, 574, 575, 576, 577,
        578, 579, 670, 671, 672, 673, 674, 675, 676, 677, 678, 679, 770, 771,
        772, 773, 774, 775, 776, 777, 778, 779, 870, 871, 872, 873, 874, 875,
        876, 877, 878, 879, 970, 971, 972, 973, 974, 975, 976, 977, 978, 979
    };
    checkHits(query, Collection<int32_t>::newInstance(results, results + SIZEOF_ARRAY(results)));
}

TEST_F(BasicSpansTest, testTerm2) {
    QueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"field", L"seventish"));
    checkHits(query, Collection<int32_t>::newInstance());
}

TEST_F(BasicSpansTest, testPhrase) {
    PhraseQueryPtr query = newLucene<PhraseQuery>();
    query->add(newLucene<Term>(L"field", L"seventy"));
    query->add(newLucene<Term>(L"field", L"seven"));

    static const int32_t results[] = {77, 177, 277, 377, 477, 577, 677, 777, 877, 977};
    checkHits(query, Collection<int32_t>::newInstance(results, results + SIZEOF_ARRAY(results)));
}

TEST_F(BasicSpansTest, testPhrase2) {
    PhraseQueryPtr query = newLucene<PhraseQuery>();
    query->add(newLucene<Term>(L"field", L"seventish"));
    query->add(newLucene<Term>(L"field", L"sevenon"));
    checkHits(query, Collection<int32_t>::newInstance());
}

TEST_F(BasicSpansTest, testBoolean) {
    BooleanQueryPtr query = newLucene<BooleanQuery>();
    query->add(newLucene<TermQuery>(newLucene<Term>(L"field", L"seventy")), BooleanClause::MUST);
    query->add(newLucene<TermQuery>(newLucene<Term>(L"field", L"seven")), BooleanClause::MUST);

    static const int32_t results[] = {
        77, 777, 177, 277, 377, 477, 577, 677, 770, 771, 772, 773, 774, 775, 776, 778, 779, 877, 977
    };
    checkHits(query, Collection<int32_t>::newInstance(results, results + SIZEOF_ARRAY(results)));
}

TEST_F(BasicSpansTest, testBoolean2) {
    BooleanQueryPtr query = newLucene<BooleanQuery>();
    query->add(newLucene<TermQuery>(newLucene<Term>(L"field", L"sevento")), BooleanClause::MUST);
    query->add(newLucene<TermQuery>(newLucene<Term>(L"field", L"sevenly")), BooleanClause::MUST);
    checkHits(query, Collection<int32_t>::newInstance());
}

TEST_F(BasicSpansTest, testSpanNearExact) {
    SpanTermQueryPtr term1 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"seventy"));
    SpanTermQueryPtr term2 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"seven"));
    SpanNearQueryPtr query = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(term1, term2), 0, true);

    static const int32_t results[] = {77, 177, 277, 377, 477, 577, 677, 777, 877, 977};
    checkHits(query, Collection<int32_t>::newInstance(results, results + SIZEOF_ARRAY(results)));

    EXPECT_TRUE(searcher->explain(query, 77)->getValue() > 0.0);
    EXPECT_TRUE(searcher->explain(query, 977)->getValue() > 0.0);

    QueryUtils::check(term1);
    QueryUtils::check(term2);
    QueryUtils::checkUnequal(term1, term2);
}

TEST_F(BasicSpansTest, testSpanNearUnordered) {
    SpanTermQueryPtr term1 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"nine"));
    SpanTermQueryPtr term2 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"six"));
    SpanNearQueryPtr query = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(term1, term2), 4, false);

    static const int32_t results[] = {609, 629, 639, 649, 659, 669, 679, 689, 699, 906, 926, 936, 946, 956, 966, 976, 986, 996};
    checkHits(query, Collection<int32_t>::newInstance(results, results + SIZEOF_ARRAY(results)));
}

TEST_F(BasicSpansTest, testSpanNearOrdered) {
    SpanTermQueryPtr term1 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"nine"));
    SpanTermQueryPtr term2 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"six"));
    SpanNearQueryPtr query = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(term1, term2), 4, true);

    static const int32_t results[] = {906, 926, 936, 946, 956, 966, 976, 986, 996};
    checkHits(query, Collection<int32_t>::newInstance(results, results + SIZEOF_ARRAY(results)));
}

TEST_F(BasicSpansTest, testSpanNot) {
    SpanTermQueryPtr term1 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"eight"));
    SpanTermQueryPtr term2 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"one"));
    SpanNearQueryPtr near1 = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(term1, term2), 4, true);

    SpanTermQueryPtr term3 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"forty"));
    SpanNotQueryPtr query = newLucene<SpanNotQuery>(near1, term3);

    static const int32_t results[] = {801, 821, 831, 851, 861, 871, 881, 891};
    checkHits(query, Collection<int32_t>::newInstance(results, results + SIZEOF_ARRAY(results)));

    EXPECT_TRUE(searcher->explain(query, 801)->getValue() > 0.0);
    EXPECT_TRUE(searcher->explain(query, 891)->getValue() > 0.0);
}

TEST_F(BasicSpansTest, testSpanWithMultipleNotSingle) {
    SpanTermQueryPtr term1 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"eight"));
    SpanTermQueryPtr term2 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"one"));
    SpanNearQueryPtr near1 = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(term1, term2), 4, true);

    SpanTermQueryPtr term3 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"forty"));
    SpanOrQueryPtr or1 = newLucene<SpanOrQuery>(newCollection<SpanQueryPtr>(term3));

    SpanNotQueryPtr query = newLucene<SpanNotQuery>(near1, or1);

    static const int32_t results[] = {801, 821, 831, 851, 861, 871, 881, 891};
    checkHits(query, Collection<int32_t>::newInstance(results, results + SIZEOF_ARRAY(results)));

    EXPECT_TRUE(searcher->explain(query, 801)->getValue() > 0.0);
    EXPECT_TRUE(searcher->explain(query, 891)->getValue() > 0.0);
}

TEST_F(BasicSpansTest, testSpanWithMultipleNotMany) {
    SpanTermQueryPtr term1 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"eight"));
    SpanTermQueryPtr term2 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"one"));
    SpanNearQueryPtr near1 = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(term1, term2), 4, true);
    SpanTermQueryPtr term3 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"forty"));
    SpanTermQueryPtr term4 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"sixty"));
    SpanTermQueryPtr term5 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"eighty"));

    SpanOrQueryPtr or1 = newLucene<SpanOrQuery>(newCollection<SpanQueryPtr>(term3, term4, term5));

    SpanNotQueryPtr query = newLucene<SpanNotQuery>(near1, or1);

    static const int32_t results[] = {801, 821, 831, 851, 871, 891};
    checkHits(query, Collection<int32_t>::newInstance(results, results + SIZEOF_ARRAY(results)));

    EXPECT_TRUE(searcher->explain(query, 801)->getValue() > 0.0);
    EXPECT_TRUE(searcher->explain(query, 891)->getValue() > 0.0);
}

TEST_F(BasicSpansTest, testNpeInSpanNearWithSpanNot) {
    SpanTermQueryPtr term1 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"eight"));
    SpanTermQueryPtr term2 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"one"));
    SpanNearQueryPtr near1 = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(term1, term2), 4, true);
    SpanTermQueryPtr hun = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"hundred"));
    SpanTermQueryPtr term3 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"forty"));
    SpanNearQueryPtr exclude1 = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(hun, term3), 1, true);

    SpanNotQueryPtr query = newLucene<SpanNotQuery>(near1, exclude1);

    static const int32_t results[] = {801, 821, 831, 851, 861, 871, 881, 891};
    checkHits(query, Collection<int32_t>::newInstance(results, results + SIZEOF_ARRAY(results)));

    EXPECT_TRUE(searcher->explain(query, 801)->getValue() > 0.0);
    EXPECT_TRUE(searcher->explain(query, 891)->getValue() > 0.0);
}

TEST_F(BasicSpansTest, testNpeInSpanNearInSpanFirstInSpanNot) {
    int32_t n = 5;
    SpanTermQueryPtr hun = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"hundred"));
    SpanTermQueryPtr term40 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"forty"));
    SpanTermQueryPtr term40c = boost::dynamic_pointer_cast<SpanTermQuery>(term40->clone());

    SpanFirstQueryPtr include = newLucene<SpanFirstQuery>(term40, n);
    SpanNearQueryPtr near1 = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(hun, term40c), n - 1, true);
    SpanFirstQueryPtr exclude = newLucene<SpanFirstQuery>(near1, n - 1);
    SpanNotQueryPtr query = newLucene<SpanNotQuery>(include, exclude);

    static const int32_t results[] = {40, 41, 42, 43, 44, 45, 46, 47, 48, 49};
    checkHits(query, Collection<int32_t>::newInstance(results, results + SIZEOF_ARRAY(results)));
}

TEST_F(BasicSpansTest, testSpanFirst) {
    SpanTermQueryPtr term1 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"five"));
    SpanFirstQueryPtr query = newLucene<SpanFirstQuery>(term1, 1);

    static const int32_t results[] = {
        5, 500, 501, 502, 503, 504, 505, 506, 507, 508, 509, 510, 511, 512, 513,
        514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 525, 526, 527,
        528, 529, 530, 531, 532, 533, 534, 535, 536, 537, 538, 539, 540, 541,
        542, 543, 544, 545, 546, 547, 548, 549, 550, 551, 552, 553, 554, 555,
        556, 557, 558, 559, 560, 561, 562, 563, 564, 565, 566, 567, 568, 569,
        570, 571, 572, 573, 574, 575, 576, 577, 578, 579, 580, 581, 582, 583,
        584, 585, 586, 587, 588, 589, 590, 591, 592, 593, 594, 595, 596, 597,
        598, 599
    };
    checkHits(query, Collection<int32_t>::newInstance(results, results + SIZEOF_ARRAY(results)));

    EXPECT_TRUE(searcher->explain(query, 5)->getValue() > 0.0);
    EXPECT_TRUE(searcher->explain(query, 599)->getValue() > 0.0);
}

TEST_F(BasicSpansTest, testSpanOr) {
    SpanTermQueryPtr term1 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"thirty"));
    SpanTermQueryPtr term2 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"three"));
    SpanNearQueryPtr near1 = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(term1, term2), 0, true);
    SpanTermQueryPtr term3 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"forty"));
    SpanTermQueryPtr term4 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"seven"));
    SpanNearQueryPtr near2 = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(term3, term4), 0, true);

    SpanOrQueryPtr query = newLucene<SpanOrQuery>(newCollection<SpanQueryPtr>(near1, near2));

    static const int32_t results[] = {
        33, 47, 133, 147, 233, 247, 333, 347, 433, 447, 533, 547, 633, 647, 733, 747, 833, 847, 933, 947
    };
    checkHits(query, Collection<int32_t>::newInstance(results, results + SIZEOF_ARRAY(results)));

    EXPECT_TRUE(searcher->explain(query, 33)->getValue() > 0.0);
    EXPECT_TRUE(searcher->explain(query, 947)->getValue() > 0.0);
}

TEST_F(BasicSpansTest, testSpanExactNested) {
    SpanTermQueryPtr term1 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"three"));
    SpanTermQueryPtr term2 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"hundred"));
    SpanNearQueryPtr near1 = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(term1, term2), 0, true);
    SpanTermQueryPtr term3 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"thirty"));
    SpanTermQueryPtr term4 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"three"));
    SpanNearQueryPtr near2 = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(term3, term4), 0, true);

    SpanNearQueryPtr query = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(near1, near2), 0, true);

    static const int32_t results[] = {333};
    checkHits(query, Collection<int32_t>::newInstance(results, results + SIZEOF_ARRAY(results)));

    EXPECT_TRUE(searcher->explain(query, 333)->getValue() > 0.0);
}

TEST_F(BasicSpansTest, testSpanNearOr) {
    SpanTermQueryPtr term1 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"six"));
    SpanTermQueryPtr term3 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"seven"));

    SpanTermQueryPtr term5 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"seven"));
    SpanTermQueryPtr term6 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"six"));

    SpanOrQueryPtr to1 = newLucene<SpanOrQuery>(newCollection<SpanQueryPtr>(term1, term3));
    SpanOrQueryPtr to2 = newLucene<SpanOrQuery>(newCollection<SpanQueryPtr>(term5, term6));

    SpanNearQueryPtr query = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(to1, to2), 10, true);

    static const int32_t results[] = {
        606, 607, 626, 627, 636, 637, 646, 647, 656,
        657, 666, 667, 676, 677, 686, 687, 696, 697,
        706, 707, 726, 727, 736, 737, 746, 747, 756,
        757, 766, 767, 776, 777, 786, 787, 796, 797
    };
    checkHits(query, Collection<int32_t>::newInstance(results, results + SIZEOF_ARRAY(results)));
}

TEST_F(BasicSpansTest, testSpanComplex1) {
    SpanTermQueryPtr term1 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"six"));
    SpanTermQueryPtr term2 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"hundred"));
    SpanNearQueryPtr near1 = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(term1, term2), 0, true);

    SpanTermQueryPtr term3 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"seven"));
    SpanTermQueryPtr term4 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"hundred"));
    SpanNearQueryPtr near2 = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(term3, term4), 0, true);

    SpanTermQueryPtr term5 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"seven"));
    SpanTermQueryPtr term6 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"six"));

    SpanOrQueryPtr to1 = newLucene<SpanOrQuery>(newCollection<SpanQueryPtr>(near1, near2));
    SpanOrQueryPtr to2 = newLucene<SpanOrQuery>(newCollection<SpanQueryPtr>(term5, term6));

    SpanNearQueryPtr query = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(to1, to2), 100, true);

    static const int32_t results[] = {
        606, 607, 626, 627, 636, 637, 646, 647, 656,
        657, 666, 667, 676, 677, 686, 687, 696, 697,
        706, 707, 726, 727, 736, 737, 746, 747, 756,
        757, 766, 767, 776, 777, 786, 787, 796, 797
    };
    checkHits(query, Collection<int32_t>::newInstance(results, results + SIZEOF_ARRAY(results)));
}

TEST_F(BasicSpansTest, testSpansSkipTo) {
    SpanTermQueryPtr term1 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"seventy"));
    SpanTermQueryPtr term2 = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"seventy"));
    SpansPtr spans1 = term1->getSpans(searcher->getIndexReader());
    SpansPtr spans2 = term2->getSpans(searcher->getIndexReader());

    EXPECT_TRUE(spans1->next());
    EXPECT_TRUE(spans2->next());

    bool hasMore = true;
    do {
        hasMore = skipTo(spans1, spans2->doc());
        EXPECT_EQ(hasMore, spans2->skipTo(spans2->doc()));
        EXPECT_EQ(spans1->doc(), spans2->doc());
    } while (hasMore);
}
