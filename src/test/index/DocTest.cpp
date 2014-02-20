/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "FSDirectory.h"
#include "IndexWriter.h"
#include "SimpleAnalyzer.h"
#include "SegmentInfo.h"
#include "FileReader.h"
#include "Document.h"
#include "Field.h"
#include "SegmentReader.h"
#include "SegmentMerger.h"
#include "TermEnum.h"
#include "TermPositions.h"
#include "Term.h"
#include "FileUtils.h"

using namespace Lucene;

typedef LuceneTestFixture DocTest;

static SegmentInfoPtr indexDoc(const IndexWriterPtr& writer, const String& fileName) {
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"contents", newLucene<FileReader>(FileUtils::joinPath(getTestDir(), fileName))));
    writer->addDocument(doc);
    writer->commit();
    return writer->newestSegment();
}

static void printSegment(StringStream& out, const SegmentInfoPtr& si) {
    SegmentReaderPtr reader = SegmentReader::get(true, si, IndexReader::DEFAULT_TERMS_INDEX_DIVISOR);

    for (int32_t i = 0; i < reader->numDocs(); ++i) {
        out << reader->document(i)->toString() << L"\n";
    }

    TermEnumPtr tis = reader->terms();
    while (tis->next()) {
        out << tis->term()->toString();
        out << L" DF=" << tis->docFreq() << L"\n";

        TermPositionsPtr positions = reader->termPositions(tis->term());
        LuceneException finally;
        try {
            while (positions->next()) {
                out << L" doc=" << positions->doc();
                out << L" TF=" << positions->freq();
                out << L" pos=";
                out << positions->nextPosition() << L"\n";
                for (int32_t j = 1; j < positions->freq(); ++j) {
                    out << L"," << positions->nextPosition();
                }
            }
        } catch (LuceneException& e) {
            finally = e;
        }
        positions->close();
        finally.throwException();
    }
    tis->close();
    reader->close();
}

static SegmentInfoPtr merge(const SegmentInfoPtr& si1, const SegmentInfoPtr& si2, const String& merged, bool useCompoundFile) {
    SegmentReaderPtr r1 = SegmentReader::get(true, si1, IndexReader::DEFAULT_TERMS_INDEX_DIVISOR);
    SegmentReaderPtr r2 = SegmentReader::get(true, si2, IndexReader::DEFAULT_TERMS_INDEX_DIVISOR);

    SegmentMergerPtr merger = newLucene<SegmentMerger>(si1->dir, merged);

    merger->add(r1);
    merger->add(r2);
    merger->merge();
    merger->closeReaders();

    if (useCompoundFile) {
        HashSet<String> filesToDelete = merger->createCompoundFile(merged + L".cfs");
        for (HashSet<String>::iterator file = filesToDelete.begin(); file != filesToDelete.end(); ++file) {
            si1->dir->deleteFile(*file);
        }
    }

    return newLucene<SegmentInfo>(merged, si1->docCount + si2->docCount, si1->dir, useCompoundFile, true);
}

TEST_F(DocTest, testIndexAndMerge) {
    String indexDir(FileUtils::joinPath(getTempDir(), L"testDoc"));

    DirectoryPtr directory = FSDirectory::open(indexDir);
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

    SegmentInfoPtr si1 = indexDoc(writer, L"testdoc1.txt");
    StringStream out;
    printSegment(out, si1);

    SegmentInfoPtr si2 = indexDoc(writer, L"testdoc2.txt");
    printSegment(out, si2);
    writer->close();

    SegmentInfoPtr siMerge = merge(si1, si2, L"merge", false);
    printSegment(out, siMerge);

    SegmentInfoPtr siMerge2 = merge(si1, si2, L"merge2", false);
    printSegment(out, siMerge2);

    SegmentInfoPtr siMerge3 = merge(siMerge, siMerge2, L"merge3", false);
    printSegment(out, siMerge3);

    directory->close();
    String multiFileOutput = out.str();

    out.str(L"");

    directory = FSDirectory::open(indexDir);
    writer = newLucene<IndexWriter>(directory, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

    si1 = indexDoc(writer, L"testdoc1.txt");
    printSegment(out, si1);

    si2 = indexDoc(writer, L"testdoc2.txt");
    printSegment(out, si2);
    writer->close();

    siMerge = merge(si1, si2, L"merge", true);
    printSegment(out, siMerge);

    siMerge2 = merge(si1, si2, L"merge2", true);
    printSegment(out, siMerge2);

    siMerge3 = merge(siMerge, siMerge2, L"merge3", true);
    printSegment(out, siMerge3);

    directory->close();

    String singleFileOutput = out.str();

    EXPECT_EQ(multiFileOutput, singleFileOutput);
}
