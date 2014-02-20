/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "MockRAMDirectory.h"
#include "IndexWriter.h"
#include "SimpleAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexSearcher.h"
#include "TermQuery.h"
#include "Term.h"
#include "TermFreqVector.h"
#include "TopDocs.h"
#include "ScoreDoc.h"
#include "IndexReader.h"
#include "TermPositionVector.h"
#include "SortedTermVectorMapper.h"
#include "TermVectorEntryFreqSortedComparator.h"
#include "FieldSortedTermVectorMapper.h"
#include "TermEnum.h"
#include "TermDocs.h"
#include "Similarity.h"
#include "TermVectorEntry.h"
#include "TermVectorOffsetInfo.h"

using namespace Lucene;

class TermVectorsTest : public LuceneTestFixture {
public:
    TermVectorsTest() {
        directory = newLucene<MockRAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        for (int32_t i = 0; i < 1000; ++i) {
            DocumentPtr doc = newLucene<Document>();
            Field::TermVector termVector;
            int32_t mod3 = i % 3;
            int32_t mod2 = i % 2;
            if (mod2 == 0 && mod3 == 0) {
                termVector = Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS;
            } else if (mod2 == 0) {
                termVector = Field::TERM_VECTOR_WITH_POSITIONS;
            } else if (mod3 == 0) {
                termVector = Field::TERM_VECTOR_WITH_OFFSETS;
            } else {
                termVector = Field::TERM_VECTOR_YES;
            }
            doc->add(newLucene<Field>(L"field", intToEnglish(i), Field::STORE_YES, Field::INDEX_ANALYZED, termVector));
            writer->addDocument(doc);
        }
        writer->close();
        searcher = newLucene<IndexSearcher>(directory, true);
    }

    virtual ~TermVectorsTest() {
    }

protected:
    IndexSearcherPtr searcher;
    DirectoryPtr directory;

public:
    void setupDoc(const DocumentPtr& doc, const String& text) {
        doc->add(newLucene<Field>(L"field2", text, Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
        doc->add(newLucene<Field>(L"field", text, Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_YES));
    }
};

TEST_F(TermVectorsTest, testTermVectors) {
    QueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"field", L"seventy"));
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(100, hits.size());
    for (int32_t i = 0; i < hits.size(); ++i) {
        Collection<TermFreqVectorPtr> vector = searcher->reader->getTermFreqVectors(hits[i]->doc);
        EXPECT_TRUE(vector);
        EXPECT_EQ(vector.size(), 1);
    }
}

TEST_F(TermVectorsTest, testTermVectorsFieldOrder) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"c", L"some content here", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    doc->add(newLucene<Field>(L"a", L"some content here", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    doc->add(newLucene<Field>(L"b", L"some content here", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    doc->add(newLucene<Field>(L"x", L"some content here", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    writer->addDocument(doc);
    writer->close();
    IndexReaderPtr reader = IndexReader::open(dir, true);
    Collection<TermFreqVectorPtr> v = reader->getTermFreqVectors(0);
    EXPECT_EQ(4, v.size());
    Collection<String> expectedFields = newCollection<String>(L"a", L"b", L"c", L"x");
    Collection<int32_t> expectedPositions = newCollection<int32_t>(1, 2, 0);
    for (int32_t i = 0; i < v.size(); ++i) {
        TermPositionVectorPtr posVec = boost::dynamic_pointer_cast<TermPositionVector>(v[i]);
        EXPECT_EQ(expectedFields[i], posVec->getField());
        Collection<String> terms = posVec->getTerms();
        EXPECT_EQ(3, terms.size());
        EXPECT_EQ(L"content", terms[0]);
        EXPECT_EQ(L"here", terms[1]);
        EXPECT_EQ(L"some", terms[2]);
        for (int32_t j = 0; j < 3; ++j) {
            Collection<int32_t> positions = posVec->getTermPositions(j);
            EXPECT_EQ(1, positions.size());
            EXPECT_EQ(expectedPositions[j], positions[0]);
        }
    }
}

TEST_F(TermVectorsTest, testTermPositionVectors) {
    QueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"field", L"zero"));
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());

    for (int32_t i = 0; i < hits.size(); ++i) {
        Collection<TermFreqVectorPtr> vector = searcher->reader->getTermFreqVectors(hits[i]->doc);
        EXPECT_TRUE(vector);
        EXPECT_EQ(vector.size(), 1);

        bool shouldBePosVector = (hits[i]->doc % 2 == 0);
        EXPECT_TRUE(!shouldBePosVector || (shouldBePosVector && boost::dynamic_pointer_cast<TermPositionVector>(vector[0])));

        bool shouldBeOffVector = (hits[i]->doc % 3 == 0);
        EXPECT_TRUE(!shouldBeOffVector || (shouldBeOffVector && boost::dynamic_pointer_cast<TermPositionVector>(vector[0])));

        if (shouldBePosVector || shouldBeOffVector) {
            TermPositionVectorPtr posVec = boost::dynamic_pointer_cast<TermPositionVector>(vector[0]);
            Collection<String> terms = posVec->getTerms();
            EXPECT_TRUE(terms && !terms.empty());

            for (int32_t j = 0; j < terms.size(); ++j) {
                Collection<int32_t> positions = posVec->getTermPositions(j);
                Collection<TermVectorOffsetInfoPtr> offsets = posVec->getOffsets(j);

                if (shouldBePosVector) {
                    EXPECT_TRUE(positions);
                    EXPECT_TRUE(!positions.empty());
                } else {
                    EXPECT_TRUE(!positions);
                }

                if (shouldBeOffVector) {
                    EXPECT_TRUE(offsets);
                    EXPECT_TRUE(!offsets.empty());
                } else {
                    EXPECT_TRUE(!offsets);
                }
            }
        } else {
            EXPECT_TRUE(!boost::dynamic_pointer_cast<TermPositionVector>(vector[0]));
            TermFreqVectorPtr freqVec = boost::dynamic_pointer_cast<TermFreqVector>(vector[0]);
            Collection<String> terms = freqVec->getTerms();
            EXPECT_TRUE(terms && !terms.empty());
        }
    }
}

TEST_F(TermVectorsTest, testTermOffsetVectors) {
    QueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"field", L"fifty"));
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(100, hits.size());

    for (int32_t i = 0; i < hits.size(); ++i) {
        Collection<TermFreqVectorPtr> vector = searcher->reader->getTermFreqVectors(hits[i]->doc);
        EXPECT_TRUE(vector);
        EXPECT_EQ(vector.size(), 1);
    }
}

