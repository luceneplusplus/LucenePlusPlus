/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "Payload.h"
#include "RAMDirectory.h"
#include "Analyzer.h"
#include "WhitespaceTokenizer.h"
#include "TokenFilter.h"
#include "TokenStream.h"
#include "PayloadAttribute.h"
#include "IndexWriter.h"
#include "Document.h"
#include "Field.h"
#include "SegmentReader.h"
#include "FieldInfos.h"
#include "FieldInfo.h"
#include "FSDirectory.h"
#include "Term.h"
#include "TermPositions.h"
#include "TermAttribute.h"
#include "WhitespaceAnalyzer.h"
#include "TermEnum.h"
#include "Base64.h"
#include "MiscUtils.h"
#include "UnicodeUtils.h"
#include "FileUtils.h"

using namespace Lucene;

typedef LuceneTestFixture PayloadsTest;

DECLARE_SHARED_PTR(PayloadData)
DECLARE_SHARED_PTR(PayloadFilter)
DECLARE_SHARED_PTR(PayloadAnalyzer)

class PayloadData : public LuceneObject {
public:
    PayloadData(int32_t skip, ByteArray data, int32_t offset, int32_t length) {
        this->numFieldInstancesToSkip = skip;
        this->data = data;
        this->offset = offset;
        this->length = length;
    }

    virtual ~PayloadData() {
    }

    LUCENE_CLASS(PayloadData);

public:
    ByteArray data;
    int32_t offset;
    int32_t length;
    int32_t numFieldInstancesToSkip;
};

/// This Filter adds payloads to the tokens.
class PayloadFilter : public TokenFilter {
public:
    PayloadFilter(const TokenStreamPtr& in, ByteArray data, int32_t offset, int32_t length) : TokenFilter(in) {
        this->payload = newLucene<Payload>();
        this->data = data;
        this->length = length;
        this->offset = offset;
        this->payloadAtt = addAttribute<PayloadAttribute>();
    }

    virtual ~PayloadFilter() {
    }

    LUCENE_CLASS(PayloadFilter);

public:
    ByteArray data;
    int32_t length;
    int32_t offset;
    PayloadPtr payload;
    PayloadAttributePtr payloadAtt;

public:
    virtual bool incrementToken() {
        bool hasNext = input->incrementToken();
        if (hasNext) {
            if (offset + length <= data.size()) {
                PayloadPtr p = newLucene<Payload>();
                payloadAtt->setPayload(p);
                p->setData(data, offset, length);
                offset += length;
            } else {
                payloadAtt->setPayload(PayloadPtr());
            }
        }
        return hasNext;
    }
};

/// This Analyzer uses an WhitespaceTokenizer and PayloadFilter.
class PayloadAnalyzer : public Analyzer {
public:
    PayloadAnalyzer() {
        fieldToData = HashMap<String, PayloadDataPtr>::newInstance();
    }

    virtual ~PayloadAnalyzer() {
    }

    LUCENE_CLASS(PayloadAnalyzer);

public:
    HashMap<String, PayloadDataPtr> fieldToData;

public:
    void setPayloadData(const String& field, ByteArray data, int32_t offset, int32_t length) {
        fieldToData.put(field, newLucene<PayloadData>(0, data, offset, length));
    }

    void setPayloadData(const String& field, int32_t numFieldInstancesToSkip, ByteArray data, int32_t offset, int32_t length) {
        PayloadDataPtr payload = newLucene<PayloadData>(numFieldInstancesToSkip, data, offset, length);
        fieldToData.put(field, payload);
    }

    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        PayloadDataPtr payload = fieldToData.get(fieldName);
        TokenStreamPtr ts = newLucene<WhitespaceTokenizer>(reader);
        if (payload) {
            if (payload->numFieldInstancesToSkip == 0) {
                ts = newLucene<PayloadFilter>(ts, payload->data, payload->offset, payload->length);
            } else {
                --payload->numFieldInstancesToSkip;
            }
        }
        return ts;
    }
};

static void generateRandomData(ByteArray data) {
    std::generate(data.get(), data.get() + data.size(), rand);
}

