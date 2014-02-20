/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "DefaultSimilarity.h"
#include "StandardAnalyzer.h"
#include "FSDirectory.h"
#include "IndexWriter.h"
#include "Document.h"
#include "Field.h"
#include "MockRAMDirectory.h"
#include "LogDocMergePolicy.h"
#include "WhitespaceAnalyzer.h"
#include "SegmentReader.h"
#include "_SegmentReader.h"
#include "FileUtils.h"

using namespace Lucene;

class SimilarityOne : public DefaultSimilarity {
public:
    virtual ~SimilarityOne() {
    }

public:
    virtual double lengthNorm(const String& fieldName, int32_t numTokens) {
        return 1.0;
    }
};

class IndexReaderCloneNormsTest : public LuceneTestFixture {
public:
    IndexReaderCloneNormsTest() {
        similarityOne = newLucene<SimilarityOne>();
        anlzr = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
        numDocNorms = 0;
        lastNorm = 0.0;
        normDelta = 0.001;
    }

    virtual ~IndexReaderCloneNormsTest() {
    }

protected:
    static const int32_t NUM_FIELDS;

    SimilarityPtr similarityOne;
    AnalyzerPtr anlzr;
    int32_t numDocNorms;
    Collection<double> norms;
    Collection<double> modifiedNorms;
    double lastNorm;
    double normDelta;

public:
    void createIndex(const DirectoryPtr& dir) {
        IndexWriterPtr iw = newLucene<IndexWriter>(dir, anlzr, true, IndexWriter::MaxFieldLengthLIMITED);
        iw->setMaxBufferedDocs(5);
        iw->setMergeFactor(3);
        iw->setSimilarity(similarityOne);
        iw->setUseCompoundFile(true);
        iw->close();
    }

    void createIndex(const DirectoryPtr& dir, bool multiSegment) {
        IndexWriter::unlock(dir);
        IndexWriterPtr w = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);

        w->setMergePolicy(newLucene<LogDocMergePolicy>(w));

        for (int32_t i = 0; i < 100; ++i) {
            w->addDocument(createDocument(i, 4));
            if (multiSegment && (i % 10) == 0) {
                w->commit();
            }
        }

        if (!multiSegment) {
            w->optimize();
        }

        w->close();

        IndexReaderPtr r = IndexReader::open(dir, false);
        if (multiSegment) {
            EXPECT_TRUE(r->getSequentialSubReaders().size() > 1);
        } else {
            EXPECT_EQ(r->getSequentialSubReaders().size(), 1);
        }
        r->close();
    }

    DocumentPtr createDocument(int32_t n, int32_t numFields) {
        StringStream sb;
        DocumentPtr doc = newLucene<Document>();
        sb << L"a" << n;
        doc->add(newLucene<Field>(L"field1", sb.str(), Field::STORE_YES, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(L"fielda", sb.str(), Field::STORE_YES, Field::INDEX_NOT_ANALYZED_NO_NORMS));
        doc->add(newLucene<Field>(L"fieldb", sb.str(), Field::STORE_YES, Field::INDEX_NO));
        sb << L" b" << n;
        for (int32_t i = 1; i < numFields; ++i) {
            doc->add(newLucene<Field>(L"field" + StringUtils::toString(i + 1), sb.str(), Field::STORE_YES, Field::INDEX_ANALYZED));
        }
        return doc;
    }

    /// try cloning and reopening the norms
    void doTestNorms(const DirectoryPtr& dir) {
        addDocs(dir, 12, true);
        IndexReaderPtr ir = IndexReader::open(dir, false);
        verifyIndex(ir);
        modifyNormsForF1(ir);
        IndexReaderPtr irc = boost::dynamic_pointer_cast<IndexReader>(ir->clone());
        verifyIndex(irc);

        modifyNormsForF1(irc);

        IndexReaderPtr irc3 = boost::dynamic_pointer_cast<IndexReader>(irc->clone());
        verifyIndex(irc3);
        modifyNormsForF1(irc3);
        verifyIndex(irc3);
        irc3->flush();
        irc3->close();
    }

    void modifyNormsForF1(const DirectoryPtr& dir) {
        IndexReaderPtr ir = IndexReader::open(dir, false);
        modifyNormsForF1(ir);
    }

    void modifyNormsForF1(const IndexReaderPtr& ir) {
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
    }

    void addDocs(const DirectoryPtr& dir, int32_t ndocs, bool compound) {
        IndexWriterPtr iw = newLucene<IndexWriter>(dir, anlzr, false, IndexWriter::MaxFieldLengthLIMITED);
        iw->setMaxBufferedDocs(5);
        iw->setMergeFactor(3);
        iw->setSimilarity(similarityOne);
        iw->setUseCompoundFile(compound);
        for (int32_t i = 0; i < ndocs; ++i) {
            iw->addDocument(newDoc());
        }
        iw->close();
    }

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
        // there's a limit to how many distinct values can be stored in a single byte
        lastNorm = (norm > 10 ? 0 : norm);
        return norm;
    }

    void verifyIndex(const DirectoryPtr& dir) {
        IndexReaderPtr ir = IndexReader::open(dir, false);
        verifyIndex(ir);
        ir->close();
    }

    void verifyIndex(const IndexReaderPtr& ir) {
        for (int32_t i = 0; i < NUM_FIELDS; ++i) {
            String field = L"f" + StringUtils::toString(i);
            ByteArray b = ir->norms(field);
            EXPECT_EQ(numDocNorms, b.size());
            Collection<double> storedNorms = (i == 1 ? modifiedNorms : norms);
            for (int32_t j = 0; j < b.size(); ++j) {
                double norm = Similarity::decodeNorm(b[j]);
                double norm1 = storedNorms[j];
                EXPECT_EQ(norm, norm1); // 0.000001 ??
            }
        }
    }
};

