/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#define NOMINMAX

#include "targetver.h"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include "LuceneHeaders.h"
#include "FilterIndexReader.h"
#include "MiscUtils.h"

using namespace Lucene;

/// Use the norms from one field for all fields.  Norms are read into memory, using a byte of memory
/// per document per searched field.  This can cause search of large collections with a large number
/// of fields to run out of memory.  If all of the fields contain only a single token, then the norms
/// are all identical, then single norm vector may be shared.
class OneNormsReader : public FilterIndexReader {
public:
    OneNormsReader(const IndexReaderPtr& in, const String& field) : FilterIndexReader(in) {
        this->field = field;
    }

    virtual ~OneNormsReader() {
    }

protected:
    String field;

public:
    virtual ByteArray norms(const String& field) {
        return in->norms(this->field);
    }
};

/// This demonstrates a typical paging search scenario, where the search engine presents pages of size n
/// to the user. The user can then go to the next page if interested in the next hits.
///
/// When the query is executed for the first time, then only enough results are collected to fill 5 result
/// pages. If the user wants to page beyond this limit, then the query is executed another time and all
/// hits are collected.
static void doPagingSearch(const SearcherPtr& searcher, const QueryPtr& query, int32_t hitsPerPage, bool raw, bool interactive) {
    // Collect enough docs to show 5 pages
    TopScoreDocCollectorPtr collector = TopScoreDocCollector::create(5 * hitsPerPage, false);
    searcher->search(query, collector);
    Collection<ScoreDocPtr> hits = collector->topDocs()->scoreDocs;

    int32_t numTotalHits = collector->getTotalHits();
    std::wcout << numTotalHits << L" total matching documents\n";

    int32_t start = 0;
    int32_t end = std::min(numTotalHits, hitsPerPage);

    while (true) {
        if (end > hits.size()) {
            std::wcout << L"Only results 1 - " << hits.size() << L" of " << numTotalHits << L" total matching documents collected.\n";
            std::wcout << L"Collect more (y/n) ?";
            String line;
            std::wcin >> line;
            boost::trim(line);

            if (line.empty() || boost::starts_with(line, L"n")) {
                break;
            }

            collector = TopScoreDocCollector::create(numTotalHits, false);
            searcher->search(query, collector);
            hits = collector->topDocs()->scoreDocs;
        }

        end = std::min(hits.size(), start + hitsPerPage);

        for (int32_t i = start; i < end; ++i) {
            if (raw) { // output raw format
                std::wcout << L"doc=" << hits[i]->doc << L" score=" << hits[i]->score << L"\n";
                continue;
            }

            DocumentPtr doc = searcher->doc(hits[i]->doc);
            String path = doc->get(L"path");
            if (!path.empty()) {
                std::wcout << StringUtils::toString(i + 1) + L". " << path << L"\n";
                String title = doc->get(L"title");
                if (!title.empty()) {
                    std::wcout << L"   Title: " << doc->get(L"title") << L"\n";
                }
            } else {
                std::wcout << StringUtils::toString(i + 1) + L". No path for this document\n";
            }
        }

        if (!interactive) {
            break;
        }

        if (numTotalHits >= end) {
            bool quit = false;
            while (true) {
                std::wcout << L"Press ";
                if (start - hitsPerPage >= 0) {
                    std::wcout << L"(p)revious page, ";
                }
                if (start + hitsPerPage < numTotalHits) {
                    std::wcout << L"(n)ext page, ";
                }
                std::wcout << L"(q)uit or enter number to jump to a page: ";

                String line;
                std::wcin >> line;
                boost::trim(line);

                if (line.empty() || boost::starts_with(line, L"q")) {
                    quit = true;
                    break;
                }
                if (boost::starts_with(line, L"p")) {
                    start = std::max((int32_t)0, start - hitsPerPage);
                    break;
                } else if (boost::starts_with(line, L"n")) {
                    if (start + hitsPerPage < numTotalHits) {
                        start += hitsPerPage;
                    }
                    break;
                } else {
                    int32_t page = 0;
                    try {
                        page = StringUtils::toInt(line);
                    } catch (NumberFormatException&) {
                    }
                    if ((page - 1) * hitsPerPage < numTotalHits) {
                        start = std::max((int32_t)0, (page - 1) * hitsPerPage);
                        break;
                    } else {
                        std::wcout << L"No such page\n";
                    }
                }
            }
            if (quit) {
                break;
            }
            end = std::min(numTotalHits, start + hitsPerPage);
        }
    }
}

class StreamingHitCollector : public Collector {
public:
    StreamingHitCollector() {
        docBase = 0;
    }

