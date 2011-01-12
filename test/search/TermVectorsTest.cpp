/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
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

class TermVectorsFixture : public LuceneTestFixture
{
public:
    TermVectorsFixture()
    {
        directory = newLucene<MockRAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        for (int32_t i = 0; i < 1000; ++i)
        {
            DocumentPtr doc = newLucene<Document>();
            Field::TermVector termVector;
            int32_t mod3 = i % 3;
            int32_t mod2 = i % 2;
            if (mod2 == 0 && mod3 == 0)
                termVector = Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS;
            else if (mod2 == 0)
                termVector = Field::TERM_VECTOR_WITH_POSITIONS;
            else if (mod3 == 0)
                termVector = Field::TERM_VECTOR_WITH_OFFSETS;
            else
                termVector = Field::TERM_VECTOR_YES;
            doc->add(newLucene<Field>(L"field", intToEnglish(i), Field::STORE_YES, Field::INDEX_ANALYZED, termVector));
            writer->addDocument(doc);
        }
        writer->close();
        searcher = newLucene<IndexSearcher>(directory, true);
    }
    
    virtual ~TermVectorsFixture()
    {
    }

protected:
    IndexSearcherPtr searcher;
    DirectoryPtr directory;

public:
    void setupDoc(DocumentPtr doc, const String& text)
    {
        doc->add(newLucene<Field>(L"field2", text, Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
        doc->add(newLucene<Field>(L"field", text, Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_YES));
    }
};

BOOST_FIXTURE_TEST_SUITE(TermVectorsTest, TermVectorsFixture)

BOOST_AUTO_TEST_CASE(testTermVectors)
{
    QueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"field", L"seventy"));
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(100, hits.size());
    for (int32_t i = 0; i < hits.size(); ++i)
    {
        Collection<TermFreqVectorPtr> vector = searcher->reader->getTermFreqVectors(hits[i]->doc);
        BOOST_CHECK(vector);
        BOOST_CHECK_EQUAL(vector.size(), 1);
    }
}

BOOST_AUTO_TEST_CASE(testTermVectorsFieldOrder)
{
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
    BOOST_CHECK_EQUAL(4, v.size());
    Collection<String> expectedFields = newCollection<String>(L"a", L"b", L"c", L"x");
    Collection<int32_t> expectedPositions = newCollection<int32_t>(1, 2, 0);
    for (int32_t i = 0; i < v.size(); ++i)
    {
        TermPositionVectorPtr posVec = boost::dynamic_pointer_cast<TermPositionVector>(v[i]);
        BOOST_CHECK_EQUAL(expectedFields[i], posVec->getField());
        Collection<String> terms = posVec->getTerms();
        BOOST_CHECK_EQUAL(3, terms.size());
        BOOST_CHECK_EQUAL(L"content", terms[0]);
        BOOST_CHECK_EQUAL(L"here", terms[1]);
        BOOST_CHECK_EQUAL(L"some", terms[2]);
        for (int32_t j = 0; j < 3; ++j)
        {
            Collection<int32_t> positions = posVec->getTermPositions(j);
            BOOST_CHECK_EQUAL(1, positions.size());
            BOOST_CHECK_EQUAL(expectedPositions[j], positions[0]);
        }
    }
}

