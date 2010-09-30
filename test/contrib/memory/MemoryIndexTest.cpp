/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TestUtils.h"
#include "BaseTokenStreamFixture.h"
#include "BufferedReader.h"
#include "FileReader.h"
#include "StopAnalyzer.h"
#include "SimpleAnalyzer.h"
#include "StandardAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "MemoryIndex.h"
#include "IndexSearcher.h"
#include "TermDocs.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "QueryParser.h"

using namespace Lucene;

class MemoryIndexTestFixture : public BaseTokenStreamFixture
{
public:
    MemoryIndexTestFixture()
    {
        fileDir = FileUtils::joinPath(getTestDir(), L"memoryfiles");
        queriesDir = FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"memory"), L"testqueries.txt");
    }
    
    virtual ~MemoryIndexTestFixture()
    {
    }

protected:
    AnalyzerPtr analyzer;
    bool fastMode;
    bool verbose;
    String fileDir;
    String queriesDir;
    
    static const String FIELD_NAME;
    
public:
    /// returns all files for indexing
    Collection<String> listFiles()
    {
        Collection<String> files = Collection<String>::newInstance();
        HashSet<String> dirList = HashSet<String>::newInstance();
        
	    if (!FileUtils::listDirectory(fileDir, true, dirList))
		    return Collection<String>();
    	
	    for (HashSet<String>::iterator dirFile = dirList.begin(); dirFile != dirList.end(); ++dirFile)
	        files.add(FileUtils::joinPath(fileDir, *dirFile));
	   
	   return files;
    }
    
    /// returns file line by line, ignoring empty lines and comments
    Collection<String> readLines()
    {
        Collection<String> lines = Collection<String>::newInstance();
        
        BufferedReaderPtr reader = newLucene<BufferedReader>(newLucene<FileReader>(queriesDir));
        String line;
        while (reader->readLine(line))
        {
            boost::trim(line);
            if (!line.empty() && !boost::starts_with(line, L"#") && !boost::starts_with(line, L"//"))
                lines.add(line);
        }
        reader->close();
        
        return lines;
    }
    
    String readFile(const String& file)
    {
        FileReaderPtr reader = newLucene<FileReader>(file);
        int32_t numChars = (int32_t)reader->length();
        CharArray buffer = CharArray::newInstance(numChars);
        numChars = reader->read(buffer.get(), 0, numChars);
        reader->close();
        return String(buffer.get(), numChars);
    }
    
    DocumentPtr createDocument(const String& content)
    {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(FIELD_NAME, content, Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS));
        return doc;
    }
    
    MemoryIndexPtr createMemoryIndex(DocumentPtr doc)
    {
        MemoryIndexPtr index = newLucene<MemoryIndex>();
        Collection<FieldablePtr> fields = doc->getFields();
        for (Collection<FieldablePtr>::iterator field = fields.begin(); field != fields.end(); ++field)
            index->addField((*field)->name(), (*field)->stringValue(), analyzer);
        return index;
    }
    
    RAMDirectoryPtr createRAMIndex(DocumentPtr doc)
    {
        RAMDirectoryPtr dir = newLucene<RAMDirectory>();
        IndexWriterPtr writer;
        LuceneException finally;
        try
        {
            writer = newLucene<IndexWriter>(dir, analyzer, true, IndexWriter::MaxFieldLengthUNLIMITED);
            writer->addDocument(doc);
            writer->optimize();
        }
        catch (IOException& e)
        {
            // can never happen
            boost::throw_exception(RuntimeException(e.getError()));
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        try
        {
            if (writer)
                writer->close();
        }
        catch (IOException& e)
        {
            boost::throw_exception(RuntimeException(e.getError()));
        }
        finally.throwException();
        return dir;
    }
    
    double query(LuceneObjectPtr index, QueryPtr query)
    {
        SearcherPtr searcher;
        double score = 0;
        LuceneException finally;
        try
        {
            if (MiscUtils::typeOf<Directory>(index))
                searcher = newLucene<IndexSearcher>(boost::dynamic_pointer_cast<Directory>(index), true);
            else
                searcher = boost::dynamic_pointer_cast<MemoryIndex>(index)->createSearcher();
            
            Collection<double> scores = Collection<double>::newInstance(1);
            scores[0] = 0.0;
            
            searcher->search(query, newLucene<MemoryIndexCollector>(scores));
            score = scores[0];
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        try
        {
            if (searcher)
                searcher->close();
        }
        catch (IOException& e)
        {
            boost::throw_exception(RuntimeException(e.getError()));
        }
        finally.throwException();
        return score;
    }
    
    QueryPtr parseQuery(const String& expression)
    {
        QueryParserPtr parser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, FIELD_NAME, analyzer);
        return parser->parse(expression);
    }
};

