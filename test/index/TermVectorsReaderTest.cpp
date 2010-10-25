/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "TermVectorOffsetInfo.h"
#include "MockRAMDirectory.h"
#include "FieldInfos.h"
#include "IndexWriter.h"
#include "Analyzer.h"
#include "TokenStream.h"
#include "TermAttribute.h"
#include "PositionIncrementAttribute.h"
#include "OffsetAttribute.h"
#include "Document.h"
#include "Field.h"
#include "IndexFileNames.h"
#include "TermVectorsReader.h"
#include "TermFreqVector.h"
#include "SegmentInfo.h"
#include "TermPositionVector.h"
#include "SegmentTermVector.h"
#include "SortedTermVectorMapper.h"
#include "TermVectorEntryFreqSortedComparator.h"
#include "FieldSortedTermVectorMapper.h"
#include "TermVectorEntry.h"
#include "IndexReader.h"
#include "Random.h"

using namespace Lucene;

DECLARE_SHARED_PTR(TestToken)

class TestToken : public LuceneObject
{
public:
    TestToken()
    {
        pos = 0;
        startOffset = 0;
        endOffset = 0;
    }
    
    virtual ~TestToken()
    {
    }
    
    LUCENE_CLASS(TestToken);

public:
    String text;
    int32_t pos;
    int32_t startOffset;
    int32_t endOffset;

public:
    int32_t compareTo(TestTokenPtr other)
    {
        return (pos - other->pos);
    }
};

class MyTokenStream : public TokenStream
{
public:
    MyTokenStream(Collection<TestTokenPtr> tokens)
    {
        this->tokens = tokens;
        
        tokenUpto = 0;
        termAtt = addAttribute<TermAttribute>();
        posIncrAtt = addAttribute<PositionIncrementAttribute>();
        offsetAtt = addAttribute<OffsetAttribute>();
    }
    
    virtual ~MyTokenStream()
    {
    }
    
    LUCENE_CLASS(MyTokenStream);

protected:
    Collection<TestTokenPtr> tokens;

public:
    int32_t tokenUpto;

    TermAttributePtr termAtt;
    PositionIncrementAttributePtr posIncrAtt;
    OffsetAttributePtr offsetAtt;

public:
    virtual bool incrementToken()
    {
        if (tokenUpto >= tokens.size())
            return false;
        else
        {
            TestTokenPtr testToken = tokens[tokenUpto++];
            clearAttributes();
            termAtt->setTermBuffer(testToken->text);
            offsetAtt->setOffset(testToken->startOffset, testToken->endOffset);
            if (tokenUpto > 1)
                posIncrAtt->setPositionIncrement(testToken->pos - tokens[tokenUpto - 2]->pos);
            else
                posIncrAtt->setPositionIncrement(testToken->pos + 1);
        }
        return true;
    }
};

class MyAnalyzer : public Analyzer
{
public:
    MyAnalyzer(Collection<TestTokenPtr> tokens)
    {
        this->tokens = tokens;
    }
    
    virtual ~MyAnalyzer()
    {
    }
    
    LUCENE_CLASS(MyAnalyzer);

protected:
    Collection<TestTokenPtr> tokens;
    
public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, ReaderPtr reader)
    {
        return newLucene<MyTokenStream>(tokens);
    }
};

DECLARE_SHARED_PTR(DocNumAwareMapper)

class DocNumAwareMapper : public TermVectorMapper
{
public:
    DocNumAwareMapper()
    {
        documentNumber = -1;
    }
    
    virtual ~DocNumAwareMapper()
    {
    }
    
    LUCENE_CLASS(DocNumAwareMapper);

protected:
    int32_t documentNumber;
    
public:
    virtual void setExpectations(const String& field, int32_t numTerms, bool storeOffsets, bool storePositions)
    {
        if (documentNumber == -1)
            BOOST_FAIL("Documentnumber should be set at this point!");
    }
    
    virtual void map(const String& term, int32_t frequency, Collection<TermVectorOffsetInfoPtr> offsets, Collection<int32_t> positions)
    {
        if (documentNumber == -1)
            BOOST_FAIL("Documentnumber should be set at this point!");
    }
    
