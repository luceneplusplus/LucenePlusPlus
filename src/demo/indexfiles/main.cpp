/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#define NOMINMAX

#include "targetver.h"
#include <iostream>
#include "LuceneHeaders.h"
#include "FileUtils.h"
#include "MiscUtils.h"

using namespace Lucene;

int32_t docNumber = 0;

DocumentPtr fileDocument(const String& docFile) {
    DocumentPtr doc = newLucene<Document>();

    // Add the path of the file as a field named "path".  Use a field that is indexed (ie. searchable), but
    // don't tokenize the field into words.
    doc->add(newLucene<Field>(L"path", docFile, Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    // Add the last modified date of the file a field named "modified".  Use a field that is indexed (ie. searchable),
    // but don't tokenize the field into words.
    doc->add(newLucene<Field>(L"modified", DateTools::timeToString(FileUtils::fileModified(docFile), DateTools::RESOLUTION_MINUTE),
                              Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    // Add the contents of the file to a field named "contents".  Specify a Reader, so that the text of the file is
    // tokenized and indexed, but not stored.  Note that FileReader expects the file to be in the system's default
    // encoding.  If that's not the case searching for special characters will fail.
    doc->add(newLucene<Field>(L"contents", newLucene<FileReader>(docFile)));

    return doc;
}

void indexDocs(const IndexWriterPtr& writer, const String& sourceDir) {
    HashSet<String> dirList(HashSet<String>::newInstance());
    if (!FileUtils::listDirectory(sourceDir, false, dirList)) {
        return;
    }

    for (HashSet<String>::iterator dirFile = dirList.begin(); dirFile != dirList.end(); ++dirFile) {
        String docFile(FileUtils::joinPath(sourceDir, *dirFile));
        if (FileUtils::isDirectory(docFile)) {
            indexDocs(writer, docFile);
        } else {
            std::wcout << L"Adding [" << ++docNumber << L"]: " << *dirFile << L"\n";

            try {
                writer->addDocument(fileDocument(docFile));
            } catch (FileNotFoundException&) {
            }
        }
    }
}

/// Index all text files under a directory.
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::wcout << L"Usage: indexfiles.exe <index source dir> <lucene index dir>\n";
        return 1;
    }

    String sourceDir(StringUtils::toUnicode(argv[1]));
    String indexDir(StringUtils::toUnicode(argv[2]));

    if (!FileUtils::isDirectory(sourceDir)) {
        std::wcout << L"Source directory doesn't exist: " << sourceDir << L"\n";
        return 1;
    }

    if (!FileUtils::isDirectory(indexDir)) {
        if (!FileUtils::createDirectory(indexDir)) {
            std::wcout << L"Unable to create directory: " << indexDir << L"\n";
            return 1;
        }
    }

    uint64_t beginIndex = MiscUtils::currentTimeMillis();

    try {
        IndexWriterPtr writer = newLucene<IndexWriter>(FSDirectory::open(indexDir), newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
        std::wcout << L"Indexing to directory: " << indexDir << L"...\n";

        indexDocs(writer, sourceDir);

        uint64_t endIndex = MiscUtils::currentTimeMillis();
        uint64_t indexDuration = endIndex - beginIndex;
        std::wcout << L"Index time: " << indexDuration << L" milliseconds\n";
        std::wcout << L"Optimizing...\n";

        writer->optimize();

        uint64_t optimizeDuration = MiscUtils::currentTimeMillis() - endIndex;
        std::wcout << L"Optimize time: " << optimizeDuration << L" milliseconds\n";

        writer->close();

        std::wcout << L"Total time: " << indexDuration + optimizeDuration << L" milliseconds\n";
    } catch (LuceneException& e) {
        std::wcout << L"Exception: " << e.getError() << L"\n";
        return 1;
    }

    return 0;
}
