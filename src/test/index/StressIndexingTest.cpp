/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "MockRAMDirectory.h"
#include "ConcurrentMergeScheduler.h"
#include "FSDirectory.h"
#include "IndexWriter.h"
#include "SimpleAnalyzer.h"
#include "LuceneThread.h"
#include "Document.h"
#include "Field.h"
#include "IndexSearcher.h"
#include "Term.h"
#include "TermQuery.h"
#include "IndexReader.h"
#include "WhitespaceAnalyzer.h"
#include "TermDocs.h"
#include "TermEnum.h"
#include "TermFreqVector.h"
#include "TermPositionVector.h"
#include "TermVectorOffsetInfo.h"
#include "SegmentTermPositionVector.h"
#include "Random.h"
#include "MiscUtils.h"
#include "FileUtils.h"

using namespace Lucene;

typedef LuceneTestFixture StressIndexingTest;

DECLARE_SHARED_PTR(DocsAndWriter)

class DocsAndWriter : public LuceneObject {
public:
    virtual ~DocsAndWriter() {
    }

    LUCENE_CLASS(DocsAndWriter);

public:
    HashMap<String, DocumentPtr> docs;
    IndexWriterPtr writer;
};

class MockIndexWriter : public IndexWriter {
public:
    MockIndexWriter(const DirectoryPtr& dir, const AnalyzerPtr& a, bool create, int32_t mfl) : IndexWriter(dir, a, create, mfl) {
        rand = newLucene<Random>();
    }

    virtual ~MockIndexWriter() {
    }

    LUCENE_CLASS(MockIndexWriter);

protected:
    RandomPtr rand;

public:
    virtual bool testPoint(const String& name) {
        if (rand->nextInt(4) == 2) {
            LuceneThread::threadYield();
        }
        return true;
    }
};

static int32_t bigFieldSize = 10;
static int32_t maxFields = 4;
static bool sameFieldOrder = false;
static int32_t mergeFactor = 3;
static int32_t maxBufferedDocs = 3;
static int32_t seed = 0;

DECLARE_SHARED_PTR(IndexingThread)

struct lessFieldName {
    inline bool operator()(const FieldablePtr& first, const FieldablePtr& second) const {
        return (first->name() < second->name());
    }
};

class IndexingThread : public LuceneThread {
public:
    IndexingThread() {
        base = 0;
        range = 0;
        iterations = 0;
        docs = HashMap<String, DocumentPtr>::newInstance();
        buffer.resize(100);
        r = newLucene<Random>();
    }

    virtual ~IndexingThread() {
    }

    LUCENE_CLASS(IndexingThread);

public:
    IndexWriterPtr w;
    int32_t base;
    int32_t range;
    int32_t iterations;
    HashMap<String, DocumentPtr> docs;
    CharArray buffer;
    RandomPtr r;

public:
    int32_t nextInt(int32_t limit = INT_MAX) {
        return r->nextInt(limit);
    }

    /// start is inclusive and end is exclusive
    int32_t nextInt(int32_t start, int32_t end) {
        return start + r->nextInt(end - start);
    }

    int32_t addUTF8Token(int32_t start) {
        int32_t end = start + nextInt(20);
        if (buffer.size() < 1 + end) {
            buffer.resize((int32_t)((double)(1 + end) * 1.25));
        }

        for (int32_t i = start; i < end; ++i) {
            int32_t t = nextInt(5);
            if (t == 0 && i < end - 1) {
#ifdef LPP_UNICODE_CHAR_SIZE_2
                // Make a surrogate pair
                // High surrogate
                buffer[i++] = (wchar_t)nextInt(0xd800, 0xdc00);
                // Low surrogate
                buffer[i] = (wchar_t)nextInt(0xdc00, 0xe000);
#else
                buffer[i] = (wchar_t)nextInt(0x10dc00, 0x10e000);
#endif
            } else if (t <= 1) {
                buffer[i] = (wchar_t)nextInt(0x01, 0x80);
            } else if (t == 2) {
                buffer[i] = (wchar_t)nextInt(0x80, 0x800);
            } else if (t == 3) {
                buffer[i] = (wchar_t)nextInt(0x800, 0xd800);
            } else if (t == 4) {
                buffer[i] = (wchar_t)nextInt(0xe000, 0xfff0);
            }
        }

        buffer[end] = L' ';
        return 1 + end;
    }

