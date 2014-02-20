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
#include "IndexReader.h"
#include "SegmentMerger.h"
#include "TermDocs.h"
#include "Term.h"
#include "TermFreqVector.h"
#include "Field.h"
#include "DefaultSimilarity.h"
#include "TermPositionVector.h"

using namespace Lucene;

class SegmentMergerTest : public LuceneTestFixture, public DocHelper {
public:
    SegmentMergerTest() {
        mergedDir = newLucene<RAMDirectory>();
        mergedSegment = L"test";
        merge1Dir = newLucene<RAMDirectory>();
        doc1 = newLucene<Document>();
        merge2Dir = newLucene<RAMDirectory>();
        doc2 = newLucene<Document>();

        DocHelper::setupDoc(doc1);
        SegmentInfoPtr info1 = DocHelper::writeDoc(merge1Dir, doc1);
        DocHelper::setupDoc(doc2);
        SegmentInfoPtr info2 = DocHelper::writeDoc(merge2Dir, doc2);
        reader1 = SegmentReader::get(true, info1, IndexReader::DEFAULT_TERMS_INDEX_DIVISOR);
        reader2 = SegmentReader::get(true, info2, IndexReader::DEFAULT_TERMS_INDEX_DIVISOR);
    }

    virtual ~SegmentMergerTest() {
    }

protected:
    // The variables for the new merged segment
    DirectoryPtr mergedDir;
    String mergedSegment;

    // First segment to be merged
    DirectoryPtr merge1Dir;
    DocumentPtr doc1;
    SegmentReaderPtr reader1;

    // Second Segment to be merged
    DirectoryPtr merge2Dir;
    DocumentPtr doc2;
    SegmentReaderPtr reader2;

public:
    void checkNorms(const IndexReaderPtr& reader) {
        // test omit norms
        for (int32_t i = 0; i < DocHelper::fields.size(); ++i) {
            FieldPtr f = DocHelper::fields[i];
            if (f->isIndexed()) {
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
};

TEST_F(SegmentMergerTest, testMerge) {
    SegmentMergerPtr merger = newLucene<SegmentMerger>(mergedDir, mergedSegment);
    merger->add(reader1);
    merger->add(reader2);
    int32_t docsMerged = merger->merge();
    merger->closeReaders();
    EXPECT_EQ(docsMerged, 2);

    // Should be able to open a new SegmentReader against the new directory
    SegmentReaderPtr mergedReader = SegmentReader::get(true, newLucene<SegmentInfo>(mergedSegment, docsMerged, mergedDir, false, true), IndexReader::DEFAULT_TERMS_INDEX_DIVISOR);
    EXPECT_TRUE(mergedReader);
    EXPECT_EQ(mergedReader->numDocs(), 2);
    DocumentPtr newDoc1 = mergedReader->document(0);
    EXPECT_TRUE(newDoc1);

    // There are 2 unstored fields on the document
    EXPECT_EQ(DocHelper::numFields(newDoc1), DocHelper::numFields(doc1) - DocHelper::unstored.size());
    DocumentPtr newDoc2 = mergedReader->document(1);
    EXPECT_TRUE(newDoc2);
    EXPECT_EQ(DocHelper::numFields(newDoc2), DocHelper::numFields(doc2) - DocHelper::unstored.size());

    TermDocsPtr termDocs = mergedReader->termDocs(newLucene<Term>(DocHelper::TEXT_FIELD_2_KEY, L"field"));
    EXPECT_TRUE(termDocs);
    EXPECT_TRUE(termDocs->next());

    HashSet<String> stored = mergedReader->getFieldNames(IndexReader::FIELD_OPTION_INDEXED_WITH_TERMVECTOR);
    EXPECT_TRUE(stored);

    EXPECT_EQ(stored.size(), 3);

    TermFreqVectorPtr vector = mergedReader->getTermFreqVector(0, DocHelper::TEXT_FIELD_2_KEY);
    EXPECT_TRUE(vector);
    Collection<String> terms = vector->getTerms();
    EXPECT_TRUE(terms);

    EXPECT_EQ(terms.size(), 3);
    Collection<int32_t> freqs = vector->getTermFrequencies();
    EXPECT_TRUE(freqs);

    EXPECT_TRUE(boost::dynamic_pointer_cast<TermPositionVector>(vector));

    for (int32_t i = 0; i < terms.size(); ++i) {
        String term = terms[i];
        int32_t freq = freqs[i];

        EXPECT_TRUE(String(DocHelper::FIELD_2_TEXT).find(term) != String::npos);
        EXPECT_EQ(DocHelper::FIELD_2_FREQS[i], freq);
    }

    checkNorms(mergedReader);
}