BOOST_AUTO_TEST_CASE(testTermPositionVectors)
{
    QueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"field", L"zero"));
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(1, hits.size());
    
    for (int32_t i = 0; i < hits.size(); ++i)
    {
        Collection<TermFreqVectorPtr> vector = searcher->reader->getTermFreqVectors(hits[i]->doc);
        BOOST_CHECK(vector);
        BOOST_CHECK_EQUAL(vector.size(), 1);

        bool shouldBePosVector = (hits[i]->doc % 2 == 0);
        BOOST_CHECK(!shouldBePosVector || (shouldBePosVector && boost::dynamic_pointer_cast<TermPositionVector>(vector[0])));

        bool shouldBeOffVector = (hits[i]->doc % 3 == 0);
        BOOST_CHECK(!shouldBeOffVector || (shouldBeOffVector && boost::dynamic_pointer_cast<TermPositionVector>(vector[0])));
        
        if (shouldBePosVector || shouldBeOffVector)
        {
            TermPositionVectorPtr posVec = boost::dynamic_pointer_cast<TermPositionVector>(vector[0]);
            Collection<String> terms = posVec->getTerms();
            BOOST_CHECK(terms && !terms.empty());
            
            for (int32_t j = 0; j < terms.size(); ++j)
            {
                Collection<int32_t> positions = posVec->getTermPositions(j);
                Collection<TermVectorOffsetInfoPtr> offsets = posVec->getOffsets(j);
                
                if (shouldBePosVector)
                {
                    BOOST_CHECK(positions);
                    BOOST_CHECK(!positions.empty());
                }
                else
                    BOOST_CHECK(!positions);
                
                if (shouldBeOffVector)
                {
                    BOOST_CHECK(offsets);
                    BOOST_CHECK(!offsets.empty());
                }
                else
                    BOOST_CHECK(!offsets);
            }
        }
        else
        {
            BOOST_CHECK(!boost::dynamic_pointer_cast<TermPositionVector>(vector[0]));
            TermFreqVectorPtr freqVec = boost::dynamic_pointer_cast<TermFreqVector>(vector[0]);
            Collection<String> terms = freqVec->getTerms();
            BOOST_CHECK(terms && !terms.empty());
        }
    }
}

BOOST_AUTO_TEST_CASE(testTermOffsetVectors)
{
    QueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"field", L"fifty"));
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(100, hits.size());
    
    for (int32_t i = 0; i < hits.size(); ++i)
    {
        Collection<TermFreqVectorPtr> vector = searcher->reader->getTermFreqVectors(hits[i]->doc);
        BOOST_CHECK(vector);
        BOOST_CHECK_EQUAL(vector.size(), 1);
    }
}

BOOST_AUTO_TEST_CASE(testKnownSetOfDocuments)
{
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
    BOOST_CHECK(writer);
    writer->addDocument(testDoc1);
    writer->addDocument(testDoc2);
    writer->addDocument(testDoc3);
    writer->addDocument(testDoc4);
    writer->close();
    
    IndexSearcherPtr knownSearcher = newLucene<IndexSearcher>(dir, true);
    TermEnumPtr termEnum = knownSearcher->reader->terms();
    TermDocsPtr termDocs = knownSearcher->reader->termDocs();

    SimilarityPtr sim = knownSearcher->getSimilarity();
    while (termEnum->next())
    {
        TermPtr term = termEnum->term();
        termDocs->seek(term);
        while (termDocs->next())
        {
            int32_t docId = termDocs->doc();
            int32_t freq = termDocs->freq();
            TermFreqVectorPtr vector = knownSearcher->reader->getTermFreqVector(docId, L"field");
            double tf = sim->tf(freq);
            double idf = sim->idf(knownSearcher->docFreq(term), knownSearcher->maxDoc());
            // This is fine since we don't have stop words
            double lNorm = sim->lengthNorm(L"field", vector->getTerms().size());
            BOOST_CHECK(vector);
            Collection<String> vTerms = vector->getTerms();
            Collection<int32_t> freqs = vector->getTermFrequencies();
            for (int32_t i = 0; i < vTerms.size(); ++i)
            {
                if (term->text() == vTerms[i])
                    BOOST_CHECK_EQUAL(freqs[i], freq);
            }
        }
    }
    QueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"field", L"chocolate"));
    Collection<ScoreDocPtr> hits = knownSearcher->search(query, FilterPtr(), 1000)->scoreDocs;
    // doc 3 should be the first hit because it is the shortest match
    BOOST_CHECK_EQUAL(hits.size(), 3);
    double score = hits[0]->score;
    BOOST_CHECK_EQUAL(hits[0]->doc, 2);
    BOOST_CHECK_EQUAL(hits[1]->doc, 3);
    BOOST_CHECK_EQUAL(hits[2]->doc, 0);
    TermFreqVectorPtr vector = knownSearcher->reader->getTermFreqVector(hits[1]->doc, L"field");
    BOOST_CHECK(vector);
    Collection<String> terms = vector->getTerms();
    Collection<int32_t> freqs = vector->getTermFrequencies();
    BOOST_CHECK(terms && terms.size() == 10);
    for (int32_t i = 0; i < terms.size(); ++i)
    {
        String term = terms[i];
        int32_t freq = freqs[i];
        BOOST_CHECK(test4.find(term) != String::npos);
        BOOST_CHECK(test4Map.contains(term));
        BOOST_CHECK_EQUAL(test4Map[term], freq);
    }
    SortedTermVectorMapperPtr mapper = newLucene<SortedTermVectorMapper>(TermVectorEntryFreqSortedComparator::compare);
    knownSearcher->reader->getTermFreqVector(hits[1]->doc, mapper);
    Collection<TermVectorEntryPtr> vectorEntrySet = mapper->getTermVectorEntrySet();
    BOOST_CHECK_EQUAL(vectorEntrySet.size(), 10);
    TermVectorEntryPtr last;
    for (Collection<TermVectorEntryPtr>::iterator tve = vectorEntrySet.begin(); tve != vectorEntrySet.end(); ++tve)
    {
        if (*tve && last)
        {
            BOOST_CHECK(last->getFrequency() >= (*tve)->getFrequency());
            int32_t expectedFreq = test4Map.get((*tve)->getTerm());
            // we expect double the expectedFreq, since there are two fields with the exact same text and we are collapsing all fields
            BOOST_CHECK_EQUAL((*tve)->getFrequency(), 2 * expectedFreq);
        }
        last = *tve;
    }
    FieldSortedTermVectorMapperPtr fieldMapper = newLucene<FieldSortedTermVectorMapper>(TermVectorEntryFreqSortedComparator::compare);
    knownSearcher->reader->getTermFreqVector(hits[1]->doc, fieldMapper);
    MapStringCollectionTermVectorEntry map = fieldMapper->getFieldToTerms();
    BOOST_CHECK_EQUAL(map.size(), 2);
    vectorEntrySet = map.get(L"field");
    BOOST_CHECK(vectorEntrySet);
    BOOST_CHECK_EQUAL(vectorEntrySet.size(), 10);
    knownSearcher->close();
}