    String getString(int32_t tokens) {
        tokens = tokens != 0 ? tokens : r->nextInt(4) + 1;

        // Half the time make a random UTF8 string
        if (nextInt() % 2 == 1) {
            return getUTF8String(tokens);
        }

        CharArray arr(CharArray::newInstance(tokens * 2));
        for (int32_t i = 0; i < tokens; ++i) {
            arr[i * 2] = (wchar_t)(L'A' + r->nextInt(10));
            arr[i * 2 + 1] = L' ';
        }
        return String(arr.get(), arr.size());
    }

    String getUTF8String(int32_t tokens) {
        int32_t upto = 0;
        MiscUtils::arrayFill(buffer.get(), 0, buffer.size(), (wchar_t)0);
        for (int32_t i = 0; i < tokens; ++i) {
            upto = addUTF8Token(upto);
        }
        return String(buffer.get(), upto);
    }

    String getIdString() {
        return StringUtils::toString(base + nextInt(range));
    }

    void indexDoc() {
        DocumentPtr d = newLucene<Document>();

        Collection<FieldPtr> fields = Collection<FieldPtr>::newInstance();
        String idString = getIdString();

        FieldPtr idField =  newLucene<Field>(newLucene<Term>(L"id", L"")->field(), idString, Field::STORE_YES, Field::INDEX_ANALYZED_NO_NORMS);
        fields.add(idField);

        int32_t numFields = nextInt(maxFields);
        for (int32_t i = 0; i < numFields; ++i) {
            Field::TermVector tvVal = Field::TERM_VECTOR_NO;
            switch (nextInt(4)) {
            case 0:
                tvVal = Field::TERM_VECTOR_NO;
                break;
            case 1:
                tvVal = Field::TERM_VECTOR_YES;
                break;
            case 2:
                tvVal = Field::TERM_VECTOR_WITH_POSITIONS;
                break;
            case 3:
                tvVal = Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS;
                break;
            }

            switch (nextInt(4)) {
            case 0:
                fields.add(newLucene<Field>(L"f" + StringUtils::toString(nextInt(100)), getString(1), Field::STORE_YES, Field::INDEX_ANALYZED_NO_NORMS, tvVal));
                break;
            case 1:
                fields.add(newLucene<Field>(L"f" + StringUtils::toString(nextInt(100)), getString(0), Field::STORE_NO, Field::INDEX_ANALYZED, tvVal));
                break;
            case 2:
                fields.add(newLucene<Field>(L"f" + StringUtils::toString(nextInt(100)), getString(0), Field::STORE_YES, Field::INDEX_NO, Field::TERM_VECTOR_NO));
                break;
            case 3:
                fields.add(newLucene<Field>(L"f" + StringUtils::toString(nextInt(100)), getString(bigFieldSize), Field::STORE_YES, Field::INDEX_ANALYZED, tvVal));
                break;
            }
        }

        if (sameFieldOrder) {
            std::sort(fields.begin(), fields.end(), lessFieldName());
        } else {
            // random placement of id field also
            std::swap(*fields.begin(), *(fields.begin() + nextInt(fields.size())));
        }

        for (int32_t i = 0; i < fields.size(); ++i) {
            d->add(fields[i]);
        }

        w->updateDocument(newLucene<Term>(L"id", L"")->createTerm(idString), d);
        docs.put(idString, d);
    }

    void deleteDoc() {
        String idString = getIdString();
        w->deleteDocuments(newLucene<Term>(L"id", L"")->createTerm(idString));
        docs.remove(idString);
    }

