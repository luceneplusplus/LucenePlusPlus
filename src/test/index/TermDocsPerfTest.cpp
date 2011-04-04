/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "RAMDirectory.h"
#include "IndexReader.h"
#include "TermEnum.h"
#include "Term.h"
#include "TermDocs.h"
#include "TokenStream.h"
#include "TermAttribute.h"
#include "Analyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexWriter.h"
#include "Random.h"
#include "MiscUtils.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(TermDocsPerfTest, LuceneTestFixture)

DECLARE_SHARED_PTR(RepeatingTokenStream)

class RepeatingTokenStream : public TokenStream
{
public:
    RepeatingTokenStream(const String& val)
    {
        this->num = 0;
        this->value = val;
        this->termAtt = addAttribute<TermAttribute>();
    }
    
    virtual ~RepeatingTokenStream()
    {
    }
    
    LUCENE_CLASS(RepeatingTokenStream);
    
public:
    int32_t num;
    TermAttributePtr termAtt;
    String value;

public:
    virtual bool incrementToken()
    {
        --num;
        if (num >= 0)
        {
            clearAttributes();
            termAtt->setTermBuffer(value);
            return true;
        }
        return false;
    }
};

class TestAnalyzer : public Analyzer
{
public:
    TestAnalyzer(RepeatingTokenStreamPtr ts, RandomPtr random, int32_t maxTF, double percentDocs)
    {
        this->ts = ts;
        this->random = random;
        this->maxTF = maxTF;
        this->percentDocs = percentDocs;
    }
    
    virtual ~TestAnalyzer()
    {
    }
    
    LUCENE_CLASS(TestAnalyzer);

protected:
    RepeatingTokenStreamPtr ts;
    RandomPtr random;
    int32_t maxTF;
    double percentDocs;

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, ReaderPtr reader)
    {
        if (random->nextDouble() < percentDocs)
            ts->num = random->nextInt(maxTF) + 1;
        else
            ts->num = 0;
        return ts;
    }
};

static void addDocs(DirectoryPtr dir, int32_t numDocs, const String& field, const String& val, int32_t maxTF, double percentDocs)
{
    RepeatingTokenStreamPtr ts = newLucene<RepeatingTokenStream>(val);
    RandomPtr random = newLucene<Random>();
    AnalyzerPtr analyzer = newLucene<TestAnalyzer>(ts, random, maxTF, percentDocs);
        
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(field, val, Field::STORE_NO, Field::INDEX_NOT_ANALYZED_NO_NORMS));
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(100);
    writer->setMergeFactor(100);
    
    for (int32_t i = 0; i < numDocs; ++i)
        writer->addDocument(doc);
    
    writer->optimize();
    writer->close();
}

BOOST_AUTO_TEST_CASE(testTermDocsPerf)
{
    static const int32_t iter = 100000;
    static const int32_t numDocs = 10000;
    static const int32_t maxTF = 3;
    static const double percentDocs = 0.1;
    
    DirectoryPtr dir = newLucene<RAMDirectory>();

    int64_t start = MiscUtils::currentTimeMillis();
    addDocs(dir, numDocs, L"foo", L"val", maxTF, percentDocs);
    int64_t end = MiscUtils::currentTimeMillis();
    
    BOOST_TEST_MESSAGE("Milliseconds for creation of " << numDocs << " docs = " << (end - start));

    IndexReaderPtr reader = IndexReader::open(dir, true);
    TermEnumPtr termEnum = reader->terms(newLucene<Term>(L"foo", L"val"));
    TermDocsPtr termDocs = reader->termDocs();

    start = MiscUtils::currentTimeMillis();

    for (int32_t i = 0; i < iter; ++i)
    {
        termDocs->seek(termEnum);
        while (termDocs->next())
            termDocs->doc();
    }
    
    end = MiscUtils::currentTimeMillis();
    BOOST_TEST_MESSAGE("Milliseconds for " << iter << " TermDocs iteration: " << (end - start));
}

BOOST_AUTO_TEST_SUITE_END()
