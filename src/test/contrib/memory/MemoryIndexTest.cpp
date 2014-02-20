/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <boost/algorithm/string.hpp>
#include "TestUtils.h"
#include "BaseTokenStreamFixture.h"
#include "BufferedReader.h"
#include "FileReader.h"
#include "StopAnalyzer.h"
#include "SimpleAnalyzer.h"
#include "StandardAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "MemoryIndex.h"
#include "IndexSearcher.h"
#include "TermDocs.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "QueryParser.h"
#include "TopDocs.h"
#include "Random.h"
#include "FileUtils.h"

using namespace Lucene;

/// Verifies that Lucene MemoryIndex and RAMDirectory have the same behaviour,
/// returning the same results for queries on some randomish indexes.
class MemoryIndexTest : public BaseTokenStreamFixture {
public:
    MemoryIndexTest() {
        fileDir = FileUtils::joinPath(getTestDir(), L"memory");
        queries = HashSet<String>::newInstance();
        HashSet<String> test1 = readQueries(L"testqueries.txt");
        queries.addAll(test1.begin(), test1.end());
        HashSet<String> test2 = readQueries(L"testqueries2.txt");
        queries.addAll(test2.begin(), test2.end());
        random = newLucene<Random>(123);
        buffer = CharArray::newInstance(20);

        /// Some terms to be indexed, in addition to random words.
        /// These terms are commonly used in the queries.
        TEST_TERMS = Collection<String>::newInstance();
        TEST_TERMS.add(L"term");
        TEST_TERMS.add(L"tErm");
        TEST_TERMS.add(L"TERM");
        TEST_TERMS.add(L"telm");
        TEST_TERMS.add(L"stop");
        TEST_TERMS.add(L"drop");
        TEST_TERMS.add(L"roll");
        TEST_TERMS.add(L"phrase");
        TEST_TERMS.add(L"a");
        TEST_TERMS.add(L"c");
        TEST_TERMS.add(L"bar");
        TEST_TERMS.add(L"blar");
        TEST_TERMS.add(L"gack");
        TEST_TERMS.add(L"weltbank");
        TEST_TERMS.add(L"worlbank");
        TEST_TERMS.add(L"hello");
        TEST_TERMS.add(L"on");
        TEST_TERMS.add(L"the");
        TEST_TERMS.add(L"apache");
        TEST_TERMS.add(L"Apache");
        TEST_TERMS.add(L"copyright");
        TEST_TERMS.add(L"Copyright");
    }

    virtual ~MemoryIndexTest() {
    }

protected:
    String fileDir;
    HashSet<String> queries;
    RandomPtr random;
    CharArray buffer;

    static const int32_t ITERATIONS;
    Collection<String> TEST_TERMS;

public:
    /// read a set of queries from a resource file
    HashSet<String> readQueries(const String& resource) {
        HashSet<String> queries = HashSet<String>::newInstance();
        BufferedReaderPtr reader = newLucene<BufferedReader>(newLucene<FileReader>(FileUtils::joinPath(fileDir, resource)));
        String line;
        while (reader->readLine(line)) {
            boost::trim(line);
            if (!line.empty() && !boost::starts_with(line, L"#") && !boost::starts_with(line, L"//")) {
                queries.add(line);
            }
        }
        reader->close();

        return queries;
    }

    /// Build a randomish document for both RAMDirectory and MemoryIndex, and run all the queries against it.
    void checkAgainstRAMDirectory() {
        StringStream fooField;
        StringStream termField;

        // add up to 250 terms to field "foo"
        int32_t fieldCount = random->nextInt(250) + 1;
        for (int32_t i = 0; i < fieldCount; ++i) {
            fooField << L" " << randomTerm();
        }

        // add up to 250 terms to field "foo"
        int32_t termCount = random->nextInt(250) + 1;
        for (int32_t i = 0; i < termCount; ++i) {
            termField << L" " << randomTerm();
        }

        RAMDirectoryPtr ramdir = newLucene<RAMDirectory>();
        AnalyzerPtr analyzer = randomAnalyzer();
        IndexWriterPtr writer = newLucene<IndexWriter>(ramdir, analyzer, IndexWriter::MaxFieldLengthUNLIMITED);
        DocumentPtr doc = newLucene<Document>();
        FieldPtr field1 = newLucene<Field>(L"foo", fooField.str(), Field::STORE_NO, Field::INDEX_ANALYZED);
        FieldPtr field2 = newLucene<Field>(L"term", termField.str(), Field::STORE_NO, Field::INDEX_ANALYZED);
        doc->add(field1);
        doc->add(field2);
        writer->addDocument(doc);
        writer->close();

        MemoryIndexPtr memory = newLucene<MemoryIndex>();
        memory->addField(L"foo", fooField.str(), analyzer);
        memory->addField(L"term", termField.str(), analyzer);
        checkAllQueries(memory, ramdir, analyzer);
    }

    void checkAllQueries(const MemoryIndexPtr& memory, const RAMDirectoryPtr& ramdir, const AnalyzerPtr& analyzer) {
        IndexSearcherPtr ram = newLucene<IndexSearcher>(ramdir);
        IndexSearcherPtr mem = memory->createSearcher();
        QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"foo", analyzer);
        for (HashSet<String>::iterator query = queries.begin(); query != queries.end(); ++query) {
            TopDocsPtr ramDocs = ram->search(qp->parse(*query), 1);
            TopDocsPtr memDocs = mem->search(qp->parse(*query), 1);
            EXPECT_EQ(ramDocs->totalHits, memDocs->totalHits);
        }
    }

    AnalyzerPtr randomAnalyzer() {
        switch (random->nextInt(3)) {
        case 0:
            return newLucene<SimpleAnalyzer>();
        case 1:
            return newLucene<StopAnalyzer>(LuceneVersion::LUCENE_CURRENT);
        default:
            return newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
        }
    }

    /// half of the time, returns a random term from TEST_TERMS.
    /// the other half of the time, returns a random unicode string.
    String randomTerm() {
        if (random->nextInt() % 2 == 1) {
            // return a random TEST_TERM
            return TEST_TERMS[random->nextInt(TEST_TERMS.size())];
        } else {
            // return a random unicode term
            return randomString();
        }
    }

    /// Return a random unicode term, like StressIndexingTest.
    String randomString() {
        int32_t end = random->nextInt(20);
        if (buffer.size() < 1 + end) {
            buffer.resize((int32_t)((double)(1 + end) * 1.25));
        }

        for (int32_t i = 0; i < end; ++i) {
            int32_t t = random->nextInt(5);
            if (t == 0 && i < end - 1) {
#ifdef LPP_UNICODE_CHAR_SIZE_2
                // Make a surrogate pair
                // High surrogate
                buffer[i++] = (wchar_t)nextInt(0xd800, 0xdc00);
                // Low surrogate
                buffer[i] = (wchar_t)nextInt(0xdc00, 0xe000);
#else
                buffer[i] = (wchar_t)nextInt(0xdc00, 0xe000);
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
        return String(buffer.get(), end);
    }

    /// start is inclusive and end is exclusive
    int32_t nextInt(int32_t start, int32_t end) {
        return start + random->nextInt(end - start);
    }
};

const int32_t MemoryIndexTest::ITERATIONS = 100;

/// runs random tests, up to ITERATIONS times.
TEST_F(MemoryIndexTest, testRandomQueries) {
    for (int32_t i = 0; i < ITERATIONS; ++i) {
        checkAgainstRAMDirectory();
    }
}
