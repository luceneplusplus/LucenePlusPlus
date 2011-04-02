/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <boost/algorithm/string.hpp>
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "MockRAMDirectory.h"
#include "StandardAnalyzer.h"
#include "IndexWriter.h"
#include "Document.h"
#include "Field.h"
#include "SegmentReader.h"
#include "FieldInfos.h"
#include "FieldInfo.h"
#include "Similarity.h"
#include "Explanation.h"
#include "IndexSearcher.h"
#include "TermQuery.h"
#include "Term.h"
#include "Collector.h"
#include "Scorer.h"
#include "BooleanQuery.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(OmitTfTest, LuceneTestFixture)

DECLARE_SHARED_PTR(CountingHitCollector)

class SimpleIDFExplanation : public IDFExplanation
{
public:
    virtual ~SimpleIDFExplanation()
    {
    }

    LUCENE_CLASS(SimpleIDFExplanation);

public:
    virtual double getIdf()
    {
        return 1.0;
    }
    
    virtual String explain()
    {
        return L"Inexplicable";
    }
};

class SimpleSimilarity : public Similarity
{
public:
    virtual ~SimpleSimilarity()
    {
    }
    
    LUCENE_CLASS(SimpleSimilarity);

public:
    virtual double lengthNorm(const String& fieldName, int32_t numTokens)
    {
        return 1.0;
    }
    
    virtual double queryNorm(double sumOfSquaredWeights)
    {
        return 1.0;
    }
    
    virtual double tf(double freq)
    {
        return freq;
    }
    
    virtual double sloppyFreq(int32_t distance)
    {
        return 2.0;
    }
    
    virtual double idf(int32_t docFreq, int32_t numDocs)
    {
        return 1.0;
    }
    
    virtual double coord(int32_t overlap, int32_t maxOverlap)
    {
        return 1.0;
    }
    
    virtual IDFExplanationPtr idfExplain(Collection<TermPtr> terms, SearcherPtr searcher)
    {
        return newLucene<SimpleIDFExplanation>();
    }
};

class CountingHitCollector : public Collector
{
public:
    CountingHitCollector()
    {
        count = 0;
        sum = 0;
        docBase = -1;
    }
    
    virtual ~CountingHitCollector()
    {
    }
    
    LUCENE_CLASS(CountingHitCollector);
    
public:
    int32_t count;
    int32_t sum;

protected:
    int32_t docBase;

public:
    virtual void setScorer(ScorerPtr scorer)
    {
    }
    
    virtual void collect(int32_t doc)
    {
        ++count;
        sum += doc + docBase; // use it to avoid any possibility of being optimized away
    }
    
    virtual void setNextReader(IndexReaderPtr reader, int32_t docBase)
    {
        this->docBase = docBase;
    }
    
    virtual bool acceptsDocsOutOfOrder()
    {
        return true;
    }
};

static void checkNoPrx(DirectoryPtr dir)
{
    HashSet<String> files = dir->listAll();
    for (HashSet<String>::iterator file = files.begin(); file != files.end(); ++file)
        BOOST_CHECK(!boost::ends_with(*file, L".prx"));
}