static ByteArray generateRandomData(int32_t n) {
    ByteArray data(ByteArray::newInstance(n));
    generateRandomData(data);
    return data;
}

static Collection<TermPtr> generateTerms(const String& fieldName, int32_t n) {
    int32_t maxDigits = (int32_t)(std::log((double)n) / std::log(10.0));
    Collection<TermPtr> terms = Collection<TermPtr>::newInstance(n);
    for (int32_t i = 0; i < n; ++i) {
        StringStream sb;
        sb << L"t";
        int32_t zeros = maxDigits - (int32_t)(std::log((double)i) / std::log(10.0));
        for (int32_t j = 0; j < zeros; ++j) {
            sb << L"0";
        }
        sb << i;
        terms[i] = newLucene<Term>(fieldName, sb.str());
    }
    return terms;
}

/// Simple tests to test the Payload class
TEST_F(PayloadsTest, testPayload) {
    ByteArray testData(ByteArray::newInstance(15));
    uint8_t input[15] = { 'T', 'h', 'i', 's', ' ', 'i', 's', ' ', 'a', ' ', 't', 'e', 's', 't', '!' };
    std::memcpy(testData.get(), input, 15);
    PayloadPtr payload = newLucene<Payload>(testData);
    EXPECT_EQ(testData.size(), payload->length());

    // test copyTo()
    ByteArray target(ByteArray::newInstance(testData.size() - 1));
    try {
        payload->copyTo(target, 0);
    } catch (IndexOutOfBoundsException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IndexOutOfBounds)(e));
    }

    target.resize(testData.size() + 3);
    payload->copyTo(target, 3);

    for (int32_t i = 0; i < testData.size(); ++i) {
        EXPECT_EQ(testData[i], target[i + 3]);
    }

    // test toByteArray()
    target = payload->toByteArray();
    EXPECT_TRUE(testData.equals(target));

    // test byteAt()
    for (int32_t i = 0; i < testData.size(); ++i) {
        EXPECT_EQ(payload->byteAt(i), testData[i]);
    }

    try {
        payload->byteAt(testData.size() + 1);
    } catch (IndexOutOfBoundsException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IndexOutOfBounds)(e));
    }

    PayloadPtr clone = boost::dynamic_pointer_cast<Payload>(payload->clone());
    EXPECT_EQ(payload->length(), clone->length());
    for (int32_t i = 0; i < payload->length(); ++i) {
        EXPECT_EQ(payload->byteAt(i), clone->byteAt(i));
    }
}