    virtual int32_t getDocumentNumber()
    {
        return documentNumber;
    }
    
    virtual void setDocumentNumber(int32_t documentNumber)
    {
        this->documentNumber = documentNumber;
    }
};

class TermVectorsReaderTestFixture : public LuceneTestFixture
{
public:
    TermVectorsReaderTestFixture()
    {
        // Must be lexicographically sorted, will do in setup, versus trying to maintain here
        testFields = newCollection<String>(L"f1", L"f2", L"f3", L"f4");
        testFieldsStorePos = newCollection<uint8_t>(true, false, true, false);
        testFieldsStoreOff = newCollection<uint8_t>(true, false, false, true);
        testTerms = newCollection<String>(L"this", L"is", L"a", L"test");
        
        positions = Collection< Collection<int32_t> >::newInstance(testTerms.size());
        offsets = Collection< Collection<TermVectorOffsetInfoPtr> >::newInstance(testTerms.size());
        dir = newLucene<MockRAMDirectory>();
        tokens = Collection<TestTokenPtr>::newInstance(testTerms.size() * TERM_FREQ);
        RandomPtr random = newLucene<Random>();
        
        std::sort(testTerms.begin(), testTerms.end());
        int32_t tokenUpto = 0;
        for (int32_t i = 0; i < testTerms.size(); ++i)
        {
            positions[i] = Collection<int32_t>::newInstance(TERM_FREQ);
            offsets[i] = Collection<TermVectorOffsetInfoPtr>::newInstance(TERM_FREQ);
            // first position must be 0
            for (int32_t j = 0; j < TERM_FREQ; ++j)
            {
                // positions are always sorted in increasing order
                positions[i][j] = (int32_t)(j * 10 + (int32_t)((double)random->nextInt(100) / 100.0) * 10);
                // offsets are always sorted in increasing order
                offsets[i][j] = newLucene<TermVectorOffsetInfo>(j * 10, j * 10 + testTerms[i].size());
                TestTokenPtr token = newLucene<TestToken>();
                tokens[tokenUpto++] = token;
                token->text = testTerms[i];
                token->pos = positions[i][j];
                token->startOffset = offsets[i][j]->getStartOffset();
                token->endOffset = offsets[i][j]->getEndOffset();
            }
        }
        std::sort(tokens.begin(), tokens.end(), luceneCompare<TestTokenPtr>());
        
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<MyAnalyzer>(tokens), true, IndexWriter::MaxFieldLengthLIMITED);
        writer->setUseCompoundFile(false);
        DocumentPtr doc = newLucene<Document>();
        for (int32_t i = 0; i < testFields.size(); ++i)
        {
            Field::TermVector tv = Field::TERM_VECTOR_YES;
            if (testFieldsStorePos[i] && testFieldsStoreOff[i])
                tv = Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS;
            else if (testFieldsStorePos[i] && !testFieldsStoreOff[i])
                tv = Field::TERM_VECTOR_WITH_POSITIONS;
            else if (!testFieldsStorePos[i] && testFieldsStoreOff[i])
                tv = Field::TERM_VECTOR_WITH_OFFSETS;
            doc->add(newLucene<Field>(testFields[i], L"", Field::STORE_NO, Field::INDEX_ANALYZED, tv));
        }
        
        // Create 5 documents for testing, they all have the same terms
        for (int32_t j = 0; j < 5; ++j)
            writer->addDocument(doc);
        
        writer->commit();
        seg = writer->newestSegment()->name;
        writer->close();

        fieldInfos = newLucene<FieldInfos>(dir, seg + L"." + IndexFileNames::FIELD_INFOS_EXTENSION());
    }
    
    virtual ~TermVectorsReaderTestFixture()
    {
    }

