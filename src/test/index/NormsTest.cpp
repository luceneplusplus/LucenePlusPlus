/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "FSDirectory.h"
#include "IndexWriter.h"
#include "StandardAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "DefaultSimilarity.h"
#include "IndexReader.h"
#include "FileUtils.h"

using namespace Lucene;

class SimilarityOne : public DefaultSimilarity {
public:
    virtual ~SimilarityOne() {
    }

    LUCENE_CLASS(SimilarityOne);

public:
    virtual double lengthNorm(const String& fieldName, int32_t numTokens) {
        return 1.0;
    }
};

/// Test that norms info is preserved during index life - including separate norms, addDocument, addIndexesNoOptimize, optimize.
class NormsTest : public LuceneTestFixture {
public:
    NormsTest() {
        similarityOne = newLucene<SimilarityOne>();
        lastNorm = 0.0;
        normDelta = 0.001;
        numDocNorms = 0;
    }

    virtual ~NormsTest() {
    }

protected:
    static const int32_t NUM_FIELDS;

    SimilarityPtr similarityOne;
    int32_t numDocNorms;
    Collection<double> norms;
    Collection<double> modifiedNorms;

    double lastNorm;
    double normDelta;

public:
    /// return unique norm values that are unchanged by encoding/decoding
    double nextNorm() {
        double norm = lastNorm + normDelta;
        do {
            double norm1 = Similarity::decodeNorm(Similarity::encodeNorm(norm));
            if (norm1 > lastNorm) {
                norm = norm1;
                break;
            }
            norm += normDelta;
        } while (true);
        norms.add(numDocNorms, norm);
        modifiedNorms.add(numDocNorms, norm);
        ++numDocNorms;
        lastNorm = (norm > 10 ? 0 : norm); // there's a limit to how many distinct values can be stored in a ingle byte
        return norm;
    }

    /// create the next document
    DocumentPtr newDoc() {
        DocumentPtr d = newLucene<Document>();
        double boost = nextNorm();
        for (int32_t i = 0; i < 10; ++i) {
            FieldPtr f = newLucene<Field>(L"f" + StringUtils::toString(i), L"v" + StringUtils::toString(i), Field::STORE_NO, Field::INDEX_NOT_ANALYZED);
            f->setBoost(boost);
            d->add(f);
        }
        return d;
    }

    void verifyIndex(const DirectoryPtr& dir) {
        IndexReaderPtr ir = IndexReader::open(dir, false);
        for (int32_t i = 0; i < NUM_FIELDS; ++i) {
            String field = L"f" + StringUtils::toString(i);
            ByteArray b = ir->norms(field);
            EXPECT_EQ(numDocNorms, b.size());
            Collection<double> storedNorms = (i == 1 ? modifiedNorms : norms);
            for (int32_t j = 0; j < b.size(); ++j) {
                double norm = Similarity::decodeNorm(b[j]);
                double norm1 = storedNorms[j];
                EXPECT_EQ(norm, norm1); // 0.000001
            }
        }
    }

    void addDocs(const DirectoryPtr& dir, int32_t ndocs, bool compound) {
        IndexWriterPtr iw = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), false, IndexWriter::MaxFieldLengthLIMITED);
        iw->setMaxBufferedDocs(5);
        iw->setMergeFactor(3);
        iw->setSimilarity(similarityOne);
        iw->setUseCompoundFile(compound);
        for (int32_t i = 0; i < ndocs; ++i) {
            iw->addDocument(newDoc());
        }
        iw->close();
    }

    void modifyNormsForF1(const DirectoryPtr& dir) {
        IndexReaderPtr ir = IndexReader::open(dir, false);
        int32_t n = ir->maxDoc();
        for (int32_t i = 0; i < n; i += 3) { // modify for every third doc
            int32_t k = (i * 3) % modifiedNorms.size();
            double origNorm = modifiedNorms[i];
            double newNorm = modifiedNorms[k];
            modifiedNorms[i] = newNorm;
            modifiedNorms[k] = origNorm;
            ir->setNorm(i, L"f1", newNorm);
            ir->setNorm(k, L"f1", origNorm);
        }
        ir->close();
    }

    void doTestNorms(const DirectoryPtr& dir) {
        for (int32_t i = 0; i < 5; ++i) {
            addDocs(dir, 12, true);
            verifyIndex(dir);
            modifyNormsForF1(dir);
            verifyIndex(dir);
            addDocs(dir, 12, false);
            verifyIndex(dir);
            modifyNormsForF1(dir);
            verifyIndex(dir);
        }
    }

    void createIndex(const DirectoryPtr& dir) {
        IndexWriterPtr iw = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
        iw->setMaxBufferedDocs(5);
        iw->setMergeFactor(3);
        iw->setSimilarity(similarityOne);
        iw->setUseCompoundFile(true);
        iw->close();
    }
};

const int32_t NormsTest::NUM_FIELDS = 10;

/// Test that norms values are preserved as the index is maintained.
/// Including separate norms.
/// Including merging indexes with separate norms.
/// Including optimize.
TEST_F(NormsTest, testNorms) {
    // test with a single index: index1
    String indexDir1(FileUtils::joinPath(getTempDir(), L"lucenetestindex1"));
    DirectoryPtr dir1 = FSDirectory::open(indexDir1);

    norms = Collection<double>::newInstance();
    modifiedNorms = Collection<double>::newInstance();

    createIndex(dir1);
    doTestNorms(dir1);

    // test with a single index: index2
    Collection<double> norms1 = norms;
    Collection<double> modifiedNorms1 = modifiedNorms;
    int32_t numDocNorms1 = numDocNorms;

    norms = Collection<double>::newInstance();
    modifiedNorms = Collection<double>::newInstance();
    numDocNorms = 0;

    String indexDir2(FileUtils::joinPath(getTempDir(), L"lucenetestindex2"));
    DirectoryPtr dir2 = FSDirectory::open(indexDir2);

    createIndex(dir2);
    doTestNorms(dir2);

    // add index1 and index2 to a third index: index3
    String indexDir3(FileUtils::joinPath(getTempDir(), L"lucenetestindex3"));
    DirectoryPtr dir3 = FSDirectory::open(indexDir3);

    createIndex(dir3);
    IndexWriterPtr iw = newLucene<IndexWriter>(dir3, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), false, IndexWriter::MaxFieldLengthLIMITED);
    iw->setMaxBufferedDocs(5);
    iw->setMergeFactor(3);

    iw->addIndexesNoOptimize(newCollection<DirectoryPtr>(dir1, dir2));
    iw->optimize();
    iw->close();

    norms1.addAll(norms.begin(), norms.end());
    norms = norms1;
    modifiedNorms1.addAll(modifiedNorms.begin(), modifiedNorms.end());
    modifiedNorms = modifiedNorms1;
    numDocNorms += numDocNorms1;

    // test with index3
    verifyIndex(dir3);
    doTestNorms(dir3);

    // now with optimize
    iw = newLucene<IndexWriter>(dir3, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), false, IndexWriter::MaxFieldLengthLIMITED);
    iw->setMaxBufferedDocs(5);
    iw->setMergeFactor(3);
    iw->optimize();
    iw->close();
    verifyIndex(dir3);

    dir1->close();
    dir2->close();
    dir3->close();
}