TEST_F(TermVectorsTest, testKnownSetOfDocuments) {
    String test1 = L"eating chocolate in a computer lab"; // 6 terms
    String test2 = L"computer in a computer lab"; // 5 terms
    String test3 = L"a chocolate lab grows old"; // 5 terms
    String test4 = L"eating chocolate with a chocolate lab in an old chocolate colored computer lab"; // 13 terms
    MapStringInt test4Map = MapStringInt::newInstance();
    test4Map.put(L"chocolate", 3);
    test4Map.put(L"lab", 2);
    test4Map.put(L"eating", 1);
    test4Map.put(L"computer", 1);
    test4Map.put(L"with", 1);
    test4Map.put(L"a", 1);
    test4Map.put(L"colored", 1);
    test4Map.put(L"in", 1);
    test4Map.put(L"an", 1);
    test4Map.put(L"computer", 1);
    test4Map.put(L"old", 1);

    DocumentPtr testDoc1 = newLucene<Document>();
    setupDoc(testDoc1, test1);
    DocumentPtr testDoc2 = newLucene<Document>();
    setupDoc(testDoc2, test2);
    DocumentPtr testDoc3 = newLucene<Document>();
    setupDoc(testDoc3, test3);
    DocumentPtr testDoc4 = newLucene<Document>();
    setupDoc(testDoc4, test4);

    DirectoryPtr dir = newLucene<MockRAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    EXPECT_TRUE(writer);
    writer->addDocument(testDoc1);
    writer->addDocument(testDoc2);
    writer->addDocument(testDoc3);
    writer->addDocument(testDoc4);
    writer->close();

    IndexSearcherPtr knownSearcher = newLucene<IndexSearcher>(dir, true);
    TermEnumPtr termEnum = knownSearcher->reader->terms();
    TermDocsPtr termDocs = knownSearcher->reader->termDocs();

    SimilarityPtr sim = knownSearcher->getSimilarity();
    while (termEnum->next()) {
        TermPtr term = termEnum->term();
        termDocs->seek(term);
        while (termDocs->next()) {
            int32_t docId = termDocs->doc();
            int32_t freq = termDocs->freq();
            TermFreqVectorPtr vector = knownSearcher->reader->getTermFreqVector(docId, L"field");
            double tf = sim->tf(freq);
            double idf = sim->idf(knownSearcher->docFreq(term), knownSearcher->maxDoc());
            // This is fine since we don't have stop words
            double lNorm = sim->lengthNorm(L"field", vector->getTerms().size());
            EXPECT_TRUE(vector);
            Collection<String> vTerms = vector->getTerms();
            Collection<int32_t> freqs = vector->getTermFrequencies();
            for (int32_t i = 0; i < vTerms.size(); ++i) {
                if (term->text() == vTerms[i]) {
                    EXPECT_EQ(freqs[i], freq);
                }
            }
        }
    }
    QueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"field", L"chocolate"));
    Collection<ScoreDocPtr> hits = knownSearcher->search(query, FilterPtr(), 1000)->scoreDocs;
    // doc 3 should be the first hit because it is the shortest match
    EXPECT_EQ(hits.size(), 3);
    double score = hits[0]->score;
    EXPECT_EQ(hits[0]->doc, 2);
    EXPECT_EQ(hits[1]->doc, 3);
    EXPECT_EQ(hits[2]->doc, 0);
    TermFreqVectorPtr vector = knownSearcher->reader->getTermFreqVector(hits[1]->doc, L"field");
    EXPECT_TRUE(vector);
    Collection<String> terms = vector->getTerms();
    Collection<int32_t> freqs = vector->getTermFrequencies();
    EXPECT_TRUE(terms && terms.size() == 10);
    for (int32_t i = 0; i < terms.size(); ++i) {
        String term = terms[i];
        int32_t freq = freqs[i];
        EXPECT_TRUE(test4.find(term) != String::npos);
        EXPECT_TRUE(test4Map.contains(term));
        EXPECT_EQ(test4Map[term], freq);
    }
    SortedTermVectorMapperPtr mapper = newLucene<SortedTermVectorMapper>(TermVectorEntryFreqSortedComparator::compare);
    knownSearcher->reader->getTermFreqVector(hits[1]->doc, mapper);
    Collection<TermVectorEntryPtr> vectorEntrySet = mapper->getTermVectorEntrySet();
    EXPECT_EQ(vectorEntrySet.size(), 10);
    TermVectorEntryPtr last;
    for (Collection<TermVectorEntryPtr>::iterator tve = vectorEntrySet.begin(); tve != vectorEntrySet.end(); ++tve) {
        if (*tve && last) {
            EXPECT_TRUE(last->getFrequency() >= (*tve)->getFrequency());
            int32_t expectedFreq = test4Map.get((*tve)->getTerm());
            // we expect double the expectedFreq, since there are two fields with the exact same text and we are collapsing all fields
            EXPECT_EQ((*tve)->getFrequency(), 2 * expectedFreq);
        }
        last = *tve;
    }
    FieldSortedTermVectorMapperPtr fieldMapper = newLucene<FieldSortedTermVectorMapper>(TermVectorEntryFreqSortedComparator::compare);
    knownSearcher->reader->getTermFreqVector(hits[1]->doc, fieldMapper);
    MapStringCollectionTermVectorEntry map = fieldMapper->getFieldToTerms();
    EXPECT_EQ(map.size(), 2);
    vectorEntrySet = map.get(L"field");
    EXPECT_TRUE(vectorEntrySet);
    EXPECT_EQ(vectorEntrySet.size(), 10);
    knownSearcher->close();
}