protected:
    Collection<String> testFields;
    Collection<uint8_t> testFieldsStorePos;
    Collection<uint8_t> testFieldsStoreOff;
    Collection<String> testTerms;
    Collection< Collection<int32_t> > positions;
    Collection< Collection<TermVectorOffsetInfoPtr> > offsets;
    MockRAMDirectoryPtr dir;
    String seg;
    FieldInfosPtr fieldInfos;
    Collection<TestTokenPtr> tokens;
    
    static const int32_t TERM_FREQ;
};

const int32_t TermVectorsReaderTestFixture::TERM_FREQ = 3;

BOOST_FIXTURE_TEST_SUITE(TermVectorsReaderTest, TermVectorsReaderTestFixture)

BOOST_AUTO_TEST_CASE(testReader)
{
    // Check to see the files were created properly in setup
    BOOST_CHECK(dir->fileExists(seg + L"." + IndexFileNames::VECTORS_DOCUMENTS_EXTENSION()));
    BOOST_CHECK(dir->fileExists(seg + L"." + IndexFileNames::VECTORS_INDEX_EXTENSION()));

    TermVectorsReaderPtr reader = newLucene<TermVectorsReader>(dir, seg, fieldInfos);
    BOOST_CHECK(reader);
    
    for (int32_t j = 0; j < 5; ++j)
    {
        TermFreqVectorPtr vector = reader->get(j, testFields[0]);
        BOOST_CHECK(vector);
        Collection<String> terms = vector->getTerms();
        BOOST_CHECK(terms);
        BOOST_CHECK_EQUAL(terms.size(), testTerms.size());
        for (int32_t i = 0; i < terms.size(); ++i)
            BOOST_CHECK_EQUAL(terms[i], testTerms[i]);
    }
}

BOOST_AUTO_TEST_CASE(testPositionReader)
{
    TermVectorsReaderPtr reader = newLucene<TermVectorsReader>(dir, seg, fieldInfos);
    BOOST_CHECK(reader);
    TermPositionVectorPtr vector = boost::dynamic_pointer_cast<TermPositionVector>(reader->get(0, testFields[0]));
    BOOST_CHECK(vector);
    Collection<String> terms = vector->getTerms();
    BOOST_CHECK(terms);
    BOOST_CHECK_EQUAL(terms.size(), testTerms.size());
    for (int32_t i = 0; i < terms.size(); ++i)
    {
        String term = terms[i];
        BOOST_CHECK_EQUAL(term, testTerms[i]);
        Collection<int32_t> positions = vector->getTermPositions(i);
        BOOST_CHECK(positions);
        BOOST_CHECK_EQUAL(positions.size(), this->positions[i].size());
        for (int32_t j = 0; j < positions.size(); ++j)
            BOOST_CHECK_EQUAL(positions[j], this->positions[i][j]);
        Collection<TermVectorOffsetInfoPtr> offset = vector->getOffsets(i);
        BOOST_CHECK(offset);
        BOOST_CHECK_EQUAL(offset.size(), this->offsets[i].size());
        for (int32_t j = 0; j < offset.size(); ++j)
        {
            TermVectorOffsetInfoPtr termVectorOffsetInfo = offset[j];
            BOOST_CHECK(termVectorOffsetInfo->equals(offsets[i][j]));
        }
    }

    TermFreqVectorPtr freqVector = reader->get(0, testFields[1]); // no pos, no offset
    BOOST_CHECK(freqVector);
    BOOST_CHECK(boost::dynamic_pointer_cast<SegmentTermVector>(freqVector));
    terms = freqVector->getTerms();
    BOOST_CHECK(terms);
    BOOST_CHECK_EQUAL(terms.size(), testTerms.size());
    for (int32_t i = 0; i < terms.size(); ++i)
        BOOST_CHECK_EQUAL(terms[i], testTerms[i]);
}

