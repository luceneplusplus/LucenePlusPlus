/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "SimpleAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "TokenStream.h"
#include "TermAttribute.h"
#include "OffsetAttribute.h"
#include "CachingTokenFilter.h"
#include "IndexReader.h"
#include "TermPositions.h"
#include "Term.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(CachingTokenFilterTest, BaseTokenStreamFixture)

static Collection<String> tokens = newCollection<String>(L"term1", L"term2", L"term3", L"term2");

static void checkTokens(TokenStreamPtr stream)
{
    int32_t count = 0;

    TermAttributePtr termAtt = stream->getAttribute<TermAttribute>();
    BOOST_CHECK(termAtt);
    while (stream->incrementToken())
    {
        BOOST_CHECK(count < tokens.size());
        BOOST_CHECK_EQUAL(tokens[count], termAtt->term());
        ++count;
    }
    BOOST_CHECK_EQUAL(tokens.size(), count);
}

namespace TestCaching
{
    class TestableTokenStream : public TokenStream
    {
    public:
        TestableTokenStream()
        {
            index = 0;
            termAtt = addAttribute<TermAttribute>();
            offsetAtt = addAttribute<OffsetAttribute>();
        }
        
        virtual ~TestableTokenStream()
        {
        }
    
    protected:
        int32_t index;
        TermAttributePtr termAtt;
        OffsetAttributePtr offsetAtt;
    
    public:
        virtual bool incrementToken()
        {
            if (index == tokens.size())
                return false;
            else
            {
                clearAttributes();
                termAtt->setTermBuffer(tokens[index++]);
                offsetAtt->setOffset(0, 0);
                return true;
            }
        }
    };
}

BOOST_AUTO_TEST_CASE(testCaching)
{
    DirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<SimpleAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    TokenStreamPtr stream = newLucene<CachingTokenFilter>(newLucene<TestCaching::TestableTokenStream>());

    doc->add(newLucene<Field>(L"preanalyzed", stream, Field::TERM_VECTOR_NO));

    // 1) we consume all tokens twice before we add the doc to the index
    checkTokens(stream);
    stream->reset();  
    checkTokens(stream);

    // 2) now add the document to the index and verify if all tokens are indexed don't reset the stream here, the 
    // DocumentWriter should do that implicitly
    writer->addDocument(doc);
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    TermPositionsPtr termPositions = reader->termPositions(newLucene<Term>(L"preanalyzed", L"term1"));
    BOOST_CHECK(termPositions->next());
    BOOST_CHECK_EQUAL(1, termPositions->freq());
    BOOST_CHECK_EQUAL(0, termPositions->nextPosition());

    termPositions->seek(newLucene<Term>(L"preanalyzed", L"term2"));
    BOOST_CHECK(termPositions->next());
    BOOST_CHECK_EQUAL(2, termPositions->freq());
    BOOST_CHECK_EQUAL(1, termPositions->nextPosition());
    BOOST_CHECK_EQUAL(3, termPositions->nextPosition());

    termPositions->seek(newLucene<Term>(L"preanalyzed", L"term3"));
    BOOST_CHECK(termPositions->next());
    BOOST_CHECK_EQUAL(1, termPositions->freq());
    BOOST_CHECK_EQUAL(2, termPositions->nextPosition());
    reader->close();

    // 3) reset stream and consume tokens again
    stream->reset();
    checkTokens(stream);
}

BOOST_AUTO_TEST_SUITE_END()