    void deleteByQuery() {
        String idString = getIdString();
        w->deleteDocuments(newLucene<TermQuery>(newLucene<Term>(L"id", L"")->createTerm(idString)));
        docs.remove(idString);
    }

    virtual void run() {
        try {
            r->setSeed(base + range + seed);
            for (int32_t i = 0; i < iterations; ++i) {
                int32_t what = nextInt(100);
                if (what < 5) {
                    deleteDoc();
                } else if (what < 10) {
                    deleteByQuery();
                } else {
                    indexDoc();
                }
            }
        } catch (LuceneException& e) {
            FAIL() << "Unexpected exception: " << e.getError();
        }
    }
};

static void verifyEquals(const IndexReaderPtr& r1, const DirectoryPtr& dir2, const String& idField);
static void verifyEquals(const DirectoryPtr& dir1, const DirectoryPtr& dir2, const String& idField);
static void verifyEquals(const IndexReaderPtr& r1, const IndexReaderPtr& r2, const String& idField);
static void verifyEquals(const DocumentPtr& d1, const DocumentPtr& d2);
static void verifyEquals(Collection<TermFreqVectorPtr> d1, Collection<TermFreqVectorPtr> d2);

static DocsAndWriterPtr indexRandomIWReader(int32_t numThreads, int32_t iterations, int32_t range, const DirectoryPtr& dir) {
    HashMap<String, DocumentPtr> docs = HashMap<String, DocumentPtr>::newInstance();
    IndexWriterPtr w = newLucene<MockIndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
    w->setUseCompoundFile(false);

    // force many merges
    w->setMergeFactor(mergeFactor);
    w->setRAMBufferSizeMB(0.1);
    w->setMaxBufferedDocs(maxBufferedDocs);

    Collection<IndexingThreadPtr> threads = Collection<IndexingThreadPtr>::newInstance(numThreads);
    for (int32_t i = 0; i < threads.size(); ++i) {
        IndexingThreadPtr th = newLucene<IndexingThread>();
        th->w = w;
        th->base = 1000000 * i;
        th->range = range;
        th->iterations = iterations;
        threads[i] = th;
    }

    for (int32_t i = 0; i < threads.size(); ++i) {
        threads[i]->start();
    }
    for (int32_t i = 0; i < threads.size(); ++i) {
        threads[i]->join();
    }

    for (int32_t i = 0; i < threads.size(); ++i) {
        IndexingThreadPtr th = threads[i];
        SyncLock syncLock(th);
        docs.putAll(th->docs.begin(), th->docs.end());
    }

    checkIndex(dir);
    DocsAndWriterPtr dw = newLucene<DocsAndWriter>();
    dw->docs = docs;
    dw->writer = w;
    return dw;
}

static HashMap<String, DocumentPtr> indexRandom(int32_t numThreads, int32_t iterations, int32_t range, const DirectoryPtr& dir) {
    HashMap<String, DocumentPtr> docs = HashMap<String, DocumentPtr>::newInstance();

    for (int32_t iter = 0; iter < 3; ++iter) {
        IndexWriterPtr w = newLucene<MockIndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
        w->setUseCompoundFile(false);

        // force many merges
        w->setMergeFactor(mergeFactor);
        w->setRAMBufferSizeMB(0.1);
        w->setMaxBufferedDocs(maxBufferedDocs);

        Collection<IndexingThreadPtr> threads = Collection<IndexingThreadPtr>::newInstance(numThreads);
        for (int32_t i = 0; i < threads.size(); ++i) {
            IndexingThreadPtr th = newLucene<IndexingThread>();
            th->w = w;
            th->base = 1000000 * i;
            th->range = range;
            th->iterations = iterations;
            threads[i] = th;
        }

        for (int32_t i = 0; i < threads.size(); ++i) {
            threads[i]->start();
        }
        for (int32_t i = 0; i < threads.size(); ++i) {
            threads[i]->join();
        }

        w->close();

        for (int32_t i = 0; i < threads.size(); ++i) {
            IndexingThreadPtr th = threads[i];
            SyncLock syncLock(th);
            docs.putAll(th->docs.begin(), th->docs.end());
        }
    }

    checkIndex(dir);

    return docs;
}