BOOST_AUTO_TEST_CASE(testOffsetReader)
{
    TermVectorsReaderPtr reader = newLucene<TermVectorsReader>(dir, seg, fieldInfos);
    BOOST_CHECK(reader);
    TermPositionVectorPtr vector = boost::dynamic_pointer_cast<TermPositionVector>(reader->get(0, testFields[0]));
    BOOST_CHECK(vector);
    Collection<String> terms = vector->getTerms();
    BOOST_CHECK(terms);
    BOOST_CHECK_EQUAL(terms.size(), testTerms.size());
    for (int32_t i = 0; i < terms.size(); ++i)
    {
        String term = terms[i];
        BOOST_CHECK_EQUAL(term, testTerms[i]);
        Collection<int32_t> positions = vector->getTermPositions(i);
        BOOST_CHECK(positions);
        BOOST_CHECK_EQUAL(positions.size(), this->positions[i].size());
        for (int32_t j = 0; j < positions.size(); ++j)
            BOOST_CHECK_EQUAL(positions[j], this->positions[i][j]);
        Collection<TermVectorOffsetInfoPtr> offset = vector->getOffsets(i);
        BOOST_CHECK(offset);
        BOOST_CHECK_EQUAL(offset.size(), this->offsets[i].size());
        for (int32_t j = 0; j < offset.size(); ++j)
        {
            TermVectorOffsetInfoPtr termVectorOffsetInfo = offset[j];
            BOOST_CHECK(termVectorOffsetInfo->equals(offsets[i][j]));
        }
    }
}

