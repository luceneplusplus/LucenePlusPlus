/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "RAMDirectory.h"
#include "Document.h"
#include "FieldInfos.h"
#include "DocHelper.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "FieldsReader.h"
#include "Field.h"
#include "SetBasedFieldSelector.h"
#include "LoadFirstFieldSelector.h"
#include "FSDirectory.h"
#include "BufferedIndexInput.h"
#include "IndexReader.h"
#include "MiscUtils.h"
#include "FileUtils.h"

using namespace Lucene;

class FieldsReaderTest : public LuceneTestFixture, public DocHelper {
public:
    FieldsReaderTest() {
        dir = newLucene<RAMDirectory>();
        testDoc = newLucene<Document>();
        fieldInfos = newLucene<FieldInfos>();
        DocHelper::setupDoc(testDoc);
        fieldInfos->add(testDoc);
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        writer->setUseCompoundFile(false);
        writer->addDocument(testDoc);
        writer->close();
    }

    virtual ~FieldsReaderTest() {
    }

protected:
    RAMDirectoryPtr dir;
    DocumentPtr testDoc;
    FieldInfosPtr fieldInfos;

    static String TEST_SEGMENT_NAME;
};

String FieldsReaderTest::TEST_SEGMENT_NAME = L"_0";

DECLARE_SHARED_PTR(FaultyFSDirectory)
DECLARE_SHARED_PTR(FaultyIndexInput)

class FaultyIndexInput : public BufferedIndexInput {
public:
    FaultyIndexInput(const IndexInputPtr& delegate) {
        this->delegate = delegate;
        count = 0;
    }

    virtual ~FaultyIndexInput() {
    }

    LUCENE_CLASS(FaultyIndexInput);

public:
    IndexInputPtr delegate;
    static bool doFail;
    int32_t count;

public:
    virtual void readInternal(uint8_t* b, int32_t offset, int32_t length) {
        simOutage();
        delegate->readBytes(b, offset, length);
    }

    virtual void seekInternal(int64_t pos) {
        delegate->seek(pos);
    }

    virtual int64_t length() {
        return delegate->length();
    }

    virtual void close() {
        delegate->close();
    }

    virtual LuceneObjectPtr clone(const LuceneObjectPtr& other = LuceneObjectPtr()) {
        return newLucene<FaultyIndexInput>(boost::dynamic_pointer_cast<IndexInput>(delegate->clone()));
    }

protected:
    void simOutage() {
        if (doFail && count++ % 2 == 1) {
            boost::throw_exception(IOException(L"Simulated network outage"));
        }
    }
};

bool FaultyIndexInput::doFail = false;

class FaultyFSDirectory : public Directory {
public:
    FaultyFSDirectory(const String& dir) {
        fsDir = FSDirectory::open(dir);
        lockFactory = fsDir->getLockFactory();
    }

    virtual ~FaultyFSDirectory() {
    }

    LUCENE_CLASS(FaultyFSDirectory);

public:
    FSDirectoryPtr fsDir;

public:
    virtual IndexInputPtr openInput(const String& name) {
        return newLucene<FaultyIndexInput>(fsDir->openInput(name));
    }

    virtual HashSet<String> listAll() {
        return fsDir->listAll();
    }

    virtual bool fileExists(const String& name) {
        return fsDir->fileExists(name);
    }

    virtual uint64_t fileModified(const String& name) {
        return fsDir->fileModified(name);
    }

    virtual void touchFile(const String& name) {
        fsDir->touchFile(name);
    }

    virtual void deleteFile(const String& name) {
        fsDir->deleteFile(name);
    }

    virtual int64_t fileLength(const String& name) {
        return fsDir->fileLength(name);
    }

    virtual IndexOutputPtr createOutput(const String& name) {
        return fsDir->createOutput(name);
    }

    virtual void close() {
        fsDir->close();
    }
};

static void checkSizeEquals(int32_t size, const uint8_t* sizebytes) {
    EXPECT_EQ((uint8_t)MiscUtils::unsignedShift(size, 24), sizebytes[0]);
    EXPECT_EQ((uint8_t)MiscUtils::unsignedShift(size, 16), sizebytes[1]);
    EXPECT_EQ((uint8_t)MiscUtils::unsignedShift(size, 8), sizebytes[2]);
    EXPECT_EQ((uint8_t)size, sizebytes[3]);
}

