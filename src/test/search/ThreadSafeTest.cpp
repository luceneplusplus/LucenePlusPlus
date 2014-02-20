/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <boost/algorithm/string.hpp>
#include "LuceneTestFixture.h"
#include "LuceneThread.h"
#include "IndexReader.h"
#include "Random.h"
#include "FieldSelector.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"

using namespace Lucene;

class TestFieldSelector : public FieldSelector {
public:
    TestFieldSelector(const RandomPtr& rand) {
        this->rand = rand;
    }

    virtual ~TestFieldSelector() {
    }

protected:
    RandomPtr rand;

public:
    virtual FieldSelectorResult accept(const String& fieldName) {
        switch (rand->nextInt(2)) {
        case 0:
            return FieldSelector::SELECTOR_LAZY_LOAD;
        case 1:
            return FieldSelector::SELECTOR_LOAD;
        default:
            return FieldSelector::SELECTOR_LOAD;
        }
    }
};

class TestThread : public LuceneThread {
public:
    TestThread(int32_t iter, const RandomPtr& rand, const IndexReaderPtr& reader) {
        this->iter = iter;
        this->rand = rand;
        this->reader = reader;
    }

    virtual ~TestThread() {
    }

    LUCENE_CLASS(TestThread);

protected:
    IndexReaderPtr reader;
    int32_t iter;
    RandomPtr rand;

public:
    virtual void run() {
        try {
            for (int32_t i = 0; i < iter; ++i) {
                loadDoc();
            }
        } catch (LuceneException& e) {
            FAIL() << "Unexpected exception: " << e.getError();
        }
    }

    void loadDoc() {
        DocumentPtr doc = reader->document(rand->nextInt(reader->maxDoc()), newLucene<TestFieldSelector>(rand));
        Collection<FieldablePtr> fields = doc->getFields();
        for (int32_t i = 0; i < fields.size(); ++i) {
            validateField(fields[i]);
        }
    }

    void validateField(const FieldablePtr& f) {
        String val = f->stringValue();
        if (!boost::starts_with(val, L"^") || !boost::ends_with(val, L"$")) {
            FAIL() << "Invalid field";
        }
    }
};

class ThreadSafeTest : public LuceneTestFixture {
public:
    ThreadSafeTest() {
        r = newLucene<Random>(17);
        dir = newLucene<RAMDirectory>();
        words = StringUtils::split(L"now is the time for all good men to come to the aid of their country", L" ");
    }

    virtual ~ThreadSafeTest() {
    }

public:
    RandomPtr r;
    DirectoryPtr dir;
    IndexReaderPtr reader;
    Collection<String> words;

public:
    void buildDir(const DirectoryPtr& dir, int32_t numDocs, int32_t maxFields, int32_t maxFieldLen) {
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        writer->setMaxBufferedDocs(10);
        for (int32_t j = 0; j < numDocs; ++j) {
            DocumentPtr doc = newLucene<Document>();
            int32_t numFields = r->nextInt(maxFields);
            for (int32_t i = 0; i < numFields; ++i) {
                int32_t flen = r->nextInt(maxFieldLen);
                StringStream buf;
                buf << L"^ ";
                while ((int32_t)buf.str().length() < flen) {
                    buf << L" " << words[r->nextInt(words.size())];
                }
                buf << L" $";
                doc->add(newLucene<Field>(L"f" + StringUtils::toString(i), buf.str(), Field::STORE_YES, Field::INDEX_ANALYZED));
            }
            writer->addDocument(doc);
        }
        writer->close();
    }

    void doTest(int32_t iter, int32_t numThreads) {
        Collection<LuceneThreadPtr> threads = Collection<LuceneThreadPtr>::newInstance(numThreads);
        for (int32_t i = 0; i < numThreads; ++i) {
            threads[i] = newLucene<TestThread>(iter, newLucene<Random>(r->nextInt()), reader);
            threads[i]->start();
        }
        for (int32_t i = 0; i < numThreads; ++i) {
            threads[i]->join();
        }
    }
};

TEST_F(ThreadSafeTest, testLazyLoadThreadSafety) {
    // test with field sizes bigger than the buffer of an index input
    buildDir(dir, 15, 5, 2000);

    // do many small tests so the thread locals go away in between
    for (int32_t i = 0; i < 100; ++i) {
        reader = IndexReader::open(dir, false);
        doTest(10, 100);
    }
}
