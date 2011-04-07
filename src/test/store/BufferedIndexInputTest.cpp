/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <fstream>
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "BufferedIndexInput.h"
#include "_SimpleFSDirectory.h"
#include "MockFSDirectory.h"
#include "FSDirectory.h"
#include "IndexWriter.h"
#include "IndexReader.h"
#include "IndexSearcher.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "Term.h"
#include "TermQuery.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "Random.h"
#include "MiscUtils.h"
#include "FileUtils.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(BufferedIndexInputTest, LuceneTestFixture)

class TestableBufferedIndexInputRead : public BufferedIndexInput
{
public:
    TestableBufferedIndexInputRead(const uint8_t* b, int32_t length) : inputBytes(b), inputLength(length), nextByte(0)
    {
    }
    
    virtual ~TestableBufferedIndexInputRead()
    {
    }

protected:
    const uint8_t* inputBytes;
    int32_t inputLength;
    int32_t nextByte;

public:
    virtual void readInternal(uint8_t* b, int32_t offset, int32_t length)
    {
        std::copy(inputBytes + nextByte + offset, inputBytes + nextByte + offset + length, b + offset);
        nextByte += length;
    }
    
    virtual void seekInternal(int64_t pos)
    {
    }
    
    virtual int64_t length()
    {
        return inputLength;
    }
    
    virtual IndexInputPtr clone()
    {
        return IndexInputPtr();
    }
};

BOOST_AUTO_TEST_CASE(testReadInt)
{
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[4] = { 1, 2, 3, 4 };
    std::memcpy(inputBytes.get(), input, 4);
    TestableBufferedIndexInputRead indexInput(inputBytes.get(), 4);
    BOOST_CHECK_EQUAL(indexInput.readInt(), 16909060);
}

BOOST_AUTO_TEST_CASE(testReadVInt)
{
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[4] = { 200, 201, 150, 96 };
    std::memcpy(inputBytes.get(), input, 4);
    TestableBufferedIndexInputRead indexInput(inputBytes.get(), 4);
    BOOST_CHECK_EQUAL(indexInput.readVInt(), 201696456);
}

BOOST_AUTO_TEST_CASE(testReadLong)
{
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[8] = { 32, 43, 32, 96, 12, 54, 22, 96 };
    std::memcpy(inputBytes.get(), input, 8);
    TestableBufferedIndexInputRead indexInput(inputBytes.get(), 8);
    BOOST_CHECK_EQUAL(indexInput.readLong(), 2317982030106072672LL);
}

BOOST_AUTO_TEST_CASE(testReadVLong)
{
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[8] = { 213, 143, 132, 196, 172, 154, 129, 96 };
    std::memcpy(inputBytes.get(), input, 8);
    TestableBufferedIndexInputRead indexInput(inputBytes.get(), 8);
    BOOST_CHECK_EQUAL(indexInput.readVLong(), -926873643LL);
}

BOOST_AUTO_TEST_CASE(testReadString)
{
    ByteArray inputBytes(ByteArray::newInstance(30));
    uint8_t input[12] = { 11, 't', 'e', 's', 't', ' ', 's', 't', 'r', 'i', 'n', 'g' };
    std::memcpy(inputBytes.get(), input, 12);
    TestableBufferedIndexInputRead indexInput(inputBytes.get(), 12);
    BOOST_CHECK_EQUAL(indexInput.readString(), L"test string");
}

BOOST_AUTO_TEST_CASE(testReadModifiedUTF8String)
{
    ByteArray inputBytes(ByteArray::newInstance(30));
    uint8_t input[12] = { 11, 't', 'e', 's', 't', ' ', 's', 't', 'r', 'i', 'n', 'g' };
    std::memcpy(inputBytes.get(), input, 12);
    TestableBufferedIndexInputRead indexInput(inputBytes.get(), 12);
    BOOST_CHECK_EQUAL(indexInput.readModifiedUTF8String(), L"test string");
}

BOOST_AUTO_TEST_CASE(testReadChars)
{
    ByteArray inputBytes(ByteArray::newInstance(30));
    uint8_t input[11] = { 't', 'e', 's', 't', ' ', 's', 't', 'r', 'i', 'n', 'g' };
    std::memcpy(inputBytes.get(), input, 11);
    
    TestableBufferedIndexInputRead indexInput(inputBytes.get(), 11);
    
    ByteArray outputChars(ByteArray::newInstance(30 * sizeof(wchar_t)));
    indexInput.readChars((wchar_t*)outputChars.get(), 0, 11);
    
    wchar_t expected[11] = { L't', L'e', L's', L't', L' ', L's', L't', L'r', L'i', L'n', L'g' };
    BOOST_CHECK_EQUAL(std::memcmp(outputChars.get(), expected, 11 * sizeof(wchar_t)), 0);
}

