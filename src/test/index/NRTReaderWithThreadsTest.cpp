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
#include "WhitespaceAnalyzer.h"
#include "IndexReader.h"
#include "LuceneThread.h"
#include "Document.h"
#include "Term.h"
#include "Field.h"
#include "TermDocs.h"
#include "Random.h"
#include "MiscUtils.h"

using namespace Lucene;

typedef LuceneTestFixture NRTReaderWithThreadsTest;

DECLARE_SHARED_PTR(RunThread)
DECLARE_SHARED_PTR(HeavyAtomicInt)

static DocumentPtr createDocument(int32_t n, const String& indexName, int32_t numFields) {
    StringStream sb;
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"id", StringUtils::toString(n), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    doc->add(newLucene<Field>(L"indexname", indexName, Field::STORE_YES, Field::INDEX_NOT_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    sb << L"a" << n;
    doc->add(newLucene<Field>(L"field1", sb.str(), Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    sb << L" b" << n;
    for (int32_t i = 1; i < numFields; ++i) {
        doc->add(newLucene<Field>(L"field" + StringUtils::toString(i + 1), sb.str(), Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    }
    return doc;
}

static int32_t count(const TermPtr& t, const IndexReaderPtr& r) {
    int32_t count = 0;
    TermDocsPtr td = r->termDocs(t);
    while (td->next()) {
        td->doc();
        ++count;
    }
    td->close();
    return count;
}

class HeavyAtomicInt : public LuceneObject {
public:
    HeavyAtomicInt(int32_t start) {
        value = start;
    }

    virtual ~HeavyAtomicInt() {

    }

protected:
    int32_t value;

public:
    int32_t addAndGet(int32_t inc) {
        SyncLock syncLock(this);
        value += inc;
        return value;
    }

    int32_t incrementAndGet() {
        SyncLock syncLock(this);
        return ++value;
    }

    int32_t intValue() {
        SyncLock syncLock(this);
        return value;
    }
};

class RunThread : public LuceneThread {
public:
    RunThread(int32_t type, const IndexWriterPtr& writer, const HeavyAtomicIntPtr& seq) {
        this->_run = true;
        this->delCount = 0;
        this->addCount = 0;
        this->type = type;
        this->writer = writer;
        this->seq = seq;
        this->rand = newLucene<Random>();
    }

    virtual ~RunThread() {
    }

    LUCENE_CLASS(RunThread);

public:
    HeavyAtomicIntPtr seq;
    IndexWriterPtr writer;
    bool _run;
    int32_t delCount;
    int32_t addCount;
    int32_t type;
    RandomPtr rand;

public:
    virtual void run() {
        try {
            while (_run) {
                if (type == 0) {
                    int32_t i = seq->addAndGet(1);
                    DocumentPtr doc = createDocument(i, L"index1", 10);
                    writer->addDocument(doc);
                    ++addCount;
                } else {
                    // we may or may not delete because the term may not exist,
                    // however we're opening and closing the reader rapidly
                    IndexReaderPtr reader = writer->getReader();
                    int32_t id = rand->nextInt(seq->intValue());
                    TermPtr term = newLucene<Term>(L"id", StringUtils::toString(id));
                    int32_t _count = count(term, reader);
                    writer->deleteDocuments(term);
                    reader->close();
                    delCount += _count;
                }
            }
        } catch (LuceneException& e) {
            _run = false;
            FAIL() << "Unexpected exception: " << e.getError();
        }
    }
};

TEST_F(NRTReaderWithThreadsTest, testIndexing) {
    HeavyAtomicIntPtr seq = newLucene<HeavyAtomicInt>(1);
    DirectoryPtr mainDir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(mainDir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    writer->setUseCompoundFile(false);
    IndexReaderPtr reader = writer->getReader(); // start pooling readers
    reader->close();
    writer->setMergeFactor(2);
    writer->setMaxBufferedDocs(10);
    Collection<RunThreadPtr> indexThreads = Collection<RunThreadPtr>::newInstance(4);
    for (int32_t x = 0; x < indexThreads.size(); ++x) {
        indexThreads[x] = newLucene<RunThread>(x % 2, writer, seq);
        indexThreads[x]->start();
    }
    int64_t startTime = MiscUtils::currentTimeMillis();
    int64_t duration = 5 * 1000;
    while (((int64_t)MiscUtils::currentTimeMillis() - startTime) < duration) {
        LuceneThread::threadSleep(100);
    }
    int32_t delCount = 0;
    int32_t addCount = 0;
    for (int32_t x = 0; x < indexThreads.size(); ++x) {
        indexThreads[x]->_run = false;
        addCount += indexThreads[x]->addCount;
        delCount += indexThreads[x]->delCount;
    }
    for (int32_t x = 0; x < indexThreads.size(); ++x) {
        indexThreads[x]->join();
    }
    writer->close();
    mainDir->close();
}
