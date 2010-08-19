/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuceneTestFixture.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "SimpleAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexReader.h"
#include "Collector.h"
#include "Scorer.h"
#include "IndexSearcher.h"
#include "TermQuery.h"
#include "Term.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(SetNormTest, LuceneTestFixture)

namespace TestSetNorm
{
    class SetNormCollector : public Collector
    {
    public:
        SetNormCollector(Collection<double> scores)
        {
            this->scores = scores;
            this->base = 0;
        }
        
        virtual ~SetNormCollector()
        {
        }
    
    protected:
        int32_t base;
        ScorerPtr scorer;
        Collection<double> scores;
    
    public:
        virtual void setScorer(ScorerPtr scorer)
        {
            this->scorer = scorer;
        }
        
        virtual void collect(int32_t doc)
        {
            scores[doc + base] = scorer->score();
        }
        
        virtual void setNextReader(IndexReaderPtr reader, int32_t docBase)
        {
            base = docBase;
        }
        
        virtual bool acceptsDocsOutOfOrder()
        {
            return true;
        }
    };
}

BOOST_AUTO_TEST_CASE(testSetNorm)
{
    RAMDirectoryPtr store = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(store, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

    // add the same document four times
    FieldablePtr f1 = newLucene<Field>(L"field", L"word", Field::STORE_YES, Field::INDEX_ANALYZED);
    DocumentPtr d1 = newLucene<Document>();
    d1->add(f1);
    writer->addDocument(d1);
    writer->addDocument(d1);
    writer->addDocument(d1);
    writer->addDocument(d1);
    writer->close();

    // reset the boost of each instance of this document
    IndexReaderPtr reader = IndexReader::open(store, false);
    reader->setNorm(0, L"field", 1.0);
    reader->setNorm(1, L"field", 2.0);
    reader->setNorm(2, L"field", 4.0);
    reader->setNorm(3, L"field", 16.0);
    reader->close();

    // check that searches are ordered by this boost
    Collection<double> scores = Collection<double>::newInstance(4);
    
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(store, true);
    searcher->search(newLucene<TermQuery>(newLucene<Term>(L"field", L"word")), newLucene<TestSetNorm::SetNormCollector>(scores));
    
    double lastScore = 0.0;
    for (int32_t i = 0; i < 4; ++i)
    {
        BOOST_CHECK(scores[i] > lastScore);
        lastScore = scores[i];
    }
}

BOOST_AUTO_TEST_SUITE_END()