BOOST_AUTO_TEST_CASE(testSkipOneChar)
{
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[5] = { 1, 2, 3, 4, 5 };
    std::memcpy(inputBytes.get(), input, 5);
    TestableBufferedIndexInputRead indexInput(inputBytes.get(), 5);
    indexInput.skipChars(1);
    BOOST_CHECK_EQUAL(indexInput.getFilePointer(), 1);
}

BOOST_AUTO_TEST_CASE(testSkipTwoChars)
{
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[5] = { 1, 2, 3, 4, 5 };
    std::memcpy(inputBytes.get(), input, 5);
    TestableBufferedIndexInputRead indexInput(inputBytes.get(), 5);
    indexInput.skipChars(2);
    BOOST_CHECK_EQUAL(indexInput.getFilePointer(), 2);
}

BOOST_AUTO_TEST_CASE(testSkipTwoCharsAdditionalChar)
{
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[5] = { 1, 132, 132, 4, 5 };
    std::memcpy(inputBytes.get(), input, 5);
    TestableBufferedIndexInputRead indexInput(inputBytes.get(), 5);
    indexInput.skipChars(2);
    BOOST_CHECK_EQUAL(indexInput.getFilePointer(), 3);
}

BOOST_AUTO_TEST_CASE(testSkipTwoCharsAdditionalTwoChars)
{
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[5] = { 1, 232, 232, 4, 5 };
    std::memcpy(inputBytes.get(), input, 5);
    TestableBufferedIndexInputRead indexInput(inputBytes.get(), 5);
    indexInput.skipChars(2);
    BOOST_CHECK_EQUAL(indexInput.getFilePointer(), 4);
}

