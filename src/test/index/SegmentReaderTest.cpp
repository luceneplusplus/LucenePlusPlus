/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "DocHelper.h"
#include "RAMDirectory.h"
#include "Document.h"
#include "SegmentReader.h"
#include "SegmentInfo.h"
#include "Field.h"
#include "TermEnum.h"
#include "Term.h"
#include "TermDocs.h"
#include "TermPositions.h"
#include "DefaultSimilarity.h"
#include "TermFreqVector.h"

using namespace Lucene;

class SegmentReaderTest : public LuceneTestFixture, public DocHelper {
public:
    SegmentReaderTest() {
        dir = newLucene<RAMDirectory>();
        testDoc = newLucene<Document>();
        DocHelper::setupDoc(testDoc);
        SegmentInfoPtr info = DocHelper::writeDoc(dir, testDoc);
        reader = SegmentReader::get(true, info, IndexReader::DEFAULT_TERMS_INDEX_DIVISOR);
    }

    virtual ~SegmentReaderTest() {
    }

protected:
    RAMDirectoryPtr dir;
    DocumentPtr testDoc;
    SegmentReaderPtr reader;
};

TEST_F(SegmentReaderTest, testSegmentReader) {
    EXPECT_TRUE(dir);
    EXPECT_TRUE(reader);
    EXPECT_TRUE(DocHelper::nameValues.size() > 0);
    EXPECT_EQ(DocHelper::numFields(testDoc), DocHelper::all.size());
}

TEST_F(SegmentReaderTest, testDocument) {
    EXPECT_EQ(reader->numDocs(), 1);
    EXPECT_TRUE(reader->maxDoc() >= 1);
    DocumentPtr result = reader->document(0);
    EXPECT_TRUE(result);

    // There are 2 unstored fields on the document that are not preserved across writing
    EXPECT_EQ(DocHelper::numFields(result), DocHelper::numFields(testDoc) - DocHelper::unstored.size());

    Collection<FieldablePtr> fields = result->getFields();
    for (Collection<FieldablePtr>::iterator field = fields.begin(); field != fields.end(); ++field) {
        EXPECT_TRUE(*field);
        EXPECT_TRUE(DocHelper::nameValues.contains((*field)->name()));
    }
}

TEST_F(SegmentReaderTest, testDelete) {
    DocumentPtr docToDelete = newLucene<Document>();
    DocHelper::setupDoc(docToDelete);
    SegmentInfoPtr info = DocHelper::writeDoc(dir, docToDelete);
    SegmentReaderPtr deleteReader = SegmentReader::get(false, info, IndexReader::DEFAULT_TERMS_INDEX_DIVISOR);
    EXPECT_TRUE(deleteReader);
    EXPECT_EQ(deleteReader->numDocs(), 1);
    deleteReader->deleteDocument(0);
    EXPECT_TRUE(deleteReader->isDeleted(0));
    EXPECT_TRUE(deleteReader->hasDeletions());
    EXPECT_EQ(deleteReader->numDocs(), 0);
}

TEST_F(SegmentReaderTest, testGetFieldNameVariations) {
    HashSet<String> result = reader->getFieldNames(IndexReader::FIELD_OPTION_ALL);
    EXPECT_TRUE(result);
    EXPECT_EQ(result.size(), DocHelper::all.size());
    for (HashSet<String>::iterator field = result.begin(); field != result.end(); ++field) {
        EXPECT_TRUE(DocHelper::nameValues.contains(*field) || field->empty());
    }
    result = reader->getFieldNames(IndexReader::FIELD_OPTION_INDEXED);
    EXPECT_TRUE(result);
    EXPECT_EQ(result.size(), DocHelper::indexed.size());
    for (HashSet<String>::iterator field = result.begin(); field != result.end(); ++field) {
        EXPECT_TRUE(DocHelper::indexed.contains(*field) || field->empty());
    }
    result = reader->getFieldNames(IndexReader::FIELD_OPTION_UNINDEXED);
    EXPECT_TRUE(result);
    EXPECT_EQ(result.size(), DocHelper::unindexed.size());

    // Get all indexed fields that are storing term vectors
    result = reader->getFieldNames(IndexReader::FIELD_OPTION_INDEXED_WITH_TERMVECTOR);
    EXPECT_TRUE(result);
    EXPECT_EQ(result.size(), DocHelper::termvector.size());

    result = reader->getFieldNames(IndexReader::FIELD_OPTION_INDEXED_NO_TERMVECTOR);
    EXPECT_TRUE(result);
    EXPECT_EQ(result.size(), DocHelper::notermvector.size());
}