/// Tests whether the DocumentWriter and SegmentMerger correctly enable the payload bit in the FieldInfo
TEST_F(PayloadsTest, testPayloadFieldBit) {
    DirectoryPtr ram = newLucene<RAMDirectory>();
    PayloadAnalyzerPtr analyzer = newLucene<PayloadAnalyzer>();
    IndexWriterPtr writer = newLucene<IndexWriter>(ram, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr d = newLucene<Document>();
    // this field won't have any payloads
    d->add(newLucene<Field>(L"f1", L"This field has no payloads", Field::STORE_NO, Field::INDEX_ANALYZED));
    // this field will have payloads in all docs, however not for all term positions,
    // so this field is used to check if the DocumentWriter correctly enables the payloads bit
    // even if only some term positions have payloads
    d->add(newLucene<Field>(L"f2", L"This field has payloads in all docs", Field::STORE_NO, Field::INDEX_ANALYZED));
    d->add(newLucene<Field>(L"f2", L"This field has payloads in all docs", Field::STORE_NO, Field::INDEX_ANALYZED));
    // this field is used to verify if the SegmentMerger enables payloads for a field if it has payloads
    // enabled in only some documents
    d->add(newLucene<Field>(L"f3", L"This field has payloads in some docs", Field::STORE_NO, Field::INDEX_ANALYZED));
    // only add payload data for field f2

    ByteArray someData(ByteArray::newInstance(8));
    uint8_t input[8] = { 's', 'o', 'm', 'e', 'd', 'a', 't', 'a' };
    std::memcpy(someData.get(), input, 8);

    analyzer->setPayloadData(L"f2", 1, someData, 0, 1);

    writer->addDocument(d);
    // flush
    writer->close();

    SegmentReaderPtr reader = SegmentReader::getOnlySegmentReader(ram);
    FieldInfosPtr fi = reader->fieldInfos();
    EXPECT_TRUE(!fi->fieldInfo(L"f1")->storePayloads);
    EXPECT_TRUE(fi->fieldInfo(L"f2")->storePayloads);
    EXPECT_TRUE(!fi->fieldInfo(L"f3")->storePayloads);
    reader->close();

    // now we add another document which has payloads for field f3 and verify if the SegmentMerger
    // enabled payloads for that field
    writer = newLucene<IndexWriter>(ram, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);
    d = newLucene<Document>();
    d->add(newLucene<Field>(L"f1", L"This field has no payloads", Field::STORE_NO, Field::INDEX_ANALYZED));
    d->add(newLucene<Field>(L"f2", L"This field has payloads in all docs", Field::STORE_NO, Field::INDEX_ANALYZED));
    d->add(newLucene<Field>(L"f2", L"This field has payloads in all docs", Field::STORE_NO, Field::INDEX_ANALYZED));
    d->add(newLucene<Field>(L"f3", L"This field has payloads in some docs", Field::STORE_NO, Field::INDEX_ANALYZED));
    // add payload data for field f2 and f3
    analyzer->setPayloadData(L"f2", someData, 0, 1);
    analyzer->setPayloadData(L"f3", someData, 0, 3);
    writer->addDocument(d);
    // force merge
    writer->optimize();
    // flush
    writer->close();

    reader = SegmentReader::getOnlySegmentReader(ram);
    fi = reader->fieldInfos();
    EXPECT_TRUE(!fi->fieldInfo(L"f1")->storePayloads);
    EXPECT_TRUE(fi->fieldInfo(L"f2")->storePayloads);
    EXPECT_TRUE(fi->fieldInfo(L"f3")->storePayloads);
    reader->close();
}

/// Builds an index with payloads in the given Directory and performs different
/// tests to verify the payload encoding
static void encodingTest(const DirectoryPtr& dir) {
    PayloadAnalyzerPtr analyzer = newLucene<PayloadAnalyzer>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);

    // should be in sync with value in TermInfosWriter
    int32_t skipInterval = 16;

    int32_t numTerms = 5;
    String fieldName = L"f1";

    int32_t numDocs = skipInterval + 1;
    // create content for the test documents with just a few terms
    Collection<TermPtr> terms = generateTerms(fieldName, numTerms);
    StringStream sb;
    for (Collection<TermPtr>::iterator term = terms.begin(); term != terms.end(); ++term) {
        sb << (*term)->text() << L" ";
    }
    String content = sb.str();

    int32_t payloadDataLength = numTerms * numDocs * 2 + numTerms * numDocs * (numDocs - 1) / 2;
    ByteArray payloadData = generateRandomData(payloadDataLength);

    DocumentPtr d = newLucene<Document>();
    d->add(newLucene<Field>(fieldName, content, Field::STORE_NO, Field::INDEX_ANALYZED));

    // add the same document multiple times to have the same payload lengths for all
    // occurrences within two consecutive skip intervals
    int32_t offset = 0;
    for (int32_t i = 0; i < 2 * numDocs; ++i) {
        analyzer->setPayloadData(fieldName, payloadData, offset, 1);
        offset += numTerms;
        writer->addDocument(d);
    }

    // make sure we create more than one segment to test merging
    writer->commit();

    for (int32_t i = 0; i < numDocs; ++i) {
        analyzer->setPayloadData(fieldName, payloadData, offset, i);
        offset += i * numTerms;
        writer->addDocument(d);
    }

    writer->optimize();
    // flush
    writer->close();

    // Verify the index
    IndexReaderPtr reader = IndexReader::open(dir, true);

    ByteArray verifyPayloadData(ByteArray::newInstance(payloadDataLength));
    offset = 0;
    Collection<TermPositionsPtr> tps = Collection<TermPositionsPtr>::newInstance(numTerms);
    for (int32_t i = 0; i < numTerms; ++i) {
        tps[i] = reader->termPositions(terms[i]);
    }

    while (tps[0]->next()) {
        for (int32_t i = 1; i < numTerms; ++i) {
            tps[i]->next();
        }
        int32_t freq = tps[0]->freq();

        for (int32_t i = 0; i < freq; ++i) {
            for (int32_t j = 0; j < numTerms; ++j) {
                tps[j]->nextPosition();
                tps[j]->getPayload(verifyPayloadData, offset);
                offset += tps[j]->getPayloadLength();
            }
        }
    }

    for (int32_t i = 0; i < numTerms; ++i) {
        tps[i]->close();
    }

    EXPECT_TRUE(payloadData.equals(verifyPayloadData));

    // test lazy skipping
    TermPositionsPtr tp = reader->termPositions(terms[0]);
    tp->next();
    tp->nextPosition();
    // now we don't read this payload
    tp->nextPosition();
    EXPECT_EQ(1, tp->getPayloadLength());
    ByteArray payload = tp->getPayload(ByteArray(), 0);
    EXPECT_EQ(payload[0], payloadData[numTerms]);
    tp->nextPosition();

    // we don't read this payload and skip to a different document
    tp->skipTo(5);
    tp->nextPosition();
    EXPECT_EQ(1, tp->getPayloadLength());
    payload = tp->getPayload(ByteArray(), 0);
    EXPECT_EQ(payload[0], payloadData[5 * numTerms]);

    // Test different lengths at skip points
    tp->seek(terms[1]);
    tp->next();
    tp->nextPosition();
    EXPECT_EQ(1, tp->getPayloadLength());
    tp->skipTo(skipInterval - 1);
    tp->nextPosition();
    EXPECT_EQ(1, tp->getPayloadLength());
    tp->skipTo(2 * skipInterval - 1);
    tp->nextPosition();
    EXPECT_EQ(1, tp->getPayloadLength());
    tp->skipTo(3 * skipInterval - 1);
    tp->nextPosition();
    EXPECT_EQ(3 * skipInterval - 2 * numDocs - 1, tp->getPayloadLength());

    // Test multiple call of getPayload()
    tp->getPayload(ByteArray(), 0);

    // it is forbidden to call getPayload() more than once without calling nextPosition()
    try {
        tp->getPayload(ByteArray(), 0);
    } catch (IOException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IO)(e));
    }

    reader->close();

    // test long payload
    analyzer = newLucene<PayloadAnalyzer>();
    writer = newLucene<IndexWriter>(dir, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);
    String singleTerm = L"lucene";

    d = newLucene<Document>();
    d->add(newLucene<Field>(fieldName, singleTerm, Field::STORE_NO, Field::INDEX_ANALYZED));
    // add a payload whose length is greater than the buffer size of BufferedIndexOutput
    payloadData = generateRandomData(2000);
    analyzer->setPayloadData(fieldName, payloadData, 100, 1500);
    writer->addDocument(d);

    writer->optimize();
    // flush
    writer->close();

    reader = IndexReader::open(dir, true);
    tp = reader->termPositions(newLucene<Term>(fieldName, singleTerm));
    tp->next();
    tp->nextPosition();

    verifyPayloadData.resize(tp->getPayloadLength());
    tp->getPayload(verifyPayloadData, 0);
    ByteArray portion(ByteArray::newInstance(1500));
    MiscUtils::arrayCopy(payloadData.get(), 100, portion.get(), 0, 1500);

    EXPECT_TRUE(portion.equals(verifyPayloadData));

    reader->close();
}