BOOST_AUTO_TEST_CASE(testReadCollection)
{
    ByteArray inputBytes(ByteArray::newInstance(100));
    
    uint8_t input[88] = {0x80, 0x01, 0xff, 0x7f, 0x80, 0x80, 0x01, 0x81, 0x80, 0x01, 0x06,
                          'L', 'u', 'c', 'e', 'n', 'e',
                         
                         // 2-byte UTF-8 (U+00BF "INVERTED QUESTION MARK") 
                         0x02, 0xc2, 0xbf, 0x0a, 'L', 'u', 0xc2, 0xbf, 'c', 'e', 0xc2, 0xbf, 'n', 'e',
                         
                         // 3-byte UTF-8 (U+2620 "SKULL AND CROSSBONES") 
                         0x03, 0xe2, 0x98, 0xa0, 0x0c, 'L', 'u', 0xe2, 0x98, 0xa0, 'c', 'e', 0xe2, 0x98, 0xa0, 'n', 'e',
                         
                         // surrogate pairs
                         // (U+1D11E "MUSICAL SYMBOL G CLEF")
                         // (U+1D160 "MUSICAL SYMBOL EIGHTH NOTE")
                         0x04, 0xf0, 0x9d, 0x84, 0x9e, 0x08, 0xf0, 0x9d, 0x84, 0x9e, 0xf0, 0x9d, 0x85, 0xa0, 0x0e,
                         'L', 'u', 0xf0, 0x9d, 0x84, 0x9e, 'c', 'e', 0xf0, 0x9d, 0x85, 0xa0, 'n', 'e',
                         
                         // null bytes
                         0x01, 0x00, 0x08, 'L', 'u', 0x00, 'c', 'e', 0x00, 'n', 'e'};
    std::memcpy(inputBytes.get(), input, 88);
    
    TestableBufferedIndexInputRead indexInput(inputBytes.get(), 88);
    indexInput.setBufferSize(10);
    
    BOOST_CHECK_EQUAL(indexInput.readVInt(), 128);
    BOOST_CHECK_EQUAL(indexInput.readVInt(), 16383);
    BOOST_CHECK_EQUAL(indexInput.readVInt(), 16384);
    BOOST_CHECK_EQUAL(indexInput.readVInt(), 16385);    
    BOOST_CHECK_EQUAL(indexInput.readString(), L"Lucene");
    
    const uint8_t question[] = {0xc2, 0xbf};
    BOOST_CHECK_EQUAL(indexInput.readString(), UTF8_TO_STRING(question));
    const uint8_t skull[] = {0x4c, 0x75, 0xc2, 0xbf, 0x63, 0x65, 0xc2, 0xbf, 0x6e, 0x65};
    BOOST_CHECK_EQUAL(indexInput.readString(), UTF8_TO_STRING(skull));
    const uint8_t gclef[] = {0xe2, 0x98, 0xa0};
    BOOST_CHECK_EQUAL(indexInput.readString(), UTF8_TO_STRING(gclef));
    const uint8_t eighthnote[] = {0x4c, 0x75, 0xe2, 0x98, 0xa0, 0x63, 0x65, 0xe2, 0x98, 0xa0, 0x6e, 0x65};
    BOOST_CHECK_EQUAL(indexInput.readString(), UTF8_TO_STRING(eighthnote));
    
    String readString(indexInput.readString());
    
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    BOOST_CHECK_EQUAL(readString[0], 55348);
    BOOST_CHECK_EQUAL(readString[1], 56606);
    #else
    BOOST_CHECK_EQUAL(readString[0], 119070);
    #endif
    
    readString = indexInput.readString();
    
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    BOOST_CHECK_EQUAL(readString[0], 55348);
    BOOST_CHECK_EQUAL(readString[1], 56606);
    BOOST_CHECK_EQUAL(readString[2], 55348);
    BOOST_CHECK_EQUAL(readString[3], 56672);
    #else
    BOOST_CHECK_EQUAL(readString[0], 119070);
    BOOST_CHECK_EQUAL(readString[1], 119136);
    #endif
    
    readString = indexInput.readString();
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    BOOST_CHECK_EQUAL(readString[0], L'L');
    BOOST_CHECK_EQUAL(readString[1], L'u');
    BOOST_CHECK_EQUAL(readString[2], 55348);
    BOOST_CHECK_EQUAL(readString[3], 56606);
    BOOST_CHECK_EQUAL(readString[4], L'c');
    BOOST_CHECK_EQUAL(readString[5], L'e');
    BOOST_CHECK_EQUAL(readString[6], 55348);
    BOOST_CHECK_EQUAL(readString[7], 56672);
    BOOST_CHECK_EQUAL(readString[8], L'n');
    BOOST_CHECK_EQUAL(readString[9], L'e');
    #else
    BOOST_CHECK_EQUAL(readString[0], L'L');
    BOOST_CHECK_EQUAL(readString[1], L'u');
    BOOST_CHECK_EQUAL(readString[2], 119070);
    BOOST_CHECK_EQUAL(readString[3], L'c');
    BOOST_CHECK_EQUAL(readString[4], L'e');
    BOOST_CHECK_EQUAL(readString[5], 119136);
    BOOST_CHECK_EQUAL(readString[6], L'n');
    BOOST_CHECK_EQUAL(readString[7], L'e');
    #endif
    
    readString = indexInput.readString();
    BOOST_CHECK_EQUAL(readString[0], 0);
    
    readString = indexInput.readString();
    BOOST_CHECK_EQUAL(readString[0], L'L');
    BOOST_CHECK_EQUAL(readString[1], L'u');
    BOOST_CHECK_EQUAL(readString[2], 0);
    BOOST_CHECK_EQUAL(readString[3], L'c');
    BOOST_CHECK_EQUAL(readString[4], L'e');
    BOOST_CHECK_EQUAL(readString[5], 0);
    BOOST_CHECK_EQUAL(readString[6], L'n');
    BOOST_CHECK_EQUAL(readString[7], L'e');
}

BOOST_AUTO_TEST_CASE(testSkipCollection)
{
    ByteArray inputBytes(ByteArray::newInstance(100));
    uint8_t input[17] = {0x80, 0x01, 0xff, 0x7f, 0x80, 0x80, 0x01, 0x81, 0x80, 0x01, 0x06, 'L', 'u', 'c', 'e', 'n', 'e'};
    std::memcpy(inputBytes.get(), input, 17);
    
    TestableBufferedIndexInputRead indexInput(inputBytes.get(), 17);
    BOOST_CHECK_EQUAL(indexInput.readVInt(), 128);
    BOOST_CHECK_EQUAL(indexInput.readVInt(), 16383);
    BOOST_CHECK_EQUAL(indexInput.readVInt(), 16384);
    BOOST_CHECK_EQUAL(indexInput.readVInt(), 16385);
    BOOST_CHECK_EQUAL(indexInput.readVInt(), 6);
    indexInput.skipChars(3);
    ByteArray remainingBytes(ByteArray::newInstance(4 * sizeof(wchar_t)));
    indexInput.readChars((wchar_t*)remainingBytes.get(), 0, 3);
    BOOST_CHECK_EQUAL(String((wchar_t*)remainingBytes.get(), 3), L"ene");
}