const int32_t IndexReaderCloneNormsTest::NUM_FIELDS = 10;

/// Test that norms values are preserved as the index is maintained.  Including separate norms.
/// Including merging indexes with separate norms. Including optimize.
TEST_F(IndexReaderCloneNormsTest, testNorms) {
    // test with a single index: index1
    String indexDir1(FileUtils::joinPath(getTempDir(), L"lucenetestindex1"));
    DirectoryPtr dir1 = FSDirectory::open(indexDir1);
    IndexWriter::unlock(dir1);

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
    IndexWriterPtr iw = newLucene<IndexWriter>(dir3, anlzr, false, IndexWriter::MaxFieldLengthLIMITED);
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
    iw = newLucene<IndexWriter>(dir3, anlzr, false, IndexWriter::MaxFieldLengthLIMITED);
    iw->setMaxBufferedDocs(5);
    iw->setMergeFactor(3);
    iw->optimize();
    iw->close();
    verifyIndex(dir3);

    dir1->close();
    dir2->close();
    dir3->close();
}

TEST_F(IndexReaderCloneNormsTest, testNormsClose) {
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    createIndex(dir1, false);
    SegmentReaderPtr reader1 = SegmentReader::getOnlySegmentReader(dir1);
    reader1->norms(L"field1");
    NormPtr r1norm = reader1->_norms.get(L"field1");
    SegmentReaderRefPtr r1BytesRef = r1norm->bytesRef();
    SegmentReaderPtr reader2 = boost::dynamic_pointer_cast<SegmentReader>(reader1->clone());
    EXPECT_EQ(2, r1norm->bytesRef()->refCount());
    reader1->close();
    EXPECT_EQ(1, r1BytesRef->refCount());
    reader2->norms(L"field1");
    reader2->close();
    dir1->close();
}

TEST_F(IndexReaderCloneNormsTest, testNormsRefCounting) {
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    createIndex(dir1, false);

    IndexReaderPtr reader1 = IndexReader::open(dir1, false);

    IndexReaderPtr reader2C = boost::dynamic_pointer_cast<IndexReader>(reader1->clone());
    SegmentReaderPtr segmentReader2C = SegmentReader::getOnlySegmentReader(reader2C);
    segmentReader2C->norms(L"field1"); // load the norms for the field
    NormPtr reader2CNorm = segmentReader2C->_norms.get(L"field1");
    EXPECT_EQ(2, reader2CNorm->bytesRef()->refCount());

    IndexReaderPtr reader3C = boost::dynamic_pointer_cast<IndexReader>(reader2C->clone());
    SegmentReaderPtr segmentReader3C = SegmentReader::getOnlySegmentReader(reader3C);
    NormPtr reader3CCNorm = segmentReader3C->_norms.get(L"field1");
    EXPECT_EQ(3, reader3CCNorm->bytesRef()->refCount());

    // edit a norm and the refcount should be 1
    IndexReaderPtr reader4C = boost::dynamic_pointer_cast<IndexReader>(reader3C->clone());
    SegmentReaderPtr segmentReader4C = SegmentReader::getOnlySegmentReader(reader4C);
    EXPECT_EQ(4, reader3CCNorm->bytesRef()->refCount());
    reader4C->setNorm(5, L"field1", 0.33);

    // generate a cannot update exception in reader1
    try {
        reader3C->setNorm(1, L"field1", 0.99);
    } catch (LockObtainFailedException& e) {
        EXPECT_TRUE(check_exception(LuceneException::LockObtainFailed)(e));
    }

    // norm values should be different
    EXPECT_NE(Similarity::decodeNorm(segmentReader3C->norms(L"field1")[5]), Similarity::decodeNorm(segmentReader4C->norms(L"field1")[5]));
    NormPtr reader4CCNorm = segmentReader4C->_norms.get(L"field1");
    EXPECT_EQ(3, reader3CCNorm->bytesRef()->refCount());
    EXPECT_EQ(1, reader4CCNorm->bytesRef()->refCount());

    IndexReaderPtr reader5C = boost::dynamic_pointer_cast<IndexReader>(reader4C->clone());
    SegmentReaderPtr segmentReader5C = SegmentReader::getOnlySegmentReader(reader5C);
    NormPtr reader5CCNorm = segmentReader5C->_norms.get(L"field1");
    reader5C->setNorm(5, L"field1", 0.7);
    EXPECT_EQ(1, reader5CCNorm->bytesRef()->refCount());

    reader5C->close();
    reader4C->close();
    reader3C->close();
    reader2C->close();
    reader1->close();
    dir1->close();
}