static void indexSerial(HashMap<String, DocumentPtr> docs, const DirectoryPtr& dir) {
    IndexWriterPtr w = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);

    // index all docs in a single thread
    for (HashMap<String, DocumentPtr>::iterator iter = docs.begin(); iter != docs.end(); ++iter) {
        DocumentPtr d = iter->second;
        Collection<FieldablePtr> fields = d->getFields();

        // put fields in same order each time
        std::sort(fields.begin(), fields.end(), lessFieldName());

        DocumentPtr d1 = newLucene<Document>();
        d1->setBoost(d->getBoost());
        for (Collection<FieldablePtr>::iterator field = fields.begin(); field != fields.end(); ++field) {
            d1->add(*field);
        }
        w->addDocument(d1);
    }
    w->close();
}

static void verifyEquals(const IndexReaderPtr& r1, const DirectoryPtr& dir2, const String& idField) {
    IndexReaderPtr r2 = IndexReader::open(dir2, true);
    verifyEquals(r1, r2, idField);
    r2->close();
}

static void verifyEquals(const DirectoryPtr& dir1, const DirectoryPtr& dir2, const String& idField) {
    IndexReaderPtr r1 = IndexReader::open(dir1, true);
    IndexReaderPtr r2 = IndexReader::open(dir2, true);
    verifyEquals(r1, r2, idField);
    r1->close();
    r2->close();
}

static void verifyEquals(const IndexReaderPtr& r1, const IndexReaderPtr& r2, const String& idField) {
    EXPECT_EQ(r1->numDocs(), r2->numDocs());
    bool hasDeletes = !(r1->maxDoc() == r2->maxDoc() && r1->numDocs() == r1->maxDoc());

    Collection<int32_t> r2r1 = Collection<int32_t>::newInstance(r2->maxDoc()); // r2 id to r1 id mapping

    TermDocsPtr termDocs1 = r1->termDocs();
    TermDocsPtr termDocs2 = r2->termDocs();

    // create mapping from id2 space to id2 based on idField
    TermEnumPtr termEnum = r1->terms(newLucene<Term>(idField, L""));

    do {
        TermPtr term = termEnum->term();
        if (!term || term->field() != idField) {
            break;
        }

        termDocs1->seek(termEnum);
        if (!termDocs1->next()) {
            // This doc is deleted and wasn't replaced
            termDocs2->seek(termEnum);
            EXPECT_TRUE(!termDocs2->next());
            continue;
        }

        int32_t id1 = termDocs1->doc();
        EXPECT_TRUE(!termDocs1->next());

        termDocs2->seek(termEnum);
        EXPECT_TRUE(termDocs2->next());
        int32_t id2 = termDocs2->doc();
        EXPECT_TRUE(!termDocs2->next());

        r2r1[id2] = id1;

        // verify stored fields are equivalent
        EXPECT_NO_THROW(verifyEquals(r1->document(id1), r2->document(id2)));

        // verify term vectors are equivalent
        EXPECT_NO_THROW(verifyEquals(r1->getTermFreqVectors(id1), r2->getTermFreqVectors(id2)));
    } while (termEnum->next());

    termEnum->close();

    // Verify postings
    TermEnumPtr termEnum1 = r1->terms(newLucene<Term>(L"", L""));
    TermEnumPtr termEnum2 = r2->terms(newLucene<Term>(L"", L""));

    // pack both doc and freq into single element for easy sorting
    Collection<int64_t> info1 = Collection<int64_t>::newInstance(r1->numDocs());
    Collection<int64_t> info2 = Collection<int64_t>::newInstance(r2->numDocs());

    while (true) {
        TermPtr term1;
        TermPtr term2;

        // iterate until we get some docs
        int32_t len1 = 0;
        while (true) {
            len1 = 0;
            term1 = termEnum1->term();
            if (!term1) {
                break;
            }
            termDocs1->seek(termEnum1);
            while (termDocs1->next()) {
                int32_t d1 = termDocs1->doc();
                int32_t f1 = termDocs1->freq();
                info1[len1] = (((int64_t)d1) << 32) | f1;
                len1++;
            }
            if (len1 > 0) {
                break;
            }
            if (!termEnum1->next()) {
                break;
            }
        }

        // iterate until we get some docs
        int32_t len2 = 0;
        while (true) {
            len2 = 0;
            term2 = termEnum2->term();
            if (!term2) {
                break;
            }
            termDocs2->seek(termEnum2);
            while (termDocs2->next()) {
                int32_t d2 = termDocs2->doc();
                int32_t f2 = termDocs2->freq();
                info2[len2] = (((int64_t)r2r1[d2]) << 32) | f2;
                len2++;
            }
            if (len2 > 0) {
                break;
            }
            if (!termEnum2->next()) {
                break;
            }
        }

        if (!hasDeletes) {
            EXPECT_EQ(termEnum1->docFreq(), termEnum2->docFreq());
        }

        EXPECT_EQ(len1, len2);
        if (len1 == 0) {
            break;    // no more terms
        }

        EXPECT_EQ(term1, term2);

        // sort info2 to get it into ascending docid
        std::sort(info2.begin(), info2.begin() + len2);

        // now compare
        for (int32_t i = 0; i < len1; ++i) {
            EXPECT_EQ(info1[i], info2[i]);
        }

        termEnum1->next();
        termEnum2->next();
    }
}