/// Test only a few docs having vectors
BOOST_AUTO_TEST_CASE(testRareVectors)
{
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    
    for (int32_t i = 0; i < 100; ++i)
    {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"field", intToEnglish(i), Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_NO));
        writer->addDocument(doc);
    }

    for (int32_t i = 0; i < 10; ++i)
    {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"field", intToEnglish(100 + i), Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
        writer->addDocument(doc);
    }
    
    writer->close();
    searcher = newLucene<IndexSearcher>(directory, true);

    QueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"field", L"hundred"));
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(10, hits.size());
    for (int32_t i = 0; i < hits.size(); ++i)
    {
        Collection<TermFreqVectorPtr> vector = searcher->reader->getTermFreqVectors(hits[i]->doc);
        BOOST_CHECK(vector);
        BOOST_CHECK_EQUAL(vector.size(), 1);
    }
}

/// In a single doc, for the same field, mix the term vectors up
BOOST_AUTO_TEST_CASE(testMixedVectrosVectors)
{
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
    BOOST_CHECK_EQUAL(1, hits.size());

    Collection<TermFreqVectorPtr> vector = searcher->reader->getTermFreqVectors(hits[0]->doc);
    BOOST_CHECK(vector);
    BOOST_CHECK_EQUAL(vector.size(), 1);
    TermPositionVectorPtr tfv = boost::dynamic_pointer_cast<TermPositionVector>(vector[0]);
    BOOST_CHECK_EQUAL(tfv->getField(), L"field");
    Collection<String> terms = tfv->getTerms();
    BOOST_CHECK_EQUAL(1, terms.size());
    BOOST_CHECK_EQUAL(terms[0], L"one");
    BOOST_CHECK_EQUAL(5, tfv->getTermFrequencies()[0]);

    Collection<int32_t> positions = tfv->getTermPositions(0);
    BOOST_CHECK_EQUAL(5, positions.size());
    for (int32_t i = 0; i < 5; ++i)
        BOOST_CHECK_EQUAL(i, positions[i]);
    Collection<TermVectorOffsetInfoPtr> offsets = tfv->getOffsets(0);
    BOOST_CHECK_EQUAL(5, offsets.size());
    for (int32_t i = 0; i < 5; ++i)
    {
        BOOST_CHECK_EQUAL(4 * i, offsets[i]->getStartOffset());
        BOOST_CHECK_EQUAL(4 * i + 3, offsets[i]->getEndOffset());
    }
}

BOOST_AUTO_TEST_SUITE_END()