BOOST_AUTO_TEST_CASE(testMapper)
{
    TermVectorsReaderPtr reader = newLucene<TermVectorsReader>(dir, seg, fieldInfos);
    BOOST_CHECK(reader);
    SortedTermVectorMapperPtr mapper = newLucene<SortedTermVectorMapper>(TermVectorEntryFreqSortedComparator::compare);
    reader->get(0, mapper);
    Collection<TermVectorEntryPtr> entrySet = mapper->getTermVectorEntrySet();
    BOOST_CHECK(entrySet);
    // three fields, 4 terms, all terms are the same
    BOOST_CHECK_EQUAL(entrySet.size(), 4);
    // check offsets and positions
    for (Collection<TermVectorEntryPtr>::iterator tve = entrySet.begin(); tve != entrySet.end(); ++tve)
    {
        BOOST_CHECK(*tve);
        BOOST_CHECK((*tve)->getOffsets());
        BOOST_CHECK((*tve)->getPositions());
    }
    
    mapper = newLucene<SortedTermVectorMapper>(TermVectorEntryFreqSortedComparator::compare);
    reader->get(1, mapper);
    entrySet = mapper->getTermVectorEntrySet();
    BOOST_CHECK(entrySet);
    // three fields, 4 terms, all terms are the same
    BOOST_CHECK_EQUAL(entrySet.size(), 4);
    // should have offsets and positions because we are munging all the fields together
    for (Collection<TermVectorEntryPtr>::iterator tve = entrySet.begin(); tve != entrySet.end(); ++tve)
    {
        BOOST_CHECK(*tve);
        BOOST_CHECK((*tve)->getOffsets());
        BOOST_CHECK((*tve)->getPositions());
    }
    
    FieldSortedTermVectorMapperPtr fsMapper = newLucene<FieldSortedTermVectorMapper>(TermVectorEntryFreqSortedComparator::compare);
    reader->get(0, fsMapper);
    MapStringCollectionTermVectorEntry map = fsMapper->getFieldToTerms();
    BOOST_CHECK_EQUAL(map.size(), testFields.size());
    for (MapStringCollectionTermVectorEntry::iterator entry = map.begin(); entry != map.end(); ++entry)
    {
        Collection<TermVectorEntryPtr> termVectorEntries = entry->second;
        BOOST_CHECK_EQUAL(termVectorEntries.size(), 4);
        for (Collection<TermVectorEntryPtr>::iterator tve = termVectorEntries.begin(); tve != termVectorEntries.end(); ++tve)
        {
            BOOST_CHECK(*tve);
            // Check offsets and positions.
            String field = (*tve)->getField();
            if (field == testFields[0])
            {
                // should have offsets
                BOOST_CHECK((*tve)->getOffsets());
                BOOST_CHECK((*tve)->getPositions());
            }
            else if (field == testFields[1])
            {
                // should not have offsets
                BOOST_CHECK(!(*tve)->getOffsets());
                BOOST_CHECK(!(*tve)->getPositions());
            }
        }
    }
    
    // Try mapper that ignores offs and positions
    fsMapper = newLucene<FieldSortedTermVectorMapper>(true, true, TermVectorEntryFreqSortedComparator::compare);
    reader->get(0, fsMapper);
    map = fsMapper->getFieldToTerms();
    BOOST_CHECK_EQUAL(map.size(), testFields.size());
    for (MapStringCollectionTermVectorEntry::iterator entry = map.begin(); entry != map.end(); ++entry)
    {
        Collection<TermVectorEntryPtr> termVectorEntries = entry->second;
        BOOST_CHECK_EQUAL(termVectorEntries.size(), 4);
        for (Collection<TermVectorEntryPtr>::iterator tve = termVectorEntries.begin(); tve != termVectorEntries.end(); ++tve)
        {
            BOOST_CHECK(*tve);
            // Check offsets and positions.
            String field = (*tve)->getField();
            if (field == testFields[0])
            {
                // should have offsets
                BOOST_CHECK(!(*tve)->getOffsets());
                BOOST_CHECK(!(*tve)->getPositions());
            }
            else if (field == testFields[1])
            {
                // should not have offsets
                BOOST_CHECK(!(*tve)->getOffsets());
                BOOST_CHECK(!(*tve)->getPositions());
            }
        }
    }
    
    // test setDocumentNumber()
    IndexReaderPtr ir = IndexReader::open(dir, true);
    DocNumAwareMapperPtr docNumAwareMapper = newLucene<DocNumAwareMapper>();
    BOOST_CHECK_EQUAL(-1, docNumAwareMapper->getDocumentNumber());

    ir->getTermFreqVector(0, docNumAwareMapper);
    BOOST_CHECK_EQUAL(0, docNumAwareMapper->getDocumentNumber());
    docNumAwareMapper->setDocumentNumber(-1);

    ir->getTermFreqVector(1, docNumAwareMapper);
    BOOST_CHECK_EQUAL(1, docNumAwareMapper->getDocumentNumber());
    docNumAwareMapper->setDocumentNumber(-1);

    ir->getTermFreqVector(0, L"f1", docNumAwareMapper);
    BOOST_CHECK_EQUAL(0, docNumAwareMapper->getDocumentNumber());
    docNumAwareMapper->setDocumentNumber(-1);

    ir->getTermFreqVector(1, L"f2", docNumAwareMapper);
    BOOST_CHECK_EQUAL(1, docNumAwareMapper->getDocumentNumber());
    docNumAwareMapper->setDocumentNumber(-1);

    ir->getTermFreqVector(0, L"f1", docNumAwareMapper);
    BOOST_CHECK_EQUAL(0, docNumAwareMapper->getDocumentNumber());

    ir->close();
}

/// Make sure exceptions and bad params are handled appropriately
BOOST_AUTO_TEST_CASE(testBadParams)
{
    {
        TermVectorsReaderPtr reader = newLucene<TermVectorsReader>(dir, seg, fieldInfos);
        BOOST_CHECK(reader);
        // Bad document number, good field number
        BOOST_CHECK_EXCEPTION(reader->get(50, testFields[0]), IOException, check_exception(LuceneException::IO));
    }
    
    {
        TermVectorsReaderPtr reader = newLucene<TermVectorsReader>(dir, seg, fieldInfos);
        BOOST_CHECK(reader);
        // Bad document number, no field
        BOOST_CHECK_EXCEPTION(reader->get(50), IOException, check_exception(LuceneException::IO));
    }
    
    {
        TermVectorsReaderPtr reader = newLucene<TermVectorsReader>(dir, seg, fieldInfos);
        BOOST_CHECK(reader);
        
        // Good document number, bad field number
        TermFreqVectorPtr vector;
        BOOST_CHECK_NO_THROW(vector = reader->get(0, L"f50"));
        BOOST_CHECK(!vector);
    }
}

BOOST_AUTO_TEST_SUITE_END()
