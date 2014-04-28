/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "RAMDirectory.h"
#include "Document.h"
#include "DocHelper.h"
#include "WhitespaceAnalyzer.h"
#include "WhitespaceTokenizer.h"
#include "SimpleAnalyzer.h"
#include "StandardAnalyzer.h"
#include "IndexWriter.h"
#include "SegmentInfo.h"
#include "SegmentReader.h"
#include "_SegmentReader.h"
#include "Field.h"
#include "FieldInfo.h"
#include "FieldInfos.h"
#include "TermPositions.h"
#include "Term.h"
#include "TokenFilter.h"
#include "TokenStream.h"
#include "TermAttribute.h"
#include "PayloadAttribute.h"
#include "PositionIncrementAttribute.h"
#include "Payload.h"
#include "TermFreqVector.h"
#include "MiscUtils.h"
#include "UnicodeUtils.h"

using namespace Lucene;

class DocumentWriterTest : public LuceneTestFixture, public DocHelper {
public:
    virtual ~DocumentWriterTest() {
    }
};

TEST_F(DocumentWriterTest, testAddDocument) {
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();

    DocumentPtr testDoc = newLucene<Document>();
    DocHelper::setupDoc(testDoc);
    AnalyzerPtr analyzer = newLucene<WhitespaceAnalyzer>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);
    writer->addDocument(testDoc);
    writer->commit();
    SegmentInfoPtr info = writer->newestSegment();
    writer->close();
    // After adding the document, we should be able to read it back in
    SegmentReaderPtr reader = SegmentReader::get(true, info, IndexReader::DEFAULT_TERMS_INDEX_DIVISOR);
    EXPECT_TRUE(reader);
    DocumentPtr doc = reader->document(0);
    EXPECT_TRUE(doc);

    Collection<FieldPtr> fields = doc->getFields(L"textField2");
    EXPECT_TRUE(fields && fields.size() == 1);
    EXPECT_EQ(fields[0]->stringValue(), DocHelper::FIELD_2_TEXT);
    EXPECT_TRUE(fields[0]->isTermVectorStored());

    fields = doc->getFields(L"textField1");
    EXPECT_TRUE(fields && fields.size() == 1);
    EXPECT_EQ(fields[0]->stringValue(), DocHelper::FIELD_1_TEXT);
    EXPECT_TRUE(!fields[0]->isTermVectorStored());

    fields = doc->getFields(L"keyField");
    EXPECT_TRUE(fields && fields.size() == 1);
    EXPECT_EQ(fields[0]->stringValue(), DocHelper::KEYWORD_TEXT);

    fields = doc->getFields(DocHelper::NO_NORMS_KEY);
    EXPECT_TRUE(fields && fields.size() == 1);
    EXPECT_EQ(fields[0]->stringValue(), DocHelper::NO_NORMS_TEXT);

    fields = doc->getFields(DocHelper::TEXT_FIELD_3_KEY);
    EXPECT_TRUE(fields && fields.size() == 1);
    EXPECT_EQ(fields[0]->stringValue(), DocHelper::FIELD_3_TEXT);

    // test that the norms are not present in the segment if omitNorms is true
    for (int32_t i = 0; i < reader->core->fieldInfos->size(); ++i) {
        FieldInfoPtr fi = reader->core->fieldInfos->fieldInfo(i);
        if (fi->isIndexed) {
            EXPECT_TRUE(fi->omitNorms == !reader->hasNorms(fi->name));
        }
    }
}

namespace TestPositionIncrementGap {

DECLARE_SHARED_PTR(TestableAnalyzer)

class TestableAnalyzer : public Analyzer {
public:
    virtual ~TestableAnalyzer() {
    }

    LUCENE_CLASS(TestableAnalyzer);

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        return newLucene<WhitespaceTokenizer>(reader);
    }

    virtual int32_t getPositionIncrementGap(const String& fieldName) {
        return 500;
    }
};

}

TEST_F(DocumentWriterTest, testPositionIncrementGap) {
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();

    AnalyzerPtr analyzer = newLucene<TestPositionIncrementGap::TestableAnalyzer>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);

    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"repeated", L"repeated one", Field::STORE_YES, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"repeated", L"repeated two", Field::STORE_YES, Field::INDEX_ANALYZED));

    writer->addDocument(doc);
    writer->commit();
    SegmentInfoPtr info = writer->newestSegment();
    writer->close();
    SegmentReaderPtr reader = SegmentReader::get(true, info, IndexReader::DEFAULT_TERMS_INDEX_DIVISOR);

    TermPositionsPtr termPositions = reader->termPositions(newLucene<Term>(L"repeated", L"repeated"));
    EXPECT_TRUE(termPositions->next());
    int32_t freq = termPositions->freq();
    EXPECT_EQ(2, freq);
    EXPECT_EQ(0, termPositions->nextPosition());
    EXPECT_EQ(502, termPositions->nextPosition());
}