static void verifyEquals(const DocumentPtr& d1, const DocumentPtr& d2) {
    Collection<FieldablePtr> ff1 = d1->getFields();
    Collection<FieldablePtr> ff2 = d2->getFields();

    std::sort(ff1.begin(), ff1.end(), lessFieldName());
    std::sort(ff2.begin(), ff2.end(), lessFieldName());

    EXPECT_EQ(ff1.size(), ff2.size());

    for (int32_t i = 0; i < ff1.size(); ++i) {
        FieldablePtr f1 = ff1[i];
        FieldablePtr f2 = ff2[i];
        if (f1->isBinary()) {
            EXPECT_TRUE(f2->isBinary());
        } else {
            EXPECT_EQ(f1->stringValue(), f2->stringValue());
        }
    }
}

static void verifyEquals(Collection<TermFreqVectorPtr> d1, Collection<TermFreqVectorPtr> d2) {
    if (!d1) {
        EXPECT_TRUE(!d2);
        return;
    }

    EXPECT_TRUE(d2);

    EXPECT_EQ(d1.size(), d2.size());
    for (int32_t i = 0; i < d1.size(); ++i) {
        TermFreqVectorPtr v1 = d1[i];
        TermFreqVectorPtr v2 = d2[i];
        EXPECT_EQ(v1->size(), v2->size());
        int32_t numTerms = v1->size();
        Collection<String> terms1 = v1->getTerms();
        Collection<String> terms2 = v2->getTerms();
        Collection<int32_t> freq1 = v1->getTermFrequencies();
        Collection<int32_t> freq2 = v2->getTermFrequencies();
        for (int32_t j = 0; j < numTerms; ++j) {
            EXPECT_EQ(terms1[j], terms2[j]);
            EXPECT_EQ(freq1[j], freq2[j]);
        }
        if (boost::dynamic_pointer_cast<SegmentTermPositionVector>(v1)) {
            EXPECT_TRUE(boost::dynamic_pointer_cast<SegmentTermPositionVector>(v2));
            SegmentTermPositionVectorPtr tpv1 = boost::dynamic_pointer_cast<SegmentTermPositionVector>(v1);
            SegmentTermPositionVectorPtr tpv2 = boost::dynamic_pointer_cast<SegmentTermPositionVector>(v2);
            for (int32_t j = 0; j < numTerms; ++j) {
                Collection<int32_t> pos1 = tpv1->getTermPositions(j);
                Collection<int32_t> pos2 = tpv2->getTermPositions(j);
                EXPECT_EQ(pos1.size(), pos2.size());
                Collection<TermVectorOffsetInfoPtr> offsets1 = tpv1->getOffsets(j);
                Collection<TermVectorOffsetInfoPtr> offsets2 = tpv2->getOffsets(j);
                if (!offsets1) {
                    EXPECT_TRUE(!offsets2);
                } else {
                    EXPECT_TRUE(offsets2);
                }
                for (int32_t k = 0; k < pos1.size(); ++k) {
                    EXPECT_EQ(pos1[k], pos2[k]);
                    if (offsets1) {
                        EXPECT_EQ(offsets1[k]->getStartOffset(), offsets2[k]->getStartOffset());
                        EXPECT_EQ(offsets1[k]->getEndOffset(), offsets2[k]->getEndOffset());
                    }
                }
            }
        }
    }
}