const String MemoryIndexTestFixture::FIELD_NAME = L"content";

BOOST_FIXTURE_TEST_SUITE(MemoryIndexTest, MemoryIndexTestFixture)

BOOST_AUTO_TEST_CASE(testMany)
{
    int32_t iters = 1;
    int32_t runs = 1;
    bool useMemIndex = true;
    bool useRAMIndex = true;
    
    Collection<String> files = listFiles();
    Collection<String> queries = readLines();
    HashSet<String> stopWords = StopAnalyzer::ENGLISH_STOP_WORDS_SET();
    
    Collection<AnalyzerPtr> analyzers = newCollection<AnalyzerPtr>(
        newLucene<SimpleAnalyzer>(), 
        newLucene<StopAnalyzer>(LuceneVersion::LUCENE_CURRENT), 
        newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT)
    );
    
    bool first = true;
    for (int32_t iter = 0; iter < iters; ++iter)
    {
        int64_t start = MiscUtils::currentTimeMillis();   
        int64_t bytes = 0;

        for (int32_t anal = 0; anal < analyzers.size(); ++anal)
        {
            this->analyzer = analyzers[anal];
            
            for (int32_t i = 0; i < files.size(); ++i)
            {
                String file = files[i];
                if (!FileUtils::fileExists(file) || FileUtils::isDirectory(file))
                    continue; // ignore
                bytes += FileUtils::fileLength(file);
                String text = readFile(file);
                DocumentPtr doc = createDocument(text);
                
                for (int32_t q = 0; q < queries.size(); ++q)
                {
                    QueryPtr query = parseQuery(queries[q]);

                    bool measureIndexing = false; // toggle this to measure query performance
                    MemoryIndexPtr memind;
                    if (useMemIndex && !measureIndexing)
                        memind = createMemoryIndex(doc);
                    
                    if (first)
                    {
                        IndexSearcherPtr s = memind->createSearcher();
                        TermDocsPtr td = s->getIndexReader()->termDocs(TermPtr());
                        BOOST_CHECK(td->next());
                        BOOST_CHECK_EQUAL(0, td->doc());
                        BOOST_CHECK_EQUAL(1, td->freq());
                        td->close();
                        s->close();
                        first = false;
                    }
                    
                    RAMDirectoryPtr ramind;
                    if (useRAMIndex && !measureIndexing)
                        ramind = createRAMIndex(doc);
                    
                    for (int32_t run = 0; run < runs; ++run)
                    {
                        double score1 = 0.0;
                        double score2 = 0.0;
                        if (useMemIndex && measureIndexing)
                            memind = createMemoryIndex(doc);
                        if (useMemIndex)
                            score1 = this->query(memind, query); 
                        if (useRAMIndex && measureIndexing)
                            ramind = createRAMIndex(doc);
                        if (useRAMIndex)
                            score2 = this->query(ramind, query);
                        if (useMemIndex && useRAMIndex)
                        {
                            BOOST_CHECK_EQUAL(score1, score2);
                            BOOST_CHECK(score1 >= 0.0 && score1 <= 1.0);
                            BOOST_CHECK(score2 >= 0.0 && score2 <= 1.0);
                        }
                    }
                }
            }
        }
        int64_t end = MiscUtils::currentTimeMillis();
        BOOST_TEST_MESSAGE("secs = " << ((double)(end - start) / 1000.0));
        BOOST_TEST_MESSAGE("queries/sec= " << (1.0 * (double)runs * (double)queries.size() * (double)analyzers.size() * (double)files.size() / ((double)(end - start) / 1000.0)));
        double mb = (1.0 * (double)bytes * (double)queries.size() * (double)runs) / (1024.0 * 1024.0);
        BOOST_TEST_MESSAGE("MB/sec = " << (mb / ((double)(end - start) / 1000.0)));
    }
}

BOOST_AUTO_TEST_SUITE_END()
