/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "Analyzer.h"
#include "LowerCaseTokenizer.h"
#include "TokenFilter.h"
#include "PayloadAttribute.h"
#include "TokenStream.h"
#include "Payload.h"
#include "Term.h"
#include "Document.h"
#include "Field.h"
#include "IndexReader.h"
#include "SegmentReader.h"
#include "SegmentTermPositions.h"
#include "IndexInput.h"

using namespace Lucene;

/// This testcase tests whether multi-level skipping is being used to reduce I/O while 
/// skipping through posting lists.  Skipping in general is already covered by 
/// several other testcases.
BOOST_FIXTURE_TEST_SUITE(MultiLevelSkipListTest, LuceneTestFixture)

class PayloadFilter : public TokenFilter
{
public:
    PayloadFilter(TokenStreamPtr input) : TokenFilter(input)
    {
        payloadAtt = addAttribute<PayloadAttribute>();
    }
    
    virtual ~PayloadFilter()
    {
    }
    
    LUCENE_CLASS(PayloadFilter);

public:
    static int32_t count;
    PayloadAttributePtr payloadAtt;
    
public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, ReaderPtr reader)
    {
        return newLucene<PayloadFilter>(newLucene<LowerCaseTokenizer>(reader));
    }
    
    virtual bool incrementToken()
    {
        bool hasNext = input->incrementToken();
        if (hasNext)
        {
            ByteArray data = ByteArray::newInstance(1);
            data[0] = (uint8_t)(count++);
            payloadAtt->setPayload(newLucene<Payload>(data));
        }
        return hasNext;
    }
};

int32_t PayloadFilter::count = 0;

class PayloadAnalyzer : public Analyzer
{
public:
    virtual ~PayloadAnalyzer()
    {
    }
    
    LUCENE_CLASS(PayloadAnalyzer);

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, ReaderPtr reader)
    {
        return newLucene<PayloadFilter>(newLucene<LowerCaseTokenizer>(reader));
    }
};

static int32_t counter = 0;

class CountingStream : public IndexInput
{
public:
    CountingStream(IndexInputPtr input)
    {
        this->input = input;
    }
    
    virtual ~CountingStream()
    {
    }
    
    LUCENE_CLASS(CountingStream);

protected:
    IndexInputPtr input;

public:
    virtual uint8_t readByte()
    {
        ++counter;
        return input->readByte();
    }
    
    virtual void readBytes(uint8_t* b, int32_t offset, int32_t length)
    {
        counter += length;
        input->readBytes(b, offset, length);
    }
    
    virtual void close()
    {
        input->close();
    }
    
    virtual int64_t getFilePointer()
    {
        return input->getFilePointer();
    }
    
    virtual void seek(int64_t pos)
    {
        input->seek(pos);
    }
    
    virtual int64_t length()
    {
        return input->length();
    }
    
    LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr())
    {
        return newLucene<CountingStream>(boost::dynamic_pointer_cast<IndexInput>(input->clone()));
    }
};

static void checkSkipTo(TermPositionsPtr tp, int32_t target, int32_t maxCounter)
{
    tp->skipTo(target);
    if (maxCounter < counter)
        BOOST_FAIL("Too many bytes read: " << counter);

    BOOST_CHECK_EQUAL(target, tp->doc());
    BOOST_CHECK_EQUAL(1, tp->freq());
    tp->nextPosition();
    ByteArray b = ByteArray::newInstance(1);
    tp->getPayload(b, 0);
    BOOST_CHECK_EQUAL((uint8_t)target, b[0]);
}

BOOST_AUTO_TEST_CASE(testSimpleSkip)
{
    DirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<PayloadAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    TermPtr term = newLucene<Term>(L"test", L"a");
    for (int32_t i = 0; i < 5000; ++i)
    {
        DocumentPtr d1 = newLucene<Document>();
        d1->add(newLucene<Field>(term->field(), term->text(), Field::STORE_NO, Field::INDEX_ANALYZED));
        writer->addDocument(d1);
    }
    
    writer->commit();
    writer->optimize();
    writer->close();

    IndexReaderPtr reader = SegmentReader::getOnlySegmentReader(dir);
    SegmentTermPositionsPtr tp = boost::dynamic_pointer_cast<SegmentTermPositions>(reader->termPositions());
    tp->freqStream(newLucene<CountingStream>(tp->freqStream()));

    for (int32_t i = 0; i < 2; ++i)
    {
        counter = 0;
        tp->seek(term);

        checkSkipTo(tp, 14, 185); // no skips
        checkSkipTo(tp, 17, 190); // one skip on level 0
        checkSkipTo(tp, 287, 200); // one skip on level 1, two on level 0

        // this test would fail if we had only one skip level, because than more bytes would be read from the freqStream
        checkSkipTo(tp, 4800, 250);// one skip on level 2
    }
}

BOOST_AUTO_TEST_SUITE_END()
