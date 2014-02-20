/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
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

typedef LuceneTestFixture OmitTfTest;

DECLARE_SHARED_PTR(CountingHitCollector)

class SimpleIDFExplanation : public IDFExplanation {
public:
    virtual ~SimpleIDFExplanation() {
    }

    LUCENE_CLASS(SimpleIDFExplanation);

public:
    virtual double getIdf() {
        return 1.0;
    }

    virtual String explain() {
        return L"Inexplicable";
    }
};

class SimpleSimilarity : public Similarity {
public:
    virtual ~SimpleSimilarity() {
    }

    LUCENE_CLASS(SimpleSimilarity);

public:
    virtual double lengthNorm(const String& fieldName, int32_t numTokens) {
        return 1.0;
    }

    virtual double queryNorm(double sumOfSquaredWeights) {
        return 1.0;
    }

    virtual double tf(double freq) {
        return freq;
    }

    virtual double sloppyFreq(int32_t distance) {
        return 2.0;
    }

    virtual double idf(int32_t docFreq, int32_t numDocs) {
        return 1.0;
    }

    virtual double coord(int32_t overlap, int32_t maxOverlap) {
        return 1.0;
    }

    virtual IDFExplanationPtr idfExplain(Collection<TermPtr> terms, const SearcherPtr& searcher) {
        return newLucene<SimpleIDFExplanation>();
    }
};

class CountingHitCollector : public Collector {
public:
    CountingHitCollector() {
        count = 0;
        sum = 0;
        docBase = -1;
    }

    virtual ~CountingHitCollector() {
    }

    LUCENE_CLASS(CountingHitCollector);

public:
    int32_t count;
    int32_t sum;

protected:
    int32_t docBase;

public:
    virtual void setScorer(const ScorerPtr& scorer) {
    }

    virtual void collect(int32_t doc) {
        ++count;
        sum += doc + docBase; // use it to avoid any possibility of being optimized away
    }

    virtual void setNextReader(const IndexReaderPtr& reader, int32_t docBase) {
        this->docBase = docBase;
    }

    virtual bool acceptsDocsOutOfOrder() {
        return true;
    }
};

static void checkNoPrx(const DirectoryPtr& dir) {
    HashSet<String> files = dir->listAll();
    for (HashSet<String>::iterator file = files.begin(); file != files.end(); ++file) {
        EXPECT_TRUE(!boost::ends_with(*file, L".prx"));
    }
}

/// Tests whether the DocumentWriter correctly enable the omitTermFreqAndPositions bit in the FieldInfo
TEST_F(OmitTfTest, testOmitTermFreqAndPositions) {
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
    EXPECT_TRUE(fi->fieldInfo(L"f1")->omitTermFreqAndPositions);
    EXPECT_TRUE(fi->fieldInfo(L"f2")->omitTermFreqAndPositions);

    reader->close();
    ram->close();
}

/// Tests whether merging of docs that have different omitTermFreqAndPositions for the same field works
TEST_F(OmitTfTest, testMixedMerge) {
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

    for (int32_t i = 0; i < 30; ++i) {
        writer->addDocument(d);
    }

    // now we add another document which has term freq for field f2 and not for f1 and verify if the SegmentMerger keep things constant
    d = newLucene<Document>();

    // Reverese
    f1->setOmitTermFreqAndPositions(true);
    d->add(f1);

    f2->setOmitTermFreqAndPositions(false);
    d->add(f2);

    for (int32_t i = 0; i < 30; ++i) {
        writer->addDocument(d);
    }

    // force merge
    writer->optimize();
    // flush
    writer->close();

    checkIndex(ram);

    SegmentReaderPtr reader = SegmentReader::getOnlySegmentReader(ram);
    FieldInfosPtr fi = reader->fieldInfos();
    EXPECT_TRUE(fi->fieldInfo(L"f1")->omitTermFreqAndPositions);
    EXPECT_TRUE(fi->fieldInfo(L"f2")->omitTermFreqAndPositions);

    reader->close();
    ram->close();
}

/// Make sure first adding docs that do not omitTermFreqAndPositions for field X, then adding docs that do
/// omitTermFreqAndPositions for that same field
TEST_F(OmitTfTest, testMixedRAM) {
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

    for (int32_t i = 0; i < 5; ++i) {
        writer->addDocument(d);
    }

    f2->setOmitTermFreqAndPositions(true);

    for (int32_t i = 0; i < 20; ++i) {
        writer->addDocument(d);
    }

    // force merge
    writer->optimize();

    // flush
    writer->close();

    checkIndex(ram);

    SegmentReaderPtr reader = SegmentReader::getOnlySegmentReader(ram);
    FieldInfosPtr fi = reader->fieldInfos();
    EXPECT_TRUE(!fi->fieldInfo(L"f1")->omitTermFreqAndPositions);
    EXPECT_TRUE(fi->fieldInfo(L"f2")->omitTermFreqAndPositions);

    reader->close();
    ram->close();
}

/// Verifies no *.prx exists when all fields omit term freq
TEST_F(OmitTfTest, testNoPrxFile) {
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

    for (int32_t i = 0; i < 30; ++i) {
        writer->addDocument(d);
    }

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

namespace TestBasic {

class CountingHitCollectorQ1 : public CountingHitCollector {
protected:
    ScorerPtr scorer;

public:
    virtual void setScorer(const ScorerPtr& scorer) {
        this->scorer = scorer;
    }

    virtual void collect(int32_t doc) {
        EXPECT_EQ(scorer->score(), 1.0);
        CountingHitCollector::collect(doc);
    }
};

class CountingHitCollectorQ2 : public CountingHitCollector {
protected:
    ScorerPtr scorer;

public:
    virtual void setScorer(const ScorerPtr& scorer) {
        this->scorer = scorer;
    }

    virtual void collect(int32_t doc) {
        EXPECT_EQ(scorer->score(), 1.0 + (double)doc);
        CountingHitCollector::collect(doc);
    }
};

class CountingHitCollectorQ3 : public CountingHitCollector {
protected:
    ScorerPtr scorer;

public:
    virtual void setScorer(const ScorerPtr& scorer) {
        this->scorer = scorer;
    }

    virtual void collect(int32_t doc) {
        EXPECT_EQ(scorer->score(), 1.0);
        EXPECT_NE(doc % 2, 0);
        CountingHitCollector::collect(doc);
    }
};

class CountingHitCollectorQ4 : public CountingHitCollector {
protected:
    ScorerPtr scorer;

public:
    virtual void setScorer(const ScorerPtr& scorer) {
        this->scorer = scorer;
    }

    virtual void collect(int32_t doc) {
        EXPECT_EQ(scorer->score(), 1.0);
        EXPECT_EQ(doc % 2, 0);
        CountingHitCollector::collect(doc);
    }
};

}

TEST_F(OmitTfTest, testBasic) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    AnalyzerPtr analyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);

    writer->setMergeFactor(2);
    writer->setMaxBufferedDocs(2);
    writer->setSimilarity(newLucene<SimpleSimilarity>());

    StringStream sb;
    for (int32_t i = 0; i < 30; ++i) {
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
    EXPECT_EQ(15, collector->count);

    searcher->close();
    dir->close();
}