    virtual ~StreamingHitCollector() {
    }

protected:
    ScorerPtr scorer;
    int32_t docBase;

public:
    /// simply print docId and score of every matching document
    virtual void collect(int32_t doc) {
        std::wcout << L"doc=" << (doc + docBase) << L" score=" << scorer->score();
    }

    virtual bool acceptsDocsOutOfOrder() {
        return true;
    }

    virtual void setNextReader(const IndexReaderPtr& reader, int32_t docBase) {
        this->docBase = docBase;
    }

    virtual void setScorer(const ScorerPtr& scorer) {
        this->scorer = scorer;
    }
};

/// This method uses a custom HitCollector implementation which simply prints out the docId and score of
/// every matching document.
///
/// This simulates the streaming search use case, where all hits are supposed to be processed, regardless
/// of their relevance.
static void doStreamingSearch(const SearcherPtr& searcher, const QueryPtr& query) {
    searcher->search(query, newLucene<StreamingHitCollector>());
}

/// Simple command-line based search demo.
int main(int argc, char* argv[]) {
    if (argc == 1 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-help") == 0) {
        std::wcout << L"Usage: searchfiles.exe [-index dir] [-field f] [-repeat n] [-queries file] [-raw] ";
        std::wcout << L"[-norms field] [-paging hitsPerPage]\n\n";
        std::wcout << L"Specify 'false' for hitsPerPage to use streaming instead of paging search.\n";
        return 1;
    }

    try {
        String index = L"index";
        String field = L"contents";
        String queries;
        int32_t repeat = 0;
        bool raw = false;
        String normsField;
        bool paging = true;
        int32_t hitsPerPage = 10;

        for (int32_t i = 0; i < argc; ++i) {
            if (strcmp(argv[i], "-index") == 0) {
                index = StringUtils::toUnicode(argv[i + 1]);
                ++i;
            } else if (strcmp(argv[i], "-field") == 0) {
                field = StringUtils::toUnicode(argv[i + 1]);
                ++i;
            } else if (strcmp(argv[i], "-queries") == 0) {
                queries = StringUtils::toUnicode(argv[i + 1]);
                ++i;
            } else if (strcmp(argv[i], "-repeat") == 0) {
                repeat = StringUtils::toInt(StringUtils::toUnicode(argv[i + 1]));
                ++i;
            } else if (strcmp(argv[i], "-raw") == 0) {
                raw = true;
            } else if (strcmp(argv[i], "-norms") == 0) {
                normsField = StringUtils::toUnicode(argv[i + 1]);
                ++i;
            } else if (strcmp(argv[i], "-paging") == 0) {
                if (strcmp(argv[i + 1], "false") == 0) {
                    paging = false;
                } else {
                    hitsPerPage = StringUtils::toInt(StringUtils::toUnicode(argv[i + 1]));
                    if (hitsPerPage == 0) {
                        paging = false;
                    }
                }
                ++i;
            }
        }

        // only searching, so read-only=true
        IndexReaderPtr reader = IndexReader::open(FSDirectory::open(index), true);

        if (!normsField.empty()) {
            reader = newLucene<OneNormsReader>(reader, normsField);
        }

        SearcherPtr searcher = newLucene<IndexSearcher>(reader);
        AnalyzerPtr analyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
        QueryParserPtr parser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, field, analyzer);

        ReaderPtr in;
        if (!queries.empty()) {
            in = newLucene<FileReader>(queries);
        }

        while (true) {
            String line;

            if (!queries.empty()) {
                wchar_t c = in->read();
                while (c != L'\n' && c != L'\r' && c != Reader::READER_EOF) {
                    line += c;
                    c = in->read();
                }
            } else {
                std::wcout << L"Enter query: ";
				getline(std::wcin, line);
            }
            boost::trim(line);

            if (line.empty()) {
                break;
            }

            QueryPtr query = parser->parse(line);
            std::wcout << L"Searching for: " << query->toString(field) << L"\n";

            if (repeat > 0) { // repeat and time as benchmark
                int64_t start = MiscUtils::currentTimeMillis();
                for (int32_t i = 0; i < repeat; ++i) {
                    searcher->search(query, FilterPtr(), 100);
                }
                std::wcout << L"Time: " << (MiscUtils::currentTimeMillis() - start) << L"ms\n";
            }

            if (paging) {
                doPagingSearch(searcher, query, hitsPerPage, raw, queries.empty());
            } else {
                doStreamingSearch(searcher, query);
            }
        }
        reader->close();
    } catch (LuceneException& e) {
        std::wcout << L"Exception: " << e.getError() << L"\n";
        return 1;
    }

    return 0;
}