namespace TestTokenReuse {

DECLARE_SHARED_PTR(TestableTokenFilter)
DECLARE_SHARED_PTR(TestableAnalyzer)

class TestableTokenFilter : public TokenFilter {
public:
    TestableTokenFilter(const ReaderPtr& reader) : TokenFilter(newLucene<WhitespaceTokenizer>(reader)) {
        first = true;
        termAtt = addAttribute<TermAttribute>();
        payloadAtt = addAttribute<PayloadAttribute>();
        posIncrAtt = addAttribute<PositionIncrementAttribute>();
    }

    virtual ~TestableTokenFilter() {
    }

    LUCENE_CLASS(TestableTokenFilter);

public:
    bool first;
    AttributeSourceStatePtr state;
    TermAttributePtr termAtt;
    PayloadAttributePtr payloadAtt;
    PositionIncrementAttributePtr posIncrAtt;

public:
    virtual bool incrementToken() {
        if (state) {
            restoreState(state);
            payloadAtt->setPayload(PayloadPtr());
            posIncrAtt->setPositionIncrement(0);
            static const wchar_t buffer[] = L"b";

            termAtt->setTermBuffer(buffer, 0, 1);
            state.reset();
            return true;
        }

        bool hasNext = input->incrementToken();
        if (!hasNext) {
            return false;
        }
        if (UnicodeUtil::isDigit(termAtt->termBufferArray()[0])) {
            posIncrAtt->setPositionIncrement(termAtt->termBufferArray()[0] - L'0');
        }
        if (first) {
            ByteArray payload = ByteArray::newInstance(1);
            payload.get()[0] = 100;

            // set payload on first position only
            payloadAtt->setPayload(newLucene<Payload>(payload));
            first = false;
        }
        // index a "synonym" for every token
        state = captureState();
        return true;
    }
};

class TestableAnalyzer : public Analyzer {
public:
    virtual ~TestableAnalyzer() {
    }

    LUCENE_CLASS(TestableAnalyzer);

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        return newLucene<TestableTokenFilter>(reader);
    }
};

}

TEST_F(DocumentWriterTest, testTokenReuse) {
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();

    AnalyzerPtr analyzer = newLucene<TestTokenReuse::TestableAnalyzer>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);

    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"f1", L"a 5 a a", Field::STORE_YES, Field::INDEX_ANALYZED));

    writer->addDocument(doc);
    writer->commit();
    SegmentInfoPtr info = writer->newestSegment();
    writer->close();
    SegmentReaderPtr reader = SegmentReader::get(true, info, IndexReader::DEFAULT_TERMS_INDEX_DIVISOR);

    TermPositionsPtr termPositions = reader->termPositions(newLucene<Term>(L"f1", L"a"));
    EXPECT_TRUE(termPositions->next());
    int32_t freq = termPositions->freq();
    EXPECT_EQ(3, freq);
    EXPECT_EQ(0, termPositions->nextPosition());
    EXPECT_TRUE(termPositions->isPayloadAvailable());
    EXPECT_EQ(6, termPositions->nextPosition());
    EXPECT_FALSE(termPositions->isPayloadAvailable());
    EXPECT_EQ(7, termPositions->nextPosition());
    EXPECT_FALSE(termPositions->isPayloadAvailable());
}

namespace TestPreAnalyzedField {

DECLARE_SHARED_PTR(TestableTokenStream)

class TestableTokenStream : public TokenStream {
public:
    TestableTokenStream() {
        tokens = newCollection<String>(L"term1", L"term2", L"term3", L"term2");
        index = 0;
        termAtt = addAttribute<TermAttribute>();
    }

    virtual ~TestableTokenStream() {
    }

    LUCENE_CLASS(TestableTokenStream);

protected:
    Collection<String> tokens;
    int32_t index;
    TermAttributePtr termAtt;

public:
    virtual bool incrementToken() {
        if (index == tokens.size()) {
            return false;
        } else {
            clearAttributes();
            termAtt->setTermBuffer(tokens[index++]);
            return true;
        }
    }
};

}