TEST_F(FieldsReaderTest, testFieldsReader) {
    EXPECT_TRUE(dir);
    EXPECT_TRUE(fieldInfos);
    FieldsReaderPtr reader = newLucene<FieldsReader>(dir, TEST_SEGMENT_NAME, fieldInfos);
    EXPECT_TRUE(reader);
    EXPECT_TRUE(reader->size() == 1);
    DocumentPtr doc = reader->doc(0, FieldSelectorPtr());
    EXPECT_TRUE(doc);
    EXPECT_TRUE(doc->getField(DocHelper::TEXT_FIELD_1_KEY));

    FieldablePtr field = doc->getField(DocHelper::TEXT_FIELD_2_KEY);
    EXPECT_TRUE(field);
    EXPECT_TRUE(field->isTermVectorStored());

    EXPECT_TRUE(field->isStoreOffsetWithTermVector());
    EXPECT_TRUE(field->isStorePositionWithTermVector());
    EXPECT_TRUE(!field->getOmitNorms());
    EXPECT_TRUE(!field->getOmitTermFreqAndPositions());

    field = doc->getField(DocHelper::TEXT_FIELD_3_KEY);
    EXPECT_TRUE(field);
    EXPECT_TRUE(!field->isTermVectorStored());
    EXPECT_TRUE(!field->isStoreOffsetWithTermVector());
    EXPECT_TRUE(!field->isStorePositionWithTermVector());
    EXPECT_TRUE(field->getOmitNorms());
    EXPECT_TRUE(!field->getOmitTermFreqAndPositions());

    field = doc->getField(DocHelper::NO_TF_KEY);
    EXPECT_TRUE(field);
    EXPECT_TRUE(!field->isTermVectorStored());
    EXPECT_TRUE(!field->isStoreOffsetWithTermVector());
    EXPECT_TRUE(!field->isStorePositionWithTermVector());
    EXPECT_TRUE(!field->getOmitNorms());
    EXPECT_TRUE(field->getOmitTermFreqAndPositions());
    reader->close();
}

TEST_F(FieldsReaderTest, testLazyFields) {
    EXPECT_TRUE(dir);
    EXPECT_TRUE(fieldInfos);
    FieldsReaderPtr reader = newLucene<FieldsReader>(dir, TEST_SEGMENT_NAME, fieldInfos);
    EXPECT_TRUE(reader);
    EXPECT_TRUE(reader->size() == 1);
    HashSet<String> loadFieldNames = HashSet<String>::newInstance();
    loadFieldNames.add(DocHelper::TEXT_FIELD_1_KEY);
    loadFieldNames.add(DocHelper::TEXT_FIELD_UTF1_KEY);
    HashSet<String> lazyFieldNames = HashSet<String>::newInstance();
    lazyFieldNames.add(DocHelper::LARGE_LAZY_FIELD_KEY);
    lazyFieldNames.add(DocHelper::LAZY_FIELD_KEY);
    lazyFieldNames.add(DocHelper::LAZY_FIELD_BINARY_KEY);
    lazyFieldNames.add(DocHelper::TEXT_FIELD_UTF2_KEY);
    SetBasedFieldSelectorPtr fieldSelector = newLucene<SetBasedFieldSelector>(loadFieldNames, lazyFieldNames);
    DocumentPtr doc = reader->doc(0, fieldSelector);
    EXPECT_TRUE(doc);
    FieldablePtr field = doc->getFieldable(DocHelper::LAZY_FIELD_KEY);
    EXPECT_TRUE(field);
    EXPECT_TRUE(field->isLazy());
    String value = field->stringValue();
    EXPECT_TRUE(!value.empty());
    EXPECT_EQ(value, DocHelper::LAZY_FIELD_TEXT);
    field = doc->getFieldable(DocHelper::TEXT_FIELD_1_KEY);
    EXPECT_TRUE(field);
    EXPECT_TRUE(!field->isLazy());
    field = doc->getFieldable(DocHelper::TEXT_FIELD_UTF1_KEY);
    EXPECT_TRUE(field);
    EXPECT_TRUE(!field->isLazy());
    EXPECT_EQ(field->stringValue(), DocHelper::FIELD_UTF1_TEXT);

    field = doc->getFieldable(DocHelper::TEXT_FIELD_UTF2_KEY);
    EXPECT_TRUE(field);
    EXPECT_TRUE(field->isLazy());
    EXPECT_EQ(field->stringValue(), DocHelper::FIELD_UTF2_TEXT);

    field = doc->getFieldable(DocHelper::LAZY_FIELD_BINARY_KEY);
    EXPECT_TRUE(field);
    EXPECT_TRUE(field->stringValue().empty());

    ByteArray bytes = field->getBinaryValue();
    EXPECT_TRUE(bytes);
    EXPECT_EQ(DocHelper::LAZY_FIELD_BINARY_BYTES.size(), bytes.size());
    EXPECT_TRUE(bytes);
    EXPECT_TRUE(bytes.equals(DocHelper::LAZY_FIELD_BINARY_BYTES));
}