// byten emulates a file - byten(n) returns the n'th byte in that file.
uint8_t byten(int64_t n)
{
    return (uint8_t)(n * n % 256);
}

void writeBytes(std::ofstream& file, int64_t size)
{
    for (int64_t i = 0; i < size; ++i)
        file << byten(i);
    file.flush();
}

static const int64_t TEST_FILE_LENGTH = 1024 * 1024;

class MyBufferedIndexInput : public BufferedIndexInput
{
public:
    MyBufferedIndexInput()
    {
        this->len = LLONG_MAX; // an infinite file
        this->pos = 0;
    }
    
    MyBufferedIndexInput(int64_t len)
    {
        this->len = len;
        this->pos = 0;
    }

protected:
    virtual void readInternal(uint8_t* b, int32_t offset, int32_t length)
    {
        for (int32_t i = offset; i < (offset + length); ++i)
            b[i] = byten(pos++);
    }
    
    virtual void seekInternal(int64_t pos)
    {
        this->pos = pos;
    }

public:
    virtual void close()
    {
    }
    
    virtual int64_t length()
    {
        return len;
    }

protected:
    int64_t pos;
    int64_t len;
};

DECLARE_SHARED_PTR(MyBufferedIndexInput)

// Call readByte() repeatedly, past the buffer boundary, and see that it is working as expected.
// Our input comes from a dynamically generated/ "file" - see MyBufferedIndexInput.
BOOST_AUTO_TEST_CASE(testReadByte)
{
    MyBufferedIndexInputPtr input(newLucene<MyBufferedIndexInput>());
    for (int32_t i = 0; i < BufferedIndexInput::BUFFER_SIZE * 10; ++i)
        BOOST_CHECK_EQUAL(input->readByte(), byten(i));
}

void checkReadBytes(IndexInputPtr input, int32_t size, int32_t pos)
{
    // Just to see that "offset" is treated properly in readBytes(), we add an arbitrary offset at 
    // the beginning of the array
    int32_t offset = size % 10; // arbitrary
    ByteArray buffer(ByteArray::newInstance(10));
    buffer.resize(MiscUtils::getNextSize(offset + size));
    BOOST_CHECK_EQUAL(pos, input->getFilePointer());
    int64_t left = TEST_FILE_LENGTH - input->getFilePointer();
    if (left <= 0)
        return;
    else if (left < size)
        size = (int32_t)left;
    input->readBytes(buffer.get(), offset, size);
    BOOST_CHECK_EQUAL(pos + size, input->getFilePointer());
    for (int32_t i = 0; i < size; ++i)
        BOOST_CHECK_EQUAL(byten(pos + i), buffer[offset + i]);
}

void runReadBytes(IndexInputPtr input, int32_t bufferSize)
{
    int32_t pos = 0;
    RandomPtr random = newLucene<Random>();
    
    // gradually increasing size
    for (int32_t size = 1; size < bufferSize * 10; size = size + size / 200 + 1)
    {
        checkReadBytes(input, size, pos);
        pos += size;
        if (pos >= TEST_FILE_LENGTH)
        {
            // wrap
            pos = 0;
            input->seek(0);
        }
    }
    // wildly fluctuating size
    for (int64_t i = 0; i < 1000; ++i)
    {
        int32_t size = random->nextInt(10000);
        checkReadBytes(input, 1 + size, pos);
        pos += 1 + size;
        if (pos >= TEST_FILE_LENGTH)
        {
            // wrap
            pos = 0;
            input->seek(0);
        }
    }
    // constant small size (7 bytes)
    for (int32_t i = 0; i < bufferSize; ++i)
    {
        checkReadBytes(input, 7, pos);
        pos += 7;
        if (pos >= TEST_FILE_LENGTH)
        {
            // wrap
            pos = 0;
            input->seek(0);
        }
    }
}

void runReadBytesAndClose(IndexInputPtr input, int32_t bufferSize)
{
    LuceneException finally;
    try
    {
        runReadBytes(input, bufferSize);
    }
    catch (LuceneException& e)
    {
        finally = e;
    }
    input->close();
    finally.throwException();
}