/// Test only a few docs having vectors
TEST_F(TermVectorsTest, testRareVectors) {
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

    for (int32_t i = 0; i < 100; ++i) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"field", intToEnglish(i), Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_NO));
        writer->addDocument(doc);
    }

    for (int32_t i = 0; i < 10; ++i) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"field", intToEnglish(100 + i), Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
        writer->addDocument(doc);
    }

    writer->close();
    searcher = newLucene<IndexSearcher>(directory, true);

    QueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"field", L"hundred"));
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(10, hits.size());
    for (int32_t i = 0; i < hits.size(); ++i) {
        Collection<TermFreqVectorPtr> vector = searcher->reader->getTermFreqVectors(hits[i]->doc);
        EXPECT_TRUE(vector);
        EXPECT_EQ(vector.size(), 1);
    }
}

/// In a single doc, for the same field, mix the term vectors up
TEST_F(TermVectorsTest, testMixedVectrosVectors) {
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", L"one", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_NO));
    doc->add(newLucene<Field>(L"field", L"one", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_YES));
    doc->add(newLucene<Field>(L"field", L"one", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS));
    doc->add(newLucene<Field>(L"field", L"one", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_OFFSETS));
    doc->add(newLucene<Field>(L"field", L"one", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    writer->addDocument(doc);
    writer->close();

    searcher = newLucene<IndexSearcher>(directory, true);

    QueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"field", L"one"));
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());

    Collection<TermFreqVectorPtr> vector = searcher->reader->getTermFreqVectors(hits[0]->doc);
    EXPECT_TRUE(vector);
    EXPECT_EQ(vector.size(), 1);
    TermPositionVectorPtr tfv = boost::dynamic_pointer_cast<TermPositionVector>(vector[0]);
    EXPECT_EQ(tfv->getField(), L"field");
    Collection<String> terms = tfv->getTerms();
    EXPECT_EQ(1, terms.size());
    EXPECT_EQ(terms[0], L"one");
    EXPECT_EQ(5, tfv->getTermFrequencies()[0]);

    Collection<int32_t> positions = tfv->getTermPositions(0);
    EXPECT_EQ(5, positions.size());
    for (int32_t i = 0; i < 5; ++i) {
        EXPECT_EQ(i, positions[i]);
    }
    Collection<TermVectorOffsetInfoPtr> offsets = tfv->getOffsets(0);
    EXPECT_EQ(5, offsets.size());
    for (int32_t i = 0; i < 5; ++i) {
        EXPECT_EQ(4 * i, offsets[i]->getStartOffset());
        EXPECT_EQ(4 * i + 3, offsets[i]->getEndOffset());
    }
}