TEST_F(FieldsReaderTest, testLazyFieldsAfterClose) {
    EXPECT_TRUE(dir);
    EXPECT_TRUE(fieldInfos);
    FieldsReaderPtr reader = newLucene<FieldsReader>(dir, TEST_SEGMENT_NAME, fieldInfos);
    EXPECT_TRUE(reader);
    EXPECT_TRUE(reader->size() == 1);
    HashSet<String> loadFieldNames = HashSet<String>::newInstance();
    loadFieldNames.add(DocHelper::TEXT_FIELD_1_KEY);
    loadFieldNames.add(DocHelper::TEXT_FIELD_UTF1_KEY);
    HashSet<String> lazyFieldNames = HashSet<String>::newInstance();
    lazyFieldNames.add(DocHelper::LARGE_LAZY_FIELD_KEY);
    lazyFieldNames.add(DocHelper::LAZY_FIELD_KEY);
    lazyFieldNames.add(DocHelper::LAZY_FIELD_BINARY_KEY);
    lazyFieldNames.add(DocHelper::TEXT_FIELD_UTF2_KEY);
    SetBasedFieldSelectorPtr fieldSelector = newLucene<SetBasedFieldSelector>(loadFieldNames, lazyFieldNames);
    DocumentPtr doc = reader->doc(0, fieldSelector);
    EXPECT_TRUE(doc);
    FieldablePtr field = doc->getFieldable(DocHelper::LAZY_FIELD_KEY);
    EXPECT_TRUE(field);
    EXPECT_TRUE(field->isLazy());
    reader->close();
    try {
        field->stringValue();
    } catch (AlreadyClosedException& e) {
        EXPECT_TRUE(check_exception(LuceneException::AlreadyClosed)(e));
    }
}

TEST_F(FieldsReaderTest, testLoadFirst) {
    EXPECT_TRUE(dir);
    EXPECT_TRUE(fieldInfos);
    FieldsReaderPtr reader = newLucene<FieldsReader>(dir, TEST_SEGMENT_NAME, fieldInfos);
    EXPECT_TRUE(reader);
    EXPECT_TRUE(reader->size() == 1);
    LoadFirstFieldSelectorPtr fieldSelector = newLucene<LoadFirstFieldSelector>();
    DocumentPtr doc = reader->doc(0, fieldSelector);
    EXPECT_TRUE(doc);
    int32_t count = 0;
    Collection<FieldablePtr> fields = doc->getFields();
    for (Collection<FieldablePtr>::iterator field = fields.begin(); field != fields.end(); ++field) {
        EXPECT_TRUE(*field);
        String sv = (*field)->stringValue();
        EXPECT_TRUE(!sv.empty());
        ++count;
    }
    EXPECT_EQ(count, 1);
}

