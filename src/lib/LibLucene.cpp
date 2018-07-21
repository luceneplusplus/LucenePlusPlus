#include <iostream>
#include <fstream>
#include <sstream>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

#include "LuceneHeaders.h"
#include "NumericField.h"
#include "FileUtils.h"
#include "Highlighter.h"
#include "SimpleFragmenter.h"
#include "MiscUtils.h"
#include "TermPositionVector.h"
#include "QueryScorer.h"

#include "lucene.pb.h"

using namespace Lucene;

struct LuceneWriteContext {
  IndexWriterPtr writer;
  int64_t docNumber;
};

void handleException(const LuceneException& exception) {
  std::cout << "Lucene threw an exception:" << std::endl;
  std::cout << StringUtils::toUTF8(exception.getError()) << std::endl;
}

DocumentPtr fileDocument(const String& docFile) {
  DocumentPtr doc = newLucene<Document>();

  // Add the path of the file as a field named "path".  Use a field that is indexed (ie. searchable), but
  // don't tokenize the field into words.
  doc->add(newLucene<Field>(L"title", docFile, Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

  // Add the last modified date of the file a field named "modified".
  auto date = newLucene<NumericField>(L"date", Field::STORE_YES, true);
  date->setLongValue(FileUtils::fileModified(docFile));
  doc->add(date);

  // Add the contents of the file to a field named "contents".  Specify a Reader, so that the text of the file is
  // tokenized and indexed, but not stored.  Note that FileReader expects the file to be in the system's default
  // encoding.  If that's not the case searching for special characters will fail.
  std::ifstream input(StringUtils::toUTF8(docFile));
  std::stringstream buffer;
  buffer << input.rdbuf();
  doc->add(newLucene<Field>(L"contents", StringUtils::toUnicode(buffer.str()), Field::STORE_YES, Field::INDEX_ANALYZED));

  return doc;
}

void indexDocs(LuceneWriteContext* context, const String& sourceDir) {
  HashSet<String> dirList(HashSet<String>::newInstance());
  if (!FileUtils::listDirectory(sourceDir, false, dirList)) {
    return;
  }

  for (HashSet<String>::iterator dirFile = dirList.begin(); dirFile != dirList.end(); ++dirFile) {
    String docFile(FileUtils::joinPath(sourceDir, *dirFile));
    if (FileUtils::isDirectory(docFile)) {
      indexDocs(context, docFile);
    } else {
      std::wcout << L"Adding [" << ++context->docNumber << L"]: " << *dirFile << L"\n";

      try {
        context->writer->addDocument(fileDocument(docFile));
      } catch (FileNotFoundException&) {
      }
    }
  }
}

extern "C" LuceneWriteContext* LuceneCreateWriteContext(const char* indexDirectory) {
  LuceneWriteContext* context = new LuceneWriteContext();
  String indexDir(StringUtils::toUnicode(indexDirectory));
  context->writer = newLucene<IndexWriter>(FSDirectory::open(indexDir), newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
  context->docNumber = 0;
  return context;
}

extern "C" void LuceneIndexDocs(LuceneWriteContext* context, const char* sourceDirectory) {
  String sourceDir(StringUtils::toUnicode(sourceDirectory));
  indexDocs(context, sourceDir);
}

extern "C" void LuceneIndexDoc(LuceneWriteContext* context, uint8_t* data, size_t length) {
  google::protobuf::io::ArrayInputStream stream(data, length);
  google::protobuf::io::CodedInputStream input(&stream);
  DocumentMessage doc_msg;
  doc_msg.MergeFromCodedStream(&input);
  DocumentPtr doc = newLucene<Document>();
  doc->add(newLucene<Field>(L"title", StringUtils::toUnicode(doc_msg.title()), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
  doc->add(newLucene<Field>(L"contents", StringUtils::toUnicode(doc_msg.contents()), Field::STORE_YES, Field::INDEX_ANALYZED));
  auto date = newLucene<NumericField>(L"date", Field::STORE_YES, true);
  date->setLongValue(doc_msg.date());
  doc->add(date);
  doc->add(newLucene<Field>(L"author", StringUtils::toUnicode(doc_msg.author()), Field::STORE_YES, Field::INDEX_ANALYZED));
  context->writer->addDocument(doc);
}

extern "C" void LuceneOptimizeIndex(LuceneWriteContext* context) {
  context->writer->optimize();
  context->writer->close();
}

extern "C" void LuceneDeleteWriteContext(LuceneWriteContext* context) {
  context->writer->close();
  delete context;
}

struct LuceneReadContext {
  IndexReaderPtr reader;
  std::unique_ptr<QueryResult> result;
};

extern "C" LuceneReadContext* LuceneCreateReadContext(const char* indexDirectory) {
  LuceneReadContext* context = new LuceneReadContext();
  String indexDir(StringUtils::toUnicode(indexDirectory));
  context->reader = IndexReader::open(FSDirectory::open(indexDir), true);
  return context;
}

extern "C" ssize_t LuceneQuery(LuceneReadContext* context, const char* field, const char* search_query) {
  try {
    String f = StringUtils::toUnicode(field);
    String q = StringUtils::toUnicode(search_query);
    SearcherPtr searcher = newLucene<IndexSearcher>(context->reader);
    AnalyzerPtr analyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    QueryParserPtr parser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, f, analyzer);
    QueryPtr query = parser->parse(q);
    std::wcout << L"Searching for: " << query->toString(f) << L"\n";

    TopScoreDocCollectorPtr collector = TopScoreDocCollector::create(20, false);
    searcher->search(query, collector);
    Collection<ScoreDocPtr> hits = collector->topDocs()->scoreDocs;
    int32_t numTotalHits = collector->getTotalHits();
    std::wcout << numTotalHits << L" total matching documents\n";

    context->result.reset(new QueryResult());

    for (int i = 0; i < hits.size(); ++i) {
      Collection<String> results = Collection<String>::newInstance();
      DocumentPtr doc = searcher->doc(hits[i]->doc);
      SearchResult* output = context->result->add_result();
      output->set_title(StringUtils::toUTF8(doc->get(L"title")));
      output->set_score(hits[i]->score);
      output->set_date(StringUtils::toLong(doc->get(L"date")));
      output->set_author(StringUtils::toUTF8(doc->get(L"author")));
      output->set_index(hits[i]->doc);

      String text = doc->get(L"contents");
      TokenStreamPtr tokenStream = analyzer->tokenStream(L"contents", newLucene<StringReader>(text));
      QueryScorerPtr scorer =  newLucene<QueryScorer>(query, L"contents");
      HighlighterPtr highlighter = newLucene<Highlighter>(scorer);
      highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(200));
      int32_t maxNumFragmentsRequired = 1;
      results.add(highlighter->getBestFragments(tokenStream, text, maxNumFragmentsRequired, L"..."));

      for (auto& result : results) {
        output->add_context(StringUtils::toUTF8(result));
      }
    }
  } catch (const LuceneException& exception) {
    handleException(exception);
    return -1;
  }
  return context->result->ByteSize();
}

extern "C" size_t LuceneLookup(LuceneReadContext* context, int32_t index) {
  auto doc = context->reader->document(index);
  context->result.reset(new QueryResult());
  SearchResult* output = context->result->add_result();
  output->set_title(StringUtils::toUTF8(doc->get(L"title")));
  output->set_date(StringUtils::toLong(doc->get(L"date")));
  output->set_author(StringUtils::toUTF8(doc->get(L"author")));
  output->set_index(index);
  output->add_context(StringUtils::toUTF8(doc->get(L"contents")));
  return context->result->ByteSize();
}

extern "C" void LuceneWriteQueryResult(LuceneReadContext* context, uint8_t* data, size_t size) {
  google::protobuf::io::ArrayOutputStream stream(data, size);
  google::protobuf::io::CodedOutputStream output(&stream);
  context->result->SerializeToCodedStream(&output);
}

extern "C" void LuceneDeleteReadContext(LuceneReadContext* context) {
  context->reader->close();
  delete context;
}