// Call readBytes() repeatedly, with various chunk sizes (from 1 byte to larger than the buffer size), and see 
// that it returns the bytes we expect. Our input comes from a dynamically generated "file" - see MyBufferedIndexInput.
BOOST_AUTO_TEST_CASE(testReadBytes)
{
    MyBufferedIndexInputPtr input(newLucene<MyBufferedIndexInput>());
    runReadBytes(input, BufferedIndexInput::BUFFER_SIZE);
    
    int32_t inputBufferSize = 128;
    String tmpInputFile(getTempDir(L"IndexInput"));
    std::ofstream file(StringUtils::toUTF8(tmpInputFile).c_str(), std::ios::binary | std::ios::out);
    writeBytes(file, TEST_FILE_LENGTH);
    
    // run test with chunk size of 10 bytes
    runReadBytesAndClose(newLucene<SimpleFSIndexInput>(tmpInputFile, inputBufferSize, 10), inputBufferSize);
    
    // run test with chunk size of 100 MB - default
    runReadBytesAndClose(newLucene<SimpleFSIndexInput>(tmpInputFile, inputBufferSize, FSDirectory::DEFAULT_READ_CHUNK_SIZE), inputBufferSize);
    
    FileUtils::removeFile(tmpInputFile);
}

// This tests that attempts to readBytes() past an EOF will fail, while reads up to the EOF will succeed. The 
// EOF is determined by the BufferedIndexInput's arbitrary length() value.
BOOST_AUTO_TEST_CASE(testEOF)
{
    MyBufferedIndexInputPtr input(newLucene<MyBufferedIndexInput>(1024));
    
    // see that we can read all the bytes at one go
    checkReadBytes(input, (int32_t)input->length(), 0);
    
    // go back and see that we can't read more than that, for small and large overflows
    int32_t pos = (int32_t)input->length() - 10;
    input->seek(pos);
    checkReadBytes(input, 10, pos);
    input->seek(pos);
    
    BOOST_CHECK_EXCEPTION(checkReadBytes(input, 11, pos), LuceneException, check_exception(LuceneException::IO));
    
    input->seek(pos);
    
    BOOST_CHECK_EXCEPTION(checkReadBytes(input, 50, pos), LuceneException, check_exception(LuceneException::IO));
    
    input->seek(pos);
    
    BOOST_CHECK_EXCEPTION(checkReadBytes(input, 100000, pos), LuceneException, check_exception(LuceneException::IO));
}

BOOST_AUTO_TEST_CASE(testSetBufferSize)
{
    String indexDir(getTempDir(L"testSetBufferSize"));
    MockFSDirectoryPtr dir = newLucene<MockFSDirectory>(indexDir);
    
    LuceneException finally;
    try
    {
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        writer->setUseCompoundFile(false);
        for (int32_t i = 0; i < 37; ++i)
        {
            DocumentPtr doc = newLucene<Document>();
            doc->add(newLucene<Field>(L"content", L"aaa bbb ccc ddd" + StringUtils::toString(i), Field::STORE_YES, Field::INDEX_ANALYZED));
            doc->add(newLucene<Field>(L"id", StringUtils::toString(i), Field::STORE_YES, Field::INDEX_ANALYZED));
            writer->addDocument(doc);
        }
        writer->close();
        
        dir->allIndexInputs.clear();
        
        IndexReaderPtr reader = IndexReader::open(dir, false);
        TermPtr aaa = newLucene<Term>(L"content", L"aaa");
        TermPtr bbb = newLucene<Term>(L"content", L"bbb");
        TermPtr ccc = newLucene<Term>(L"content", L"ccc");
        BOOST_CHECK_EQUAL(reader->docFreq(ccc), 37);
        reader->deleteDocument(0);
        BOOST_CHECK_EQUAL(reader->docFreq(aaa), 37);
        dir->tweakBufferSizes();
        reader->deleteDocument(4);
        BOOST_CHECK_EQUAL(reader->docFreq(bbb), 37);
        dir->tweakBufferSizes();
        
        IndexSearcherPtr searcher = newLucene<IndexSearcher>(reader);
        Collection<ScoreDocPtr> hits = searcher->search(newLucene<TermQuery>(bbb), FilterPtr(), 1000)->scoreDocs;
        dir->tweakBufferSizes();
        BOOST_CHECK_EQUAL(hits.size(), 35);
        dir->tweakBufferSizes();
        hits = searcher->search(newLucene<TermQuery>(newLucene<Term>(L"id", L"33")), FilterPtr(), 1000)->scoreDocs;
        dir->tweakBufferSizes();
        BOOST_CHECK_EQUAL(hits.size(), 1);
        hits = searcher->search(newLucene<TermQuery>(aaa), FilterPtr(), 1000)->scoreDocs;
        dir->tweakBufferSizes();
        BOOST_CHECK_EQUAL(hits.size(), 35);
        searcher->close();
        reader->close();
    }
    catch (LuceneException& e)
    {
        finally = e;
    }
    FileUtils::removeDirectory(indexDir);
    finally.throwException();
}

BOOST_AUTO_TEST_SUITE_END()
