/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <boost/algorithm/string.hpp>
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "FSDirectory.h"
#include "IndexReader.h"
#include "IndexWriter.h"
#include "IndexSearcher.h"
#include "WhitespaceAnalyzer.h"
#include "Term.h"
#include "Document.h"
#include "Field.h"
#include "FieldInfos.h"
#include "FieldInfo.h"
#include "ReaderUtil.h"
#include "SegmentReader.h"
#include "FieldsReader.h"
#include "FieldSelector.h"
#include "TermQuery.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "CompoundFileReader.h"
#include "NumericField.h"
#include "FileUtils.h"

using namespace Lucene;

typedef LuceneTestFixture BackwardsCompatibilityTest;

/// Verify we can read the pre-2.1 file format, do searches against it, and add documents to it.

static String fullDir(const String& dirName) {
    return FileUtils::joinPath(getTempDir(), dirName);
}

static void rmDir(const String& dirName) {
    FileUtils::removeDirectory(FileUtils::joinPath(getTempDir(), dirName));
}

static void addDoc(const IndexWriterPtr& writer, int32_t id) {
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", L"aaa", Field::STORE_NO, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"id", StringUtils::toString(id), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    const uint8_t utf8Field[] = {0x4c, 0x75, 0xf0, 0x9d, 0x84, 0x9e, 0x63, 0x65, 0xf0, 0x9d, 0x85, 0xa0, 0x6e, 0x65,
                                 0x20, 0x00, 0x20, 0xe2, 0x98, 0xa0, 0x20, 0x61, 0x62, 0xf1, 0x95, 0xb0, 0x97, 0x63, 0x64
                                };

    const uint8_t utf8Field2[] = {0x66, 0x69, 0x65, 0xe2, 0xb1, 0xb7, 0x6c, 0x64};

    doc->add(newLucene<Field>(L"autf8", UTF8_TO_STRING(utf8Field), Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    doc->add(newLucene<Field>(L"utf8", UTF8_TO_STRING(utf8Field), Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    doc->add(newLucene<Field>(L"content2", L"here is more content with aaa aaa aaa", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    doc->add(newLucene<Field>(UTF8_TO_STRING(utf8Field2), L"field with non-ascii name", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));

    // add numeric fields, to test if flex preserves encoding
    doc->add(newLucene<NumericField>(L"trieInt", 4)->setIntValue(id));
    doc->add(newLucene<NumericField>(L"trieLong", 4)->setLongValue(id));

    writer->addDocument(doc);
}

static void addNoProxDoc(const IndexWriterPtr& writer) {
    DocumentPtr doc = newLucene<Document>();
    FieldPtr f = newLucene<Field>(L"content3", L"aaa", Field::STORE_YES, Field::INDEX_ANALYZED);
    f->setOmitTermFreqAndPositions(true);
    doc->add(f);
    f = newLucene<Field>(L"content4", L"aaa", Field::STORE_YES, Field::INDEX_NO);
    f->setOmitTermFreqAndPositions(true);
    doc->add(f);
    writer->addDocument(doc);
}

static void createIndex(const String& dirName, bool doCFS) {
    FileUtils::removeDirectory(dirName);
    String fullName(fullDir(dirName));

    DirectoryPtr dir = FSDirectory::open(fullName);
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setUseCompoundFile(doCFS);
    writer->setMaxBufferedDocs(10);

    for (int32_t i = 0; i < 35; ++i) {
        addDoc(writer, i);
    }

    EXPECT_EQ(35, writer->maxDoc());
    writer->close();

    // open fresh writer so we get no prx file in the added segment
    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    writer->setUseCompoundFile(doCFS);
    writer->setMaxBufferedDocs(10);
    addNoProxDoc(writer);
    writer->close();

    // Delete one doc so we get a .del file:
    IndexReaderPtr reader = IndexReader::open(dir, false);
    TermPtr searchTerm = newLucene<Term>(L"id", L"7");
    int32_t delCount = reader->deleteDocuments(searchTerm);
    EXPECT_EQ(1, delCount); // delete the right number of documents

    // Set one norm so we get a .s0 file:
    reader->setNorm(21, L"content", 1.5);
    reader->close();
}

static void copyIndex(const String& dirName) {
    String dirSource(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"legacyindex"), dirName));
    String dirDest(FileUtils::joinPath(getTempDir(), dirName));
    FileUtils::copyDirectory(dirSource, dirDest);
}

static const wchar_t* oldNames[] = {
    L"19.cfs",
    L"19.nocfs",
    L"20.cfs",
    L"20.nocfs",
    L"21.cfs",
    L"21.nocfs",
    L"22.cfs",
    L"22.nocfs",
    L"23.cfs",
    L"23.nocfs",
    L"24.cfs",
    L"24.nocfs",
    L"29.cfs",
    L"29.nocfs"
};
static const int32_t oldNamesLength = SIZEOF_ARRAY(oldNames);

namespace CheckCompressedFields {

class CompressedFieldSelector : public FieldSelector {
public:
    virtual ~CompressedFieldSelector() {
    };

    LUCENE_CLASS(CompressedFieldSelector);

public:
    virtual FieldSelectorResult accept(const String& fieldName) {
        return fieldName == L"compressed" ? FieldSelector::SELECTOR_SIZE : FieldSelector::SELECTOR_LOAD;
    }
};

}

void checkCompressedFields29(const DirectoryPtr& dir, bool shouldStillBeCompressed) {
    int32_t count = 0;
    static String TEXT_TO_COMPRESS = L"this is a compressed field and should appear in 3.0 as an uncompressed field after merge";
    int32_t TEXT_PLAIN_LENGTH = TEXT_TO_COMPRESS.length() * 2;
    // FieldSelectorResult.SIZE returns 2*number_of_chars for String fields
    static uint8_t BINARY_TO_COMPRESS[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    int32_t BINARY_PLAIN_LENGTH = SIZEOF_ARRAY(BINARY_TO_COMPRESS);

    IndexReaderPtr reader = IndexReader::open(dir, true);

    LuceneException finally;
    try {
        // look into sub readers and check if raw merge is on/off
        Collection<IndexReaderPtr> readers = Collection<IndexReaderPtr>::newInstance();
        ReaderUtil::gatherSubReaders(readers, reader);

        for (Collection<IndexReaderPtr>::iterator ir = readers.begin(); ir != readers.end(); ++ir) {
            FieldsReaderPtr fr = boost::dynamic_pointer_cast<SegmentReader>(*ir)->getFieldsReader();
            // for a 2.9 index, FieldsReader.canReadRawDocs() must be false and other way round for a trunk index
            EXPECT_NE(shouldStillBeCompressed, fr->canReadRawDocs());
        }

        // test that decompression works correctly
        for (int32_t i = 0; i < reader->maxDoc(); ++i) {
            if (!reader->isDeleted(i)) {
                DocumentPtr d = reader->document(i);
                if (!d->get(L"content3").empty()) {
                    continue;
                }
                ++count;
                FieldablePtr compressed = d->getFieldable(L"compressed");
                if (StringUtils::toInt(d->get(L"id")) % 2 == 0) {
                    EXPECT_TRUE(!compressed->isBinary());
                    EXPECT_EQ(TEXT_TO_COMPRESS, compressed->stringValue()); // correctly decompressed string
                } else {
                    EXPECT_TRUE(compressed->isBinary());
                    EXPECT_TRUE(std::memcmp(BINARY_TO_COMPRESS, compressed->getBinaryValue().get(), BINARY_PLAIN_LENGTH) == 0); // correctly decompressed binary
                }
            }
        }

        // check if field was decompressed after optimize
        for (int32_t i = 0; i < reader->maxDoc(); ++i) {
            if (!reader->isDeleted(i)) {
                DocumentPtr d = reader->document(i, newLucene<CheckCompressedFields::CompressedFieldSelector>());
                if (!d->get(L"content3").empty()) {
                    continue;
                }
                ++count;
                // read the size from the binary value using DataInputStream (this prevents us from doing the shift ops ourselves)
                uint8_t* ds = d->getFieldable(L"compressed")->getBinaryValue().get();
                int32_t actualSize = ((ds[0] & 0xff) << 24) + ((ds[1] & 0xff) << 16) + ((ds[2] & 0xff) << 8) + (ds[3] & 0xff);
                int32_t compressedSize = StringUtils::toInt(d->get(L"compressedSize"));
                bool binary = (StringUtils::toInt(d->get(L"id")) % 2 > 0);
                int32_t shouldSize = shouldStillBeCompressed ? compressedSize : (binary ? BINARY_PLAIN_LENGTH : TEXT_PLAIN_LENGTH);
                EXPECT_EQ(shouldSize, actualSize);
                if (!shouldStillBeCompressed) {
                    EXPECT_NE(compressedSize, actualSize);
                }

            }
        }
        EXPECT_EQ(34 * 2, count); // correct number of tests
    } catch (LuceneException& e) {
        finally = e;
    }
    reader->close();
    finally.throwException();
}

static void testHits(Collection<ScoreDocPtr> hits, int32_t expectedCount, const IndexReaderPtr& reader) {
    int32_t hitCount = hits.size();
    EXPECT_EQ(expectedCount, hitCount);
    for (int32_t i = 0; i < hitCount; ++i) {
        reader->document(hits[i]->doc);
        reader->getTermFreqVectors(hits[i]->doc);
    }
}

static void searchIndex(const String& dirName, const String& oldName) {
    String dirPath = fullDir(dirName);

    DirectoryPtr dir = FSDirectory::open(dirPath);
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, true);
    IndexReaderPtr reader = searcher->getIndexReader();

    checkIndex(dir);

    const uint8_t utf8Field[] = {0x4c, 0x75, 0xf0, 0x9d, 0x84, 0x9e, 0x63, 0x65, 0xf0, 0x9d, 0x85, 0xa0, 0x6e, 0x65,
                                 0x20, 0x00, 0x20, 0xe2, 0x98, 0xa0, 0x20, 0x61, 0x62, 0xf1, 0x95, 0xb0, 0x97, 0x63, 0x64
                                };

    const uint8_t utf8Field2[] = {0x66, 0x69, 0x65, 0xe2, 0xb1, 0xb7, 0x6c, 0x64};

    const uint8_t utf8Lucene[] = {0x4c, 0x75, 0xf0, 0x9d, 0x84, 0x9e, 0x63, 0x65, 0xf0, 0x9d, 0x85, 0xa0, 0x6e, 0x65};

    const uint8_t utf8Abcd[] = {0x61, 0x62, 0xf1, 0x95, 0xb0, 0x97, 0x63, 0x64};

    const wchar_t _zeroField[] = {0x0000};
    String zeroField(_zeroField, SIZEOF_ARRAY(_zeroField));

    for (int32_t i = 0; i < 35; ++i) {
        if (!reader->isDeleted(i)) {
            DocumentPtr d = reader->document(i);
            Collection<FieldablePtr> fields = d->getFields();
            if (!boost::starts_with(oldName, L"19.") && !boost::starts_with(oldName, L"20.") &&
                    !boost::starts_with(oldName, L"21.") && !boost::starts_with(oldName, L"22.")) {
                if (!d->getField(L"content3")) {
                    int32_t numFields = boost::starts_with(oldName, L"29.") ? 7 : 5;
                    EXPECT_EQ(numFields, fields.size());

                    FieldPtr f = boost::dynamic_pointer_cast<Field>(d->getField(L"id"));
                    EXPECT_EQ(StringUtils::toString(i), f->stringValue());

                    f = boost::dynamic_pointer_cast<Field>(d->getField(L"utf8"));
                    EXPECT_EQ(UTF8_TO_STRING(utf8Field), f->stringValue());

                    f = boost::dynamic_pointer_cast<Field>(d->getField(L"autf8"));
                    EXPECT_EQ(UTF8_TO_STRING(utf8Field), f->stringValue());

                    f = boost::dynamic_pointer_cast<Field>(d->getField(L"content2"));
                    EXPECT_EQ(L"here is more content with aaa aaa aaa", f->stringValue());

                    f = boost::dynamic_pointer_cast<Field>(d->getField(UTF8_TO_STRING(utf8Field2)));
                    EXPECT_EQ(L"field with non-ascii name", f->stringValue());
                }
            }
        } else {
            // Only ID 7 is deleted
            EXPECT_EQ(7, i);
        }
    }

    Collection<ScoreDocPtr> hits = searcher->search(newLucene<TermQuery>(newLucene<Term>(L"content", L"aaa")), FilterPtr(), 1000)->scoreDocs;

    // First document should be #21 since it's norm was increased
    DocumentPtr d = searcher->doc(hits[0]->doc);
    EXPECT_EQ(L"21", d->get(L"id")); // get the right document first

    testHits(hits, 34, searcher->getIndexReader());

    if (!boost::starts_with(oldName, L"19.") && !boost::starts_with(oldName, L"20.") &&
            !boost::starts_with(oldName, L"21.") && !boost::starts_with(oldName, L"22.")) {
        // Test on indices >= 2.3
        hits = searcher->search(newLucene<TermQuery>(newLucene<Term>(L"utf8", zeroField)), FilterPtr(), 1000)->scoreDocs;
        EXPECT_EQ(34, hits.size());
        hits = searcher->search(newLucene<TermQuery>(newLucene<Term>(L"utf8", UTF8_TO_STRING(utf8Lucene))), FilterPtr(), 1000)->scoreDocs;
        EXPECT_EQ(34, hits.size());
        hits = searcher->search(newLucene<TermQuery>(newLucene<Term>(L"utf8", UTF8_TO_STRING(utf8Abcd))), FilterPtr(), 1000)->scoreDocs;
        EXPECT_EQ(34, hits.size());
    }

    searcher->close();
    dir->close();
}

// Open pre-lockless index, add docs, do a delete and setNorm, and search
static void changeIndexNoAdds(const String& dirName) {
    String dirPath = fullDir(dirName);

    DirectoryPtr dir = FSDirectory::open(dirPath);

    // make sure searching sees right # hits
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, true);
    Collection<ScoreDocPtr> hits = searcher->search(newLucene<TermQuery>(newLucene<Term>(L"content", L"aaa")), FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(34, hits.size()); // number of hits
    DocumentPtr d = searcher->doc(hits[0]->doc);
    EXPECT_EQ(L"21", d->get(L"id")); // first document
    searcher->close();

    // make sure we can do a delete & setNorm against this pre-lockless segment
    IndexReaderPtr reader = IndexReader::open(dir, false);
    TermPtr searchTerm = newLucene<Term>(L"id", L"6");
    int32_t delCount = reader->deleteDocuments(searchTerm);
    EXPECT_EQ(1, delCount); // delete count
    reader->setNorm(22, L"content", 2.0);
    reader->close();

    // make sure they "took"
    searcher = newLucene<IndexSearcher>(dir, true);
    hits = searcher->search(newLucene<TermQuery>(newLucene<Term>(L"content", L"aaa")), FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(33, hits.size()); // number of hits
    d = searcher->doc(hits[0]->doc);
    EXPECT_EQ(L"22", d->get(L"id")); // first document
    testHits(hits, 33, searcher->getIndexReader());
    searcher->close();

    // optimize
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthUNLIMITED);
    writer->optimize();
    writer->close();

    searcher = newLucene<IndexSearcher>(dir, true);
    hits = searcher->search(newLucene<TermQuery>(newLucene<Term>(L"content", L"aaa")), FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(33, hits.size()); // number of hits
    d = searcher->doc(hits[0]->doc);
    EXPECT_EQ(L"22", d->get(L"id")); // first document
    testHits(hits, 33, searcher->getIndexReader());
    searcher->close();

    dir->close();
}

// Open pre-lockless index, add docs, do a delete and setNorm, and search
static void changeIndexWithAdds(const String& dirName) {
    String origDirName(dirName);
    String dirPath = fullDir(dirName);

    DirectoryPtr dir = FSDirectory::open(dirPath);

    // open writer
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthUNLIMITED);

    // add 10 docs
    for (int32_t i = 0; i < 10; ++i) {
        addDoc(writer, 35 + i);
    }

    // make sure writer sees right total - writer seems not to know about deletes in .del?
    int32_t dirNumber = StringUtils::toInt(dirName.substr(0, 2));
    int32_t expected = dirNumber < 24 ? 45 : 46;

    EXPECT_EQ(expected, writer->maxDoc()); // doc count
    writer->close();

    // make sure searching sees right # hits
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, true);
    Collection<ScoreDocPtr> hits = searcher->search(newLucene<TermQuery>(newLucene<Term>(L"content", L"aaa")), FilterPtr(), 1000)->scoreDocs;
    DocumentPtr d = searcher->doc(hits[0]->doc);
    EXPECT_EQ(L"21", d->get(L"id")); // first document
    testHits(hits, 44, searcher->getIndexReader());
    searcher->close();

    // make sure we can do delete & setNorm against this pre-lockless segment
    IndexReaderPtr reader = IndexReader::open(dir, false);
    TermPtr searchTerm = newLucene<Term>(L"id", L"6");
    int32_t delCount = reader->deleteDocuments(searchTerm);
    EXPECT_EQ(1, delCount); // delete count
    reader->setNorm(22, L"content", 2.0);
    reader->close();

    // make sure they "took"
    searcher = newLucene<IndexSearcher>(dir, true);
    hits = searcher->search(newLucene<TermQuery>(newLucene<Term>(L"content", L"aaa")), FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(43, hits.size()); // number of hits
    d = searcher->doc(hits[0]->doc);
    EXPECT_EQ(L"22", d->get(L"id")); // first document
    testHits(hits, 43, searcher->getIndexReader());
    searcher->close();

    // optimize
    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthUNLIMITED);
    writer->optimize();
    writer->close();

    searcher = newLucene<IndexSearcher>(dir, true);
    hits = searcher->search(newLucene<TermQuery>(newLucene<Term>(L"content", L"aaa")), FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(43, hits.size()); // number of hits
    d = searcher->doc(hits[0]->doc);
    testHits(hits, 43, searcher->getIndexReader());
    EXPECT_EQ(L"22", d->get(L"id")); // first document
    searcher->close();

    dir->close();
}

TEST_F(BackwardsCompatibilityTest, testCreateCFS) {
    String dirName(L"testindex.cfs");
    createIndex(dirName, true);
    rmDir(dirName);
}

TEST_F(BackwardsCompatibilityTest, testCreateNoCFS) {
    String dirName(L"testindex.nocfs");
    createIndex(dirName, true);
    rmDir(dirName);
}

TEST_F(BackwardsCompatibilityTest, testOptimizeOldIndex) {
    int32_t hasTested29 = 0;

    for (int32_t i = 0; i < oldNamesLength; ++i) {
        copyIndex(oldNames[i]);
        String dirName(fullDir(oldNames[i]));
        DirectoryPtr dir = FSDirectory::open(dirName);

        if (boost::starts_with(oldNames[i], L"29.")) {
            checkCompressedFields29(dir, true);
            ++hasTested29;
        }

        IndexWriterPtr w = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
        w->optimize();
        w->close();

        checkIndex(dir);

        if (boost::starts_with(oldNames[i], L"29.")) {
            checkCompressedFields29(dir, false);
            ++hasTested29;
        }

        dir->close();
        rmDir(oldNames[i]);
    }

    EXPECT_EQ(4, hasTested29); // test for compressed field should have run 4 times
}

TEST_F(BackwardsCompatibilityTest, testSearchOldIndex) {
    for (int32_t i = 0; i < oldNamesLength; ++i) {
        copyIndex(oldNames[i]);
        String dirName(fullDir(oldNames[i]));
        searchIndex(oldNames[i], oldNames[i]);
        rmDir(oldNames[i]);
    }
}

TEST_F(BackwardsCompatibilityTest, testIndexOldIndexNoAdds) {
    for (int32_t i = 0; i < oldNamesLength; ++i) {
        copyIndex(oldNames[i]);
        String dirName(fullDir(oldNames[i]));
        changeIndexNoAdds(oldNames[i]);
        rmDir(oldNames[i]);
    }
}

TEST_F(BackwardsCompatibilityTest, testIndexOldIndex) {
    for (int32_t i = 0; i < oldNamesLength; ++i) {
        copyIndex(oldNames[i]);
        String dirName(fullDir(oldNames[i]));
        changeIndexWithAdds(oldNames[i]);
        rmDir(oldNames[i]);
    }
}

// Verifies that the expected file names were produced
TEST_F(BackwardsCompatibilityTest, testExactFileNames) {
    String outputDir = L"lucene.backwardscompat0.index";
    rmDir(outputDir);

    LuceneException finally;
    try {
        DirectoryPtr dir = FSDirectory::open(fullDir(outputDir));

        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
        writer->setRAMBufferSizeMB(16.0);

        for (int32_t i = 0; i < 35; ++i) {
            addDoc(writer, i);
        }

        EXPECT_EQ(35, writer->maxDoc()); // doc count
        writer->close();

        // Delete one doc so we get a .del file
        IndexReaderPtr reader = IndexReader::open(dir, false);
        TermPtr searchTerm = newLucene<Term>(L"id", L"7");
        int32_t delCount = reader->deleteDocuments(searchTerm);
        EXPECT_EQ(1, delCount); // delete the right number of documents

        // Set one norm so we get a .s0 file
        reader->setNorm(21, L"content", 1.5);
        reader->close();

        CompoundFileReaderPtr cfsReader = newLucene<CompoundFileReader>(dir, L"_0.cfs");
        FieldInfosPtr fieldInfos = newLucene<FieldInfos>(cfsReader, L"_0.fnm");
        int32_t contentFieldIndex = -1;

        for (int32_t i = 0; i < fieldInfos->size(); ++i) {
            FieldInfoPtr fi = fieldInfos->fieldInfo(i);
            if (fi->name == L"content") {
                contentFieldIndex = i;
                break;
            }
        }

        cfsReader->close();
        EXPECT_NE(contentFieldIndex, -1); // locate the 'content' field number in the _2.cfs segment

        // Now verify file names
        HashSet<String> expected = HashSet<String>::newInstance();
        expected.add(L"_0.cfs");
        expected.add(L"_0_1.del");
        expected.add(L"_0_1.s" + StringUtils::toString(contentFieldIndex));
        expected.add(L"segments_3");
        expected.add(L"segments.gen");

        HashSet<String> actual = dir->listAll();

        EXPECT_EQ(expected, actual);

        dir->close();
    } catch (LuceneException& e) {
        finally = e;
    }
    rmDir(outputDir);
    finally.throwException();
}