TEST_F(SegmentReaderTest, testTerms) {
    TermEnumPtr terms = reader->terms();
    EXPECT_TRUE(terms);
    while (terms->next()) {
        TermPtr term = terms->term();
        EXPECT_TRUE(term);
        String fieldValue = DocHelper::nameValues.get(term->field());
        EXPECT_NE(fieldValue.find(term->text()), -1);
    }

    TermDocsPtr termDocs = reader->termDocs();
    EXPECT_TRUE(termDocs);
    termDocs->seek(newLucene<Term>(DocHelper::TEXT_FIELD_1_KEY, L"field"));
    EXPECT_TRUE(termDocs->next());

    termDocs->seek(newLucene<Term>(DocHelper::NO_NORMS_KEY, DocHelper::NO_NORMS_TEXT));
    EXPECT_TRUE(termDocs->next());

    TermPositionsPtr positions = reader->termPositions();
    positions->seek(newLucene<Term>(DocHelper::TEXT_FIELD_1_KEY, L"field"));
    EXPECT_TRUE(positions);
    EXPECT_EQ(positions->doc(), 0);
    EXPECT_TRUE(positions->nextPosition() >= 0);
}

TEST_F(SegmentReaderTest, testNorms) {
    // test omit norms
    for (int32_t i = 0; i < DocHelper::fields.size(); ++i) {
        FieldPtr f = DocHelper::fields[i];
        if (f->isIndexed()) {
            bool a = reader->hasNorms(f->name());
            bool b = !f->getOmitNorms();

            EXPECT_EQ(reader->hasNorms(f->name()), !f->getOmitNorms());
            EXPECT_EQ(reader->hasNorms(f->name()), !DocHelper::noNorms.contains(f->name()));

            if (!reader->hasNorms(f->name())) {
                // test for fake norms of 1.0 or null depending on the flag
                ByteArray norms = reader->norms(f->name());
                uint8_t norm1 = DefaultSimilarity::encodeNorm(1.0);
                EXPECT_TRUE(!norms);
                norms.resize(reader->maxDoc());
                reader->norms(f->name(), norms, 0);
                for (int32_t j = 0; j < reader->maxDoc(); ++j) {
                    EXPECT_EQ(norms[j], norm1);
                }
            }
        }
    }
}

TEST_F(SegmentReaderTest, testTermVectors) {
    TermFreqVectorPtr result = reader->getTermFreqVector(0, DocHelper::TEXT_FIELD_2_KEY);
    EXPECT_TRUE(result);
    Collection<String> terms = result->getTerms();
    Collection<int32_t> freqs = result->getTermFrequencies();
    EXPECT_TRUE(terms);
    EXPECT_EQ(terms.size(), 3);
    EXPECT_TRUE(freqs);
    EXPECT_EQ(freqs.size(), 3);
    for (int32_t i = 0; i < terms.size(); ++i) {
        String term = terms[i];
        int32_t freq = freqs[i];
        EXPECT_NE(String(DocHelper::FIELD_2_TEXT).find(term), -1);
        EXPECT_TRUE(freq > 0);
    }

    Collection<TermFreqVectorPtr> results = reader->getTermFreqVectors(0);
    EXPECT_TRUE(results);
    EXPECT_EQ(results.size(), 3);
}