/// Tests if payloads are correctly stored and loaded using both RamDirectory and FSDirectory
TEST_F(PayloadsTest, testPayloadsEncoding) {
    // first perform the test using a RAMDirectory
    DirectoryPtr dir = newLucene<RAMDirectory>();
    encodingTest(dir);

    // now use a FSDirectory and repeat same test
    String dirName(FileUtils::joinPath(getTempDir(), L"test_payloads"));
    dir = FSDirectory::open(dirName);
    encodingTest(dir);
    FileUtils::removeDirectory(dirName);
}

namespace TestThreadSafety {

DECLARE_SHARED_PTR(ByteArrayPool)

class ByteArrayPool : public LuceneObject {
public:
    ByteArrayPool(int32_t capacity, int32_t size) {
        pool = Collection<ByteArray>::newInstance();
        for (int32_t i = 0; i < capacity; ++i) {
            pool.add(ByteArray::newInstance(size));
        }
    }

    virtual ~ByteArrayPool() {
    }

    LUCENE_CLASS(ByteArrayPool);

public:
    Collection<ByteArray> pool;

public:
    String bytesToString(ByteArray bytes) {
        SyncLock syncLock(this);
        return Base64::encode(bytes);
    }

    ByteArray get() {
        SyncLock syncLock(this);
        return pool.removeFirst();
    }