namespace RunStressTest {

DECLARE_SHARED_PTR(StressTimedThread)
DECLARE_SHARED_PTR(StressIndexerThread)
DECLARE_SHARED_PTR(StressSearcherThread)

class StressTimedThread : public LuceneThread {
public:
    StressTimedThread() {
        this->failed = false;
        this->RUN_TIME_SEC = 6;
        this->rand = newLucene<Random>();
    }

    virtual ~StressTimedThread() {
    }

    LUCENE_CLASS(StressTimedThread);

public:
    bool failed;

protected:
    int32_t RUN_TIME_SEC;
    RandomPtr rand;

public:
    virtual void doWork() = 0;

    virtual void run() {
        int64_t stopTime = MiscUtils::currentTimeMillis() + 1000 * RUN_TIME_SEC;

        try {
            while ((int64_t)MiscUtils::currentTimeMillis() < stopTime && !failed) {
                doWork();
            }
        } catch (LuceneException& e) {
            failed = true;
            FAIL() << "Unexpected exception: " << e.getError();
        }
    }
};

class StressIndexerThread : public StressTimedThread {
public:
    StressIndexerThread(const IndexWriterPtr& writer) {
        this->writer = writer;
        this->nextID = 0;
    }

    virtual ~StressIndexerThread() {
    }