TEST_F(DocumentWriterTest, testPreAnalyzedField) {
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();

    doc->add(newLucene<Field>(L"preanalyzed", newLucene<TestPreAnalyzedField::TestableTokenStream>(), Field::TERM_VECTOR_NO));

    writer->addDocument(doc);
    writer->commit();
    SegmentInfoPtr info = writer->newestSegment();
    writer->close();
    SegmentReaderPtr reader = SegmentReader::get(true, info, IndexReader::DEFAULT_TERMS_INDEX_DIVISOR);

    TermPositionsPtr termPositions = reader->termPositions(newLucene<Term>(L"preanalyzed", L"term1"));
    EXPECT_TRUE(termPositions->next());
    EXPECT_EQ(1, termPositions->freq());
    EXPECT_EQ(0, termPositions->nextPosition());

    termPositions->seek(newLucene<Term>(L"preanalyzed", L"term2"));
    EXPECT_TRUE(termPositions->next());
    EXPECT_EQ(2, termPositions->freq());
    EXPECT_EQ(1, termPositions->nextPosition());
    EXPECT_EQ(3, termPositions->nextPosition());

    termPositions->seek(newLucene<Term>(L"preanalyzed", L"term3"));
    EXPECT_TRUE(termPositions->next());
    EXPECT_EQ(1, termPositions->freq());
    EXPECT_EQ(2, termPositions->nextPosition());
}

/// Test adding two fields with the same name, but with different term vector setting
TEST_F(DocumentWriterTest, testMixedTermVectorSettingsSameField) {
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    DocumentPtr doc = newLucene<Document>();
    // f1 first without tv then with tv
    doc->add(newLucene<Field>(L"f1", L"v1", Field::STORE_YES, Field::INDEX_NOT_ANALYZED, Field::TERM_VECTOR_NO));
    doc->add(newLucene<Field>(L"f1", L"v2", Field::STORE_YES, Field::INDEX_NOT_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    // f2 first with tv then without tv
    doc->add(newLucene<Field>(L"f2", L"v1", Field::STORE_YES, Field::INDEX_NOT_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    doc->add(newLucene<Field>(L"f2", L"v2", Field::STORE_YES, Field::INDEX_NOT_ANALYZED, Field::TERM_VECTOR_NO));

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->addDocument(doc);
    writer->close();

    checkIndex(dir);

    IndexReaderPtr reader = IndexReader::open(dir, true);
    // f1
    TermFreqVectorPtr tfv1 = reader->getTermFreqVector(0, L"f1");
    EXPECT_TRUE(tfv1);
    EXPECT_EQ(2, tfv1->getTerms().size());
    // f2
    TermFreqVectorPtr tfv2 = reader->getTermFreqVector(0, L"f2");
    EXPECT_TRUE(tfv2);
    EXPECT_EQ(2, tfv2->getTerms().size());
}

/// Test adding two fields with the same name, one indexed the other stored only. The omitNorms and
/// omitTermFreqAndPositions setting of the stored field should not affect the indexed one
TEST_F(DocumentWriterTest, testMixedTermVectorSettingsSameField2) {
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    DocumentPtr doc = newLucene<Document>();
    // f1 has no norms
    doc->add(newLucene<Field>(L"f1", L"v1", Field::STORE_NO, Field::INDEX_ANALYZED_NO_NORMS));
    doc->add(newLucene<Field>(L"f1", L"v2", Field::STORE_YES, Field::INDEX_NO));
    // f2 has no TF
    FieldPtr f = newLucene<Field>(L"f2", L"v1", Field::STORE_NO, Field::INDEX_ANALYZED);
    f->setOmitTermFreqAndPositions(true);
    doc->add(f);
    doc->add(newLucene<Field>(L"f2", L"v2", Field::STORE_YES, Field::INDEX_NO));

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->addDocument(doc);
    writer->optimize(); // be sure to have a single segment
    writer->close();

    checkIndex(dir);

    SegmentReaderPtr reader = SegmentReader::getOnlySegmentReader(dir);
    FieldInfosPtr fi = reader->fieldInfos();
    // f1
    EXPECT_TRUE(!reader->hasNorms(L"f1"));
    EXPECT_TRUE(!fi->fieldInfo(L"f1")->omitTermFreqAndPositions);
    // f2
    EXPECT_TRUE(reader->hasNorms(L"f2"));
    EXPECT_TRUE(fi->fieldInfo(L"f2")->omitTermFreqAndPositions);
}
