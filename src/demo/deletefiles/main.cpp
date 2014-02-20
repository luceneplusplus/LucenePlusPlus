/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#define NOMINMAX

#include "targetver.h"
#include <iostream>
#include "LuceneHeaders.h"

using namespace Lucene;

/// Deletes documents from an index that do not contain a term.
int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::wcout << L"Usage: deletefiles.exe <lucene index dir> <unique_term>\n";
        return 1;
    }

    try {
        DirectoryPtr directory = FSDirectory::open(StringUtils::toUnicode(argv[1]));

        // we don't want read-only because we are about to delete
        IndexReaderPtr reader = IndexReader::open(directory, false);

        TermPtr term = newLucene<Term>(L"path", StringUtils::toUnicode(argv[2]));
        int32_t deleted = reader->deleteDocuments(term);

        std::wcout << L"Deleted " << deleted << L" documents containing " << term->toString() << L"\n";

        reader->close();
        directory->close();
    } catch (LuceneException& e) {
        std::wcout << L"Exception: " << e.getError() << L"\n";
        return 1;
    }

    return 0;
}