    LUCENE_CLASS(StressIndexerThread);

public:
    IndexWriterPtr writer;
    int32_t nextID;

public:
    virtual void doWork() {
        // Add 10 docs
        for (int32_t i = 0; i < 10; ++i) {
            DocumentPtr d = newLucene<Document>();
            d->add(newLucene<Field>(L"id", StringUtils::toString(nextID++), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
            d->add(newLucene<Field>(L"contents", intToEnglish(rand->nextInt()), Field::STORE_NO, Field::INDEX_ANALYZED));
            writer->addDocument(d);
        }

        // Delete 5 docs
        int32_t deleteID = nextID - 1;
        for (int32_t i = 0; i < 5; ++i) {
            writer->deleteDocuments(newLucene<Term>(L"id", StringUtils::toString(deleteID)));
            deleteID -= 2;
        }
    }
};

class StressSearcherThread : public StressTimedThread {
public:
    StressSearcherThread(const DirectoryPtr& directory) {
        this->directory = directory;
    }

    virtual ~StressSearcherThread() {
    }

    LUCENE_CLASS(StressSearcherThread);

protected:
    DirectoryPtr directory;

public:
    virtual void doWork() {
        for (int32_t i = 0; i < 100; ++i) {
            newLucene<IndexSearcher>(directory, true)->close();
        }
    }
};

}

/// Run one indexer and 2 searchers against single index as stress test.
static void runStressTest(const DirectoryPtr& directory, const MergeSchedulerPtr& mergeScheduler) {
    AnalyzerPtr analyzer = newLucene<SimpleAnalyzer>();
    IndexWriterPtr modifier = newLucene<IndexWriter>(directory, analyzer, true, IndexWriter::MaxFieldLengthUNLIMITED);

    modifier->setMaxBufferedDocs(10);

    Collection<RunStressTest::StressTimedThreadPtr> threads = Collection<RunStressTest::StressTimedThreadPtr>::newInstance(4);
    int32_t numThread = 0;

    if (mergeScheduler) {
        modifier->setMergeScheduler(mergeScheduler);
    }

    // One modifier that writes 10 docs then removes 5, over and over
    RunStressTest::StressIndexerThreadPtr indexerThread1 = newLucene<RunStressTest::StressIndexerThread>(modifier);
    threads[numThread++] = indexerThread1;
    indexerThread1->start();

    RunStressTest::StressIndexerThreadPtr indexerThread2 = newLucene<RunStressTest::StressIndexerThread>(modifier);
    threads[numThread++] = indexerThread2;
    indexerThread2->start();

    // Two searchers that constantly just re-instantiate the searcher
    RunStressTest::StressSearcherThreadPtr searcherThread1 = newLucene<RunStressTest::StressSearcherThread>(directory);
    threads[numThread++] = searcherThread1;
    searcherThread1->start();

    RunStressTest::StressSearcherThreadPtr searcherThread2 = newLucene<RunStressTest::StressSearcherThread>(directory);
    threads[numThread++] = searcherThread2;
    searcherThread2->start();

    for (int32_t i = 0; i < numThread; ++i) {
        threads[i]->join();
    }

    modifier->close();

    EXPECT_TRUE(!indexerThread1->failed); // hit unexpected exception in indexer1
    EXPECT_TRUE(!indexerThread2->failed); // hit unexpected exception in indexer2
    EXPECT_TRUE(!searcherThread1->failed); // hit unexpected exception in search1
    EXPECT_TRUE(!searcherThread2->failed); // hit unexpected exception in search2
}

TEST_F(StressIndexingTest, testStressIndexAndSearching) {
    // With ConcurrentMergeScheduler, in RAMDir
    DirectoryPtr directory = newLucene<MockRAMDirectory>();
    runStressTest(directory, newLucene<ConcurrentMergeScheduler>());
    directory->close();

    // With ConcurrentMergeScheduler, in FSDir
    String dirPath(FileUtils::joinPath(getTempDir(), L"lucene.test.stress"));
    directory = FSDirectory::open(dirPath);

    runStressTest(directory, newLucene<ConcurrentMergeScheduler>());
    directory->close();

    FileUtils::removeDirectory(dirPath);
}

TEST_F(StressIndexingTest, testRandomIWReader) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();

    DocsAndWriterPtr dw = indexRandomIWReader(10, 100, 100, dir);
    IndexReaderPtr r = dw->writer->getReader();
    dw->writer->commit();
    verifyEquals(r, dir, L"id");
    r->close();
    dw->writer->close();
    dir->close();
}

TEST_F(StressIndexingTest, testRandom) {
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    DirectoryPtr dir2 = newLucene<MockRAMDirectory>();

    HashMap<String, DocumentPtr> docs = indexRandom(10, 100, 100, dir1);
    indexSerial(docs, dir2);

    verifyEquals(dir1, dir2, L"id");
}

TEST_F(StressIndexingTest, testMultiConfig) {
    RandomPtr r = newLucene<Random>();
    // test lots of smaller different params together
    for (int32_t i = 0; i < 100; ++i) { // increase iterations for better testing
        sameFieldOrder = (r->nextInt() % 2 == 1);
        mergeFactor = r->nextInt(3) + 2;
        maxBufferedDocs = r->nextInt(3) + 2;
        seed++;

        int32_t numThreads = r->nextInt(5) + 1;
        int32_t iter = r->nextInt(10) + 1;
        int32_t range = r->nextInt(20) + 1;
        DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
        DirectoryPtr dir2 = newLucene<MockRAMDirectory>();
        HashMap<String, DocumentPtr> docs = indexRandom(numThreads, iter, range, dir1);
        indexSerial(docs, dir2);
        verifyEquals(dir1, dir2, L"id");
    }
}