/// Not really a test per se, but we should have some way of assessing whether this is worthwhile.
/// Must test using a File based directory.
TEST_F(FieldsReaderTest, testLazyPerformance) {
    String path(FileUtils::joinPath(getTempDir(), L"lazyDir"));
    FSDirectoryPtr tmpDir = FSDirectory::open(path);
    EXPECT_TRUE(tmpDir);

    IndexWriterPtr writer = newLucene<IndexWriter>(tmpDir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setUseCompoundFile(false);
    writer->addDocument(testDoc);
    writer->close();

    EXPECT_TRUE(fieldInfos);
    FieldsReaderPtr reader;
    int64_t lazyTime = 0;
    int64_t regularTime = 0;
    int32_t length = 50;
    HashSet<String> lazyFieldNames = HashSet<String>::newInstance();
    lazyFieldNames.add(DocHelper::LARGE_LAZY_FIELD_KEY);
    SetBasedFieldSelectorPtr fieldSelector = newLucene<SetBasedFieldSelector>(HashSet<String>::newInstance(), lazyFieldNames);

    for (int32_t i = 0; i < length; ++i) {
        reader = newLucene<FieldsReader>(tmpDir, TEST_SEGMENT_NAME, fieldInfos);
        EXPECT_TRUE(reader);
        EXPECT_TRUE(reader->size() == 1);

        DocumentPtr doc = reader->doc(0, FieldSelectorPtr()); // Load all of them
        EXPECT_TRUE(doc);
        FieldablePtr field = doc->getFieldable(DocHelper::LARGE_LAZY_FIELD_KEY);
        EXPECT_TRUE(!field->isLazy());
        int64_t start = MiscUtils::currentTimeMillis();
        String value = field->stringValue();
        int64_t finish = MiscUtils::currentTimeMillis(); // ~ 0ms
        EXPECT_TRUE(!value.empty());
        EXPECT_TRUE(field);
        regularTime += (finish - start);
        reader->close();
        reader.reset();
        doc.reset();
        reader = newLucene<FieldsReader>(tmpDir, TEST_SEGMENT_NAME, fieldInfos);
        doc = reader->doc(0, fieldSelector);
        field = doc->getFieldable(DocHelper::LARGE_LAZY_FIELD_KEY);
        EXPECT_TRUE(field->isLazy());
        start = MiscUtils::currentTimeMillis();
        value = field->stringValue();
        finish = MiscUtils::currentTimeMillis(); // ~ 50 - 70ms
        EXPECT_TRUE(!value.empty());
        lazyTime += (finish - start);
        reader->close();
    }

    // std::cout << "Average Non-lazy time (should be very close to zero): " << (regularTime / length) << " ms for " << length << " reads";
    // std::cout << "Average Lazy Time (should be greater than zero): " << (lazyTime / length) << " ms for " << length << " reads";

    FileUtils::removeDirectory(path);
}

namespace TestLoadSize {

DECLARE_SHARED_PTR(TestableFieldSelector)

class TestableFieldSelector : public FieldSelector {
public:
    virtual ~TestableFieldSelector() {
    }

    LUCENE_CLASS(TestableFieldSelector);

public:
    virtual FieldSelectorResult accept(const String& fieldName) {
        if (fieldName == DocHelper::TEXT_FIELD_1_KEY || fieldName == DocHelper::LAZY_FIELD_BINARY_KEY) {
            return FieldSelector::SELECTOR_SIZE;
        } else if (fieldName == DocHelper::TEXT_FIELD_3_KEY) {
            return FieldSelector::SELECTOR_LOAD;
        } else {
            return FieldSelector::SELECTOR_NO_LOAD;
        }
    }
};

}

TEST_F(FieldsReaderTest, testLoadSize) {
    FieldsReaderPtr reader = newLucene<FieldsReader>(dir, TEST_SEGMENT_NAME, fieldInfos);
    DocumentPtr doc = reader->doc(0, newLucene<TestLoadSize::TestableFieldSelector>());

    FieldablePtr f1 = doc->getFieldable(DocHelper::TEXT_FIELD_1_KEY);
    FieldablePtr f3 = doc->getFieldable(DocHelper::TEXT_FIELD_3_KEY);
    FieldablePtr fb = doc->getFieldable(DocHelper::LAZY_FIELD_BINARY_KEY);
    EXPECT_TRUE(f1->isBinary());
    EXPECT_TRUE(!f3->isBinary());
    EXPECT_TRUE(fb->isBinary());
    checkSizeEquals(2 * String(DocHelper::FIELD_1_TEXT).length(), f1->getBinaryValue().get());
    EXPECT_EQ(DocHelper::FIELD_3_TEXT, f3->stringValue());
    checkSizeEquals(DocHelper::LAZY_FIELD_BINARY_BYTES.size(), fb->getBinaryValue().get());

    reader->close();
}

TEST_F(FieldsReaderTest, testExceptions) {
    String indexDir(FileUtils::joinPath(getTempDir(), L"testfieldswriterexceptions"));

    LuceneException finally;
    try {
        DirectoryPtr dir = newLucene<FaultyFSDirectory>(indexDir);
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        for (int32_t i = 0; i < 2; ++i) {
            writer->addDocument(testDoc);
        }
        writer->optimize();
        writer->close();

        IndexReaderPtr reader = IndexReader::open(dir, true);

        FaultyIndexInput::doFail = true;

        bool exc = false;

        for (int32_t i = 0; i < 2; ++i) {
            try {
                reader->document(i);
            } catch (IOException&) {
                exc = true; // expected
            }
            try {
                reader->document(i);
            } catch (IOException&) {
                exc = true; // expected
            }
        }
        EXPECT_TRUE(exc);
        reader->close();
        dir->close();
    } catch (LuceneException& e) {
        finally = e;
    }
    FileUtils::removeDirectory(indexDir);
    finally.throwException();
}