    void release(ByteArray b) {
        SyncLock syncLock(this);
        pool.add(b);
    }

    int32_t size() {
        SyncLock syncLock(this);
        return pool.size();
    }
};

class PoolingPayloadTokenStream : public TokenStream {
public:
    PoolingPayloadTokenStream(const ByteArrayPoolPtr& pool) {
        this->pool = pool;
        payload = pool->get();
        generateRandomData(payload);
        term = pool->bytesToString(payload);
        first = true;
        payloadAtt = addAttribute<PayloadAttribute>();
        termAtt = addAttribute<TermAttribute>();
    }

    virtual ~PoolingPayloadTokenStream() {
    }

    LUCENE_CLASS(PoolingPayloadTokenStream);

public:
    ByteArray payload;
    bool first;
    ByteArrayPoolPtr pool;
    String term;

    TermAttributePtr termAtt;
    PayloadAttributePtr payloadAtt;

public:
    virtual bool incrementToken() {
        if (!first) {
            return false;
        }
        first = false;
        clearAttributes();
        termAtt->setTermBuffer(term);
        payloadAtt->setPayload(newLucene<Payload>(payload));
        return true;
    }

    virtual void close() {
        pool->release(payload);
    }
};

class IngesterThread : public LuceneThread {
public:
    IngesterThread(int32_t numDocs, const ByteArrayPoolPtr& pool, const IndexWriterPtr& writer) {
        this->numDocs = numDocs;
        this->pool = pool;
        this->writer = writer;
    }

    virtual ~IngesterThread() {
    }

    LUCENE_CLASS(IngesterThread);

protected:
    int32_t numDocs;
    ByteArrayPoolPtr pool;
    IndexWriterPtr writer;

public:
    virtual void run() {
        try {
            for (int32_t j = 0; j < numDocs; ++j) {
                DocumentPtr d = newLucene<Document>();
                d->add(newLucene<Field>(L"test", newLucene<PoolingPayloadTokenStream>(pool)));
                writer->addDocument(d);
            }
        } catch (LuceneException& e) {
            FAIL() << "Unexpected exception: " << e.getError();
        }
    }
};

}

TEST_F(PayloadsTest, testThreadSafety) {
    int32_t numThreads = 5;
    int32_t numDocs = 50;
    TestThreadSafety::ByteArrayPoolPtr pool = newLucene<TestThreadSafety::ByteArrayPool>(numThreads, 5);

    DirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);

    Collection<LuceneThreadPtr> ingesters = Collection<LuceneThreadPtr>::newInstance(numThreads);
    for (int32_t i = 0; i < numThreads; ++i) {
        ingesters[i] = newLucene<TestThreadSafety::IngesterThread>(numDocs, pool, writer);
        ingesters[i]->start();
    }

    for (int32_t i = 0; i < numThreads; ++i) {
        ingesters[i]->join();
    }

    writer->close();
    IndexReaderPtr reader = IndexReader::open(dir, true);
    TermEnumPtr terms = reader->terms();
    while (terms->next()) {
        TermPositionsPtr tp = reader->termPositions(terms->term());
        while (tp->next()) {
            int32_t freq = tp->freq();
            for (int32_t i = 0; i < freq; ++i) {
                tp->nextPosition();
                EXPECT_EQ(pool->bytesToString(tp->getPayload(ByteArray::newInstance(5), 0)), terms->term()->text());
            }
        }
        tp->close();
    }

    terms->close();
    reader->close();

    EXPECT_EQ(pool->size(), numThreads);
}
