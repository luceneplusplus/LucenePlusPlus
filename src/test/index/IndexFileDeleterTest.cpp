/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "IndexFileDeleter.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexReader.h"
#include "Term.h"
#include "CompoundFileReader.h"
#include "FieldInfos.h"
#include "FieldInfo.h"
#include "IndexInput.h"
#include "IndexOutput.h"

using namespace Lucene;

typedef LuceneTestFixture IndexFileDeleterTest;

static void addDoc(const IndexWriterPtr& writer, int32_t id) {
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", L"aaa", Field::STORE_YES, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"id", StringUtils::toString(id), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    writer->addDocument(doc);
}

static void copyFile(const DirectoryPtr& dir, const String& src, const String& dest) {
    IndexInputPtr in = dir->openInput(src);
    IndexOutputPtr out = dir->createOutput(dest);
    ByteArray b = ByteArray::newInstance(1024);
    int64_t remainder = in->length();
    while (remainder > 0) {
        int32_t len = std::min(b.size(), (int32_t)remainder);
        in->readBytes(b.get(), 0, len);
        out->writeBytes(b.get(), len);
        remainder -= len;
    }
    in->close();
    out->close();
}

static HashSet<String> difFiles(Collection<String> files1, Collection<String> files2) {
    HashSet<String> set1 = HashSet<String>::newInstance();
    HashSet<String> set2 = HashSet<String>::newInstance();
    HashSet<String> extra = HashSet<String>::newInstance();
    for (Collection<String>::iterator file = files1.begin(); file != files1.end(); ++file) {
        set1.add(*file);
    }
    for (Collection<String>::iterator file = files2.begin(); file != files2.end(); ++file) {
        set2.add(*file);
    }
    for (HashSet<String>::iterator file = set1.begin(); file != set1.end(); ++file) {
        if (!set2.contains(*file)) {
            extra.add(*file);
        }
    }
    for (HashSet<String>::iterator file = set2.begin(); file != set2.end(); ++file) {
        if (!set1.contains(*file)) {
            extra.add(*file);
        }
    }
    return extra;
}

TEST_F(IndexFileDeleterTest, testDeleteLeftoverFiles) {
    DirectoryPtr dir = newLucene<RAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(10);
    int32_t i = 0;
    for (; i < 35; ++i) {
        addDoc(writer, i);
    }
    writer->setUseCompoundFile(false);
    for (; i < 45; ++i) {
        addDoc(writer, i);
    }
    writer->close();

    // Delete one doc so we get a .del file
    IndexReaderPtr reader = IndexReader::open(dir, false);
    TermPtr searchTerm = newLucene<Term>(L"id", L"7");
    int32_t delCount = reader->deleteDocuments(searchTerm);
    EXPECT_EQ(1, delCount);

    // Set one norm so we get a .s0 file
    reader->setNorm(21, L"content", 1.5);
    reader->close();

    // Now, artificially create an extra .del file and extra .s0 file
    HashSet<String> _files = dir->listAll();

    // Here we have to figure out which field number corresponds to "content", and then
    // set our expected file names below accordingly.
    CompoundFileReaderPtr cfsReader = newLucene<CompoundFileReader>(dir, L"_2.cfs");
    FieldInfosPtr fieldInfos = newLucene<FieldInfos>(cfsReader, L"_2.fnm");
    int32_t contentFieldIndex = -1;
    for (int32_t j = 0; j < fieldInfos->size(); ++j) {
        FieldInfoPtr fi = fieldInfos->fieldInfo(j);
        if (fi->name == L"content") {
            contentFieldIndex = j;
            break;
        }
    }

    cfsReader->close();
    EXPECT_NE(contentFieldIndex, -1);

    String normSuffix = L"s" + StringUtils::toString(contentFieldIndex);

    // Create a bogus separate norms file for a segment/field that actually has a
    // separate norms file already
    copyFile(dir, L"_2_1." + normSuffix, L"_2_2." + normSuffix);

    // Create a bogus separate norms file for a segment/field that actually has a
    // separate norms file already, using the "not compound file" extension
    copyFile(dir, L"_2_1." + normSuffix, L"_2_2.f" + StringUtils::toString(contentFieldIndex));

    // Create a bogus separate norms file for a segment/field that does not have a
    // separate norms file already
    copyFile(dir, L"_2_1." + normSuffix, L"_1_1." + normSuffix);

    // Create a bogus separate norms file for a segment/field that does not have a
    // separate norms file already using the "not compound file" extension
    copyFile(dir, L"_2_1." + normSuffix, L"_1_1.f" + StringUtils::toString(contentFieldIndex));

    // Create a bogus separate del file for a segment that already has a separate
    // del file
    copyFile(dir, L"_0_1.del", L"_0_2.del");

    // Create a bogus separate del file for a segment that does not yet have a
    // separate del file
    copyFile(dir, L"_0_1.del", L"_1_1.del");

    // Create a bogus separate del file for a non-existent segment
    copyFile(dir, L"_0_1.del", L"_188_1.del");

    // Create a bogus segment file
    copyFile(dir, L"_0.cfs", L"_188.cfs");

    // Create a bogus fnm file when the CFS already exists
    copyFile(dir, L"_0.cfs", L"_0.fnm");

    // Create a deletable file
    copyFile(dir, L"_0.cfs", L"deletable");

    // Create some old segments file
    copyFile(dir, L"segments_3", L"segments");
    copyFile(dir, L"segments_3", L"segments_2");

    // Create a bogus cfs file shadowing a non-cfs segment
    copyFile(dir, L"_2.cfs", L"_3.cfs");

    HashSet<String> filesPre = dir->listAll();

    // Open and close a writer: it should delete the above 4 files and nothing more
    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthLIMITED);
    writer->close();

    HashSet<String> _files2 = dir->listAll();
    dir->close();

    Collection<String> files = Collection<String>::newInstance(_files.begin(), _files.end());
    Collection<String> files2 = Collection<String>::newInstance(_files2.begin(), _files2.end());

    std::sort(files.begin(), files.end());
    std::sort(files2.begin(), files2.end());

    HashSet<String> dif = difFiles(files, files2);

    EXPECT_TRUE(dif.empty());
}