/// Tests whether the DocumentWriter correctly enable the omitTermFreqAndPositions bit in the FieldInfo
BOOST_AUTO_TEST_CASE(testOmitTermFreqAndPositions)
{
    DirectoryPtr ram = newLucene<MockRAMDirectory>();
    AnalyzerPtr analyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    IndexWriterPtr writer = newLucene<IndexWriter>(ram, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr d = newLucene<Document>();

    // this field will have Tf
    FieldPtr f1 = newLucene<Field>(L"f1", L"This field has term freqs", Field::STORE_NO, Field::INDEX_ANALYZED);
    d->add(f1);

    // this field will NOT have Tf
    FieldPtr f2 = newLucene<Field>(L"f2", L"This field has NO Tf in all docs", Field::STORE_NO, Field::INDEX_ANALYZED);
    f2->setOmitTermFreqAndPositions(true);
    d->add(f2);

    writer->addDocument(d);
    writer->optimize();
    // now we add another document which has term freq for field f2 and not for f1 and verify if the SegmentMerger keep things constant
    d = newLucene<Document>();

    // Reverese
    f1->setOmitTermFreqAndPositions(true);
    d->add(f1);

    f2->setOmitTermFreqAndPositions(false);        
    d->add(f2);

    writer->addDocument(d);
    // force merge
    writer->optimize();
    // flush
    writer->close();
    checkIndex(ram);

    SegmentReaderPtr reader = SegmentReader::getOnlySegmentReader(ram);
    FieldInfosPtr fi = reader->fieldInfos();
    BOOST_CHECK(fi->fieldInfo(L"f1")->omitTermFreqAndPositions);
    BOOST_CHECK(fi->fieldInfo(L"f2")->omitTermFreqAndPositions);

    reader->close();
    ram->close();
}

/// Tests whether merging of docs that have different omitTermFreqAndPositions for the same field works
BOOST_AUTO_TEST_CASE(testMixedMerge)
{
    DirectoryPtr ram = newLucene<MockRAMDirectory>();
    AnalyzerPtr analyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    IndexWriterPtr writer = newLucene<IndexWriter>(ram, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(3);
    writer->setMergeFactor(2);
    DocumentPtr d = newLucene<Document>();

    // this field will have Tf
    FieldPtr f1 = newLucene<Field>(L"f1", L"This field has term freqs", Field::STORE_NO, Field::INDEX_ANALYZED);
    d->add(f1);

    // this field will NOT have Tf
    FieldPtr f2 = newLucene<Field>(L"f2", L"This field has NO Tf in all docs", Field::STORE_NO, Field::INDEX_ANALYZED);
    f2->setOmitTermFreqAndPositions(true);
    d->add(f2);

    for (int32_t i = 0; i < 30; ++i)
        writer->addDocument(d);
    
    // now we add another document which has term freq for field f2 and not for f1 and verify if the SegmentMerger keep things constant
    d = newLucene<Document>();

    // Reverese
    f1->setOmitTermFreqAndPositions(true);
    d->add(f1);

    f2->setOmitTermFreqAndPositions(false);        
    d->add(f2);
    
    for (int32_t i = 0; i < 30; ++i)
        writer->addDocument(d);

    // force merge
    writer->optimize();
    // flush
    writer->close();

    checkIndex(ram);

    SegmentReaderPtr reader = SegmentReader::getOnlySegmentReader(ram);
    FieldInfosPtr fi = reader->fieldInfos();
    BOOST_CHECK(fi->fieldInfo(L"f1")->omitTermFreqAndPositions);
    BOOST_CHECK(fi->fieldInfo(L"f2")->omitTermFreqAndPositions);
    
    reader->close();
    ram->close();
}

/// Make sure first adding docs that do not omitTermFreqAndPositions for field X, then adding docs that do 
/// omitTermFreqAndPositions for that same field
BOOST_AUTO_TEST_CASE(testMixedRAM)
{
    DirectoryPtr ram = newLucene<MockRAMDirectory>();
    AnalyzerPtr analyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    IndexWriterPtr writer = newLucene<IndexWriter>(ram, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(10);
    writer->setMergeFactor(2);
    DocumentPtr d = newLucene<Document>();
    
    // this field will have Tf
    FieldPtr f1 = newLucene<Field>(L"f1", L"This field has term freqs", Field::STORE_NO, Field::INDEX_ANALYZED);
    d->add(f1);

    // this field will NOT have Tf
    FieldPtr f2 = newLucene<Field>(L"f2", L"This field has NO Tf in all docs", Field::STORE_NO, Field::INDEX_ANALYZED);
    d->add(f2);
    
    for (int32_t i = 0; i < 5; ++i)
        writer->addDocument(d);
    
    f2->setOmitTermFreqAndPositions(true);

    for (int32_t i = 0; i < 20; ++i)
        writer->addDocument(d);

    // force merge
    writer->optimize();

    // flush
    writer->close();

    checkIndex(ram);

    SegmentReaderPtr reader = SegmentReader::getOnlySegmentReader(ram);
    FieldInfosPtr fi = reader->fieldInfos();
    BOOST_CHECK(!fi->fieldInfo(L"f1")->omitTermFreqAndPositions);
    BOOST_CHECK(fi->fieldInfo(L"f2")->omitTermFreqAndPositions);

    reader->close();
    ram->close();
}

/// Verifies no *.prx exists when all fields omit term freq
BOOST_AUTO_TEST_CASE(testNoPrxFile)
{
    DirectoryPtr ram = newLucene<MockRAMDirectory>();
    AnalyzerPtr analyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    IndexWriterPtr writer = newLucene<IndexWriter>(ram, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(3);
    writer->setMergeFactor(2);
    writer->setUseCompoundFile(false);
    DocumentPtr d = newLucene<Document>();
    
    // this field will have Tf
    FieldPtr f1 = newLucene<Field>(L"f1", L"This field has term freqs", Field::STORE_NO, Field::INDEX_ANALYZED);
    f1->setOmitTermFreqAndPositions(true);
    d->add(f1);
    
    for (int32_t i = 0; i < 30; ++i)
        writer->addDocument(d);

    writer->commit();

    checkNoPrx(ram);

    // force merge
    writer->optimize();
    // flush
    writer->close();

    checkNoPrx(ram);
    checkIndex(ram);
    ram->close();
}

namespace TestBasic
{
    class CountingHitCollectorQ1 : public CountingHitCollector
    {
    protected:
        ScorerPtr scorer;
    
    public:
        virtual void setScorer(ScorerPtr scorer)
        {
            this->scorer = scorer;
        }
        
        virtual void collect(int32_t doc)
        {
            BOOST_CHECK_EQUAL(scorer->score(), 1.0);
            CountingHitCollector::collect(doc);
        }
    };
    
    class CountingHitCollectorQ2 : public CountingHitCollector
    {
    protected:
        ScorerPtr scorer;
    
    public:
        virtual void setScorer(ScorerPtr scorer)
        {
            this->scorer = scorer;
        }
        
        virtual void collect(int32_t doc)
        {
            BOOST_CHECK_EQUAL(scorer->score(), 1.0 + (double)doc);
            CountingHitCollector::collect(doc);
        }
    };
    
    class CountingHitCollectorQ3 : public CountingHitCollector
    {
    protected:
        ScorerPtr scorer;
    
    public:
        virtual void setScorer(ScorerPtr scorer)
        {
            this->scorer = scorer;
        }
        
        virtual void collect(int32_t doc)
        {
            BOOST_CHECK_EQUAL(scorer->score(), 1.0);
            BOOST_CHECK_NE(doc % 2, 0);
            CountingHitCollector::collect(doc);
        }
    };
    
    class CountingHitCollectorQ4 : public CountingHitCollector
    {
    protected:
        ScorerPtr scorer;
    
    public:
        virtual void setScorer(ScorerPtr scorer)
        {
            this->scorer = scorer;
        }
        
        virtual void collect(int32_t doc)
        {
            BOOST_CHECK_EQUAL(scorer->score(), 1.0);
            BOOST_CHECK_EQUAL(doc % 2, 0);
            CountingHitCollector::collect(doc);
        }
    };
}

BOOST_AUTO_TEST_CASE(testBasic)
{
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    AnalyzerPtr analyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);

    writer->setMergeFactor(2);
    writer->setMaxBufferedDocs(2);
    writer->setSimilarity(newLucene<SimpleSimilarity>());
    
    StringStream sb;
    for (int32_t i = 0; i < 30; ++i)
    {
        DocumentPtr d = newLucene<Document>();
        sb << L"term ";
        String content  = sb.str();
        FieldPtr noTf = newLucene<Field>(L"noTf", content + (i % 2 == 0 ? L"" : L" notf"), Field::STORE_NO, Field::INDEX_ANALYZED);
        noTf->setOmitTermFreqAndPositions(true);
        d->add(noTf);

        FieldPtr tf = newLucene<Field>(L"tf", content + (i % 2 == 0 ? L" tf" : L""), Field::STORE_NO, Field::INDEX_ANALYZED);
        d->add(tf);

        writer->addDocument(d);
    }
    
    writer->optimize();
    // flush
    writer->close();
    checkIndex(dir);

    // Verify the index
    SearcherPtr searcher = newLucene<IndexSearcher>(dir, true);
    searcher->setSimilarity(newLucene<SimpleSimilarity>());

    TermPtr a = newLucene<Term>(L"noTf", L"term");
    TermPtr b = newLucene<Term>(L"tf", L"term");
    TermPtr c = newLucene<Term>(L"noTf", L"noTf");
    TermPtr d = newLucene<Term>(L"tf", L"tf");
    TermQueryPtr q1 = newLucene<TermQuery>(a);
    TermQueryPtr q2 = newLucene<TermQuery>(b);
    TermQueryPtr q3 = newLucene<TermQuery>(c);
    TermQueryPtr q4 = newLucene<TermQuery>(d);

    searcher->search(q1, newLucene<TestBasic::CountingHitCollectorQ1>());
    
    searcher->search(q2, newLucene<TestBasic::CountingHitCollectorQ2>());
    
    searcher->search(q3, newLucene<TestBasic::CountingHitCollectorQ3>());
    
    searcher->search(q4, newLucene<TestBasic::CountingHitCollectorQ4>());

    BooleanQueryPtr bq = newLucene<BooleanQuery>();
    bq->add(q1, BooleanClause::MUST);
    bq->add(q4, BooleanClause::MUST);
    
    CountingHitCollectorPtr collector = newLucene<CountingHitCollector>();
    
    searcher->search(bq, collector);
    BOOST_CHECK_EQUAL(15, collector->count);
        
    searcher->close();
    dir->close();
}

BOOST_AUTO_TEST_SUITE_END()
