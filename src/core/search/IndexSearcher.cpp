/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "IndexSearcher.h"
#include "_IndexSearcher.h"
#include "IndexReader.h"
#include "TopScoreDocCollector.h"
#include "TopFieldDocs.h"
#include "TopFieldCollector.h"
#include "Weight.h"
#include "DocIdSet.h"
#include "Scorer.h"
#include "Filter.h"
#include "Query.h"
#include "ReaderUtil.h"
#include "Similarity.h"
#include "ThreadPool.h"
#include "HitQueue.h"
#include "ScoreDoc.h"
#include "SortField.h"
#include "FieldDoc.h"
#include "VariantUtils.h"

namespace Lucene
{
    IndexSearcher::IndexSearcher(DirectoryPtr path, bool readOnly)
    {
        ConstructSearcher(IndexReader::open(path, readOnly), true, NONE);
    }
    
    IndexSearcher::IndexSearcher(IndexReaderPtr reader, ExecutorService executor)
    {
        ConstructSearcher(reader, false, executor);
    }
    
    IndexSearcher::IndexSearcher(IndexReaderPtr reader, Collection<IndexReaderPtr> subReaders, Collection<int32_t> docStarts, ExecutorService executor)
    {
        this->fieldSortDoTrackScores = false;
        this->fieldSortDoMaxScore = false;
        this->reader = reader;
        this->subReaders = subReaders;
        this->docStarts = docStarts;
        this->closeReader = false;
        this->executor = executor;
        if (executor != NONE)
        {
            subSearchers = Collection<IndexSearcherPtr>::newInstance(subReaders.size());
            for (int32_t i = 0; i < subReaders.size(); ++i)
                subSearchers[i] = newLucene<IndexSearcher>(subReaders[i]);
        }
    }
    
    IndexSearcher::~IndexSearcher()
    {
    }
    
    void IndexSearcher::ConstructSearcher(IndexReaderPtr reader, bool closeReader, ExecutorService executor)
    {
        this->fieldSortDoTrackScores = false;
        this->fieldSortDoMaxScore = false;
        this->reader = reader;
        this->closeReader = closeReader;
        this->executor = executor;
        this->similarity = Similarity::getDefault();
        
        Collection<IndexReaderPtr> subReadersList(Collection<IndexReaderPtr>::newInstance());
        gatherSubReaders(subReadersList, reader);
        subReaders = subReadersList;
        docStarts = Collection<int32_t>::newInstance(subReaders.size());
        int32_t maxDoc = 0;
        for (int32_t i = 0; i < subReaders.size(); ++i)
        {
            docStarts[i] = maxDoc;
            maxDoc += subReaders[i]->maxDoc();
        }
        if (executor != NONE)
        {
            subSearchers = Collection<IndexSearcherPtr>::newInstance(subReaders.size());
            for (int32_t i = 0; i < subReaders.size(); ++i)
                subSearchers[i] = newLucene<IndexSearcher>(subReaders[i]);
        }
    }
    
    void IndexSearcher::gatherSubReaders(Collection<IndexReaderPtr> allSubReaders, IndexReaderPtr reader)
    {
        ReaderUtil::gatherSubReaders(allSubReaders, reader);
    }
    
    IndexReaderPtr IndexSearcher::getIndexReader()
    {
        return reader;
    }
    
    Collection<IndexReaderPtr> IndexSearcher::getSubReaders()
    {
        return subReaders;
    }
    
    int32_t IndexSearcher::maxDoc()
    {
        return reader->maxDoc();
    }
    
    int32_t IndexSearcher::docFreq(TermPtr term)
    {
        if (executor == NONE)
            return reader->docFreq(term);
        else
        {
            ThreadPoolPtr threadPool(ThreadPool::getInstance());
            Collection<FuturePtr> searchThreads(Collection<FuturePtr>::newInstance(subReaders.size()));
            for (int32_t i = 0; i < subReaders.size(); ++i)
                searchThreads[i] = threadPool->scheduleTask(boost::protect(boost::bind<int32_t>(boost::mem_fn(&Searchable::docFreq), subSearchers[i], term)));
            int32_t docFreq = 0;
            for (int32_t i = 0; i < searchThreads.size(); ++i)
                docFreq += searchThreads[i]->get<int32_t>();
            return docFreq;
        }
    }
    
    DocumentPtr IndexSearcher::doc(int32_t n)
    {
        return reader->document(n);
    }
    
    DocumentPtr IndexSearcher::doc(int32_t n, FieldSelectorPtr fieldSelector)
    {
        return reader->document(n, fieldSelector);
    }
    
    void IndexSearcher::setSimilarity(SimilarityPtr similarity)
    {
        this->similarity = similarity;
    }
    
    SimilarityPtr IndexSearcher::getSimilarity()
    {
        return similarity;
    }
    
    void IndexSearcher::close()
    {
        if (closeReader)
            reader->close();
    }
    
    TopDocsPtr IndexSearcher::search(QueryPtr query, int32_t n)
    {
        return search(query, FilterPtr(), n);
    }
    
    TopDocsPtr IndexSearcher::search(QueryPtr query, FilterPtr filter, int32_t n)
    {
        return search(createWeight(query), filter, n);
    }
    
    void IndexSearcher::search(QueryPtr query, FilterPtr filter, CollectorPtr results)
    {
        search(createWeight(query), filter, results);
    }
    
    void IndexSearcher::search(QueryPtr query, CollectorPtr results)
    {
        search(createWeight(query), FilterPtr(), results);
    }
    
    TopFieldDocsPtr IndexSearcher::search(QueryPtr query, FilterPtr filter, int32_t n, SortPtr sort)
    {
        return search(createWeight(query), filter, n, sort);
    }
    
    TopFieldDocsPtr IndexSearcher::search(QueryPtr query, int32_t n, SortPtr sort)
    {
        return search(createWeight(query), FilterPtr(), n, sort);
    }
    
    TopDocsPtr IndexSearcher::search(WeightPtr weight, FilterPtr filter, int32_t n)
    {
        if (executor == NONE)
        {
            // single thread
            int32_t limit = reader->maxDoc();
            if (limit == 0)
                limit = 1;
            n = std::min(n, limit);
            TopScoreDocCollectorPtr collector(TopScoreDocCollector::create(n, !weight->scoresDocsOutOfOrder()));
            search(weight, filter, collector);
            return collector->topDocs();
        }
        else
        {
            HitQueuePtr hq(newLucene<HitQueue>(n, false));
            SynchronizePtr lock(newInstance<Synchronize>());
            ThreadPoolPtr threadPool(ThreadPool::getInstance());
            Collection<FuturePtr> searchThreads(Collection<FuturePtr>::newInstance(subReaders.size()));
            Collection<MultiIndexSearcherCallableNoSortPtr> multiSearcher(Collection<MultiIndexSearcherCallableNoSortPtr>::newInstance(subReaders.size()));
            for (int32_t i = 0; i < subReaders.size(); ++i) // search each sub
            { 
                multiSearcher[i] = newLucene<MultiIndexSearcherCallableNoSort>(lock, subSearchers[i], weight, filter, n, hq, i, docStarts);
                searchThreads[i] = threadPool->scheduleTask(boost::protect(boost::bind<TopDocsPtr>(boost::mem_fn(&MultiIndexSearcherCallableNoSort::call), multiSearcher[i])));
            }

            int32_t totalHits = 0;
            double maxScore = -std::numeric_limits<double>::infinity();
            
            for (int32_t i = 0; i < searchThreads.size(); ++i)
            {
                TopDocsPtr topDocs(searchThreads[i]->get<TopDocsPtr>());
                if (topDocs->totalHits != 0)
                {
                    totalHits += topDocs->totalHits;
                    maxScore = std::max(maxScore, topDocs->getMaxScore());
                }
            }
            
            Collection<ScoreDocPtr> scoreDocs(Collection<ScoreDocPtr>::newInstance(hq->size()));
            for (int32_t i = hq->size() - 1; i >= 0; --i) // put docs in array
                scoreDocs[i] = hq->pop();
            
            return newLucene<TopDocs>(totalHits, scoreDocs, maxScore);
        }
    }
    
    TopFieldDocsPtr IndexSearcher::search(WeightPtr weight, FilterPtr filter, int32_t n, SortPtr sort)
    {
        return search(weight, filter, n, sort, true);
    }
    
    TopFieldDocsPtr IndexSearcher::search(WeightPtr weight, FilterPtr filter, int32_t n, SortPtr sort, bool fillFields)
    {
        if (!sort)
            boost::throw_exception(NullPointerException());

        if (executor == NONE)
        {
            // single thread
            int32_t limit = reader->maxDoc();
            if (limit == 0)
                limit = 1;
            n = std::min(n, limit);

            TopFieldCollectorPtr collector(TopFieldCollector::create(sort, n, fillFields, fieldSortDoTrackScores, fieldSortDoMaxScore, !weight->scoresDocsOutOfOrder()));
            search(weight, filter, collector);
            return boost::static_pointer_cast<TopFieldDocs>(collector->topDocs());
        }
        else
        {
            TopFieldCollectorPtr topCollector(TopFieldCollector::create(sort, n, fillFields, fieldSortDoTrackScores, fieldSortDoMaxScore, false));
            SynchronizePtr lock(newInstance<Synchronize>());
            ThreadPoolPtr threadPool(ThreadPool::getInstance());
            Collection<FuturePtr> searchThreads(Collection<FuturePtr>::newInstance(subReaders.size()));
            Collection<MultiIndexSearcherCallableWithSortPtr> multiSearcher(Collection<MultiIndexSearcherCallableWithSortPtr>::newInstance(subReaders.size()));
            for (int32_t i = 0; i < subReaders.size(); ++i) // search each sub
            {
                multiSearcher[i] = newLucene<MultiIndexSearcherCallableWithSort>(lock, subSearchers[i], weight, filter, n, topCollector, sort, i, docStarts);
                searchThreads[i] = threadPool->scheduleTask(boost::protect(boost::bind<TopFieldDocsPtr>(boost::mem_fn(&MultiIndexSearcherCallableWithSort::call), multiSearcher[i])));
            }
            
            int32_t totalHits = 0;
            double maxScore = -std::numeric_limits<double>::infinity();
            
            for (int32_t i = 0; i < searchThreads.size(); ++i)
            {
                TopFieldDocsPtr topFieldDocs(searchThreads[i]->get<TopFieldDocsPtr>());
                if (topFieldDocs->totalHits != 0)
                {
                    totalHits += topFieldDocs->totalHits;
                    maxScore = std::max(maxScore, topFieldDocs->getMaxScore());
                }
            }

            TopFieldDocsPtr topDocs(boost::static_pointer_cast<TopFieldDocs>(topCollector->topDocs()));
            return newLucene<TopFieldDocs>(totalHits, topDocs->scoreDocs, topDocs->fields, topDocs->getMaxScore());
        }
    }
    
    void IndexSearcher::search(WeightPtr weight, FilterPtr filter, CollectorPtr results)
    {
        // always use single thread
        if (!filter)
        {
            for (int32_t i = 0; i < subReaders.size(); ++i) // search each subreader
            {
                results->setNextReader(subReaders[i], docStarts[i]);
                ScorerPtr scorer(weight->scorer(subReaders[i], !results->acceptsDocsOutOfOrder(), true));
                if (scorer)
                    scorer->score(results);
            }
        }
        else
        {
            for (int32_t i = 0; i < subReaders.size(); ++i) // search each subreader
            {
                results->setNextReader(subReaders[i], docStarts[i]);
                searchWithFilter(subReaders[i], weight, filter, results);
            }
        }
    }
    
    void IndexSearcher::searchWithFilter(IndexReaderPtr reader, WeightPtr weight, FilterPtr filter, CollectorPtr collector)
    {
        BOOST_ASSERT(filter);
        
        ScorerPtr scorer(weight->scorer(reader, true, false));
        if (!scorer)
            return;
        
        int32_t docID = scorer->docID();
        BOOST_ASSERT(docID == -1 || docID == DocIdSetIterator::NO_MORE_DOCS);
        
        DocIdSetPtr filterDocIdSet(filter->getDocIdSet(reader));
        if (!filterDocIdSet)
        {
            // this means the filter does not accept any documents.
            return;
        }
        
        DocIdSetIteratorPtr filterIter(filterDocIdSet->iterator());
        if (!filterIter)
        {
            // this means the filter does not accept any documents.
            return;
        }
        
        int32_t filterDoc = filterIter->nextDoc();
        int32_t scorerDoc = scorer->advance(filterDoc);
        
        collector->setScorer(scorer);
        while (true)
        {
            if (scorerDoc == filterDoc)
            {
                // Check if scorer has exhausted, only before collecting.
                if (scorerDoc == DocIdSetIterator::NO_MORE_DOCS)
                    break;
                collector->collect(scorerDoc);
                filterDoc = filterIter->nextDoc();
                scorerDoc = scorer->advance(filterDoc);
            }
            else if (scorerDoc > filterDoc)
                filterDoc = filterIter->advance(scorerDoc);
            else
                scorerDoc = scorer->advance(filterDoc);
        }
    }
    
    QueryPtr IndexSearcher::rewrite(QueryPtr original)
    {
        QueryPtr query(original);
        for (QueryPtr rewrittenQuery(query->rewrite(reader)); rewrittenQuery != query; rewrittenQuery = query->rewrite(reader))
            query = rewrittenQuery;
        return query;
    }
    
    ExplanationPtr IndexSearcher::explain(QueryPtr query, int32_t doc)
    {
        return explain(createWeight(query), doc);
    }
    
    ExplanationPtr IndexSearcher::explain(WeightPtr weight, int32_t doc)
    {
        int32_t n = ReaderUtil::subIndex(doc, docStarts);
        int32_t deBasedDoc = doc - docStarts[n];
        return weight->explain(subReaders[n], deBasedDoc);
    }
    
    void IndexSearcher::setDefaultFieldSortScoring(bool doTrackScores, bool doMaxScore)
    {
        fieldSortDoTrackScores = doTrackScores;
        fieldSortDoMaxScore = doMaxScore;
        if (subSearchers) // propagate settings to subs
        { 
            for (Collection<IndexSearcherPtr>::iterator sub = subSearchers.begin(); sub != subSearchers.end(); ++sub)
                (*sub)->setDefaultFieldSortScoring(doTrackScores, doMaxScore);
        }
    }
    
    WeightPtr IndexSearcher::createWeight(QueryPtr query)
    {
        return query->weight(shared_from_this());
    }
    
    MultiIndexSearcherCallableNoSort::MultiIndexSearcherCallableNoSort(SynchronizePtr lock, IndexSearcherPtr searchable, WeightPtr weight, 
                                                                       FilterPtr filter, int32_t nDocs, HitQueuePtr hq, int32_t i, 
                                                                       Collection<int32_t> starts)
    {
        this->lock = lock;
        this->searchable = searchable;
        this->weight = weight;
        this->filter = filter;
        this->nDocs = nDocs;
        this->hq = hq;
        this->i = i;
        this->starts = starts;
    }
    
    MultiIndexSearcherCallableNoSort::~MultiIndexSearcherCallableNoSort()
    {
    }
    
    TopDocsPtr MultiIndexSearcherCallableNoSort::call()
    {
        TopDocsPtr docs(searchable->search(weight, filter, nDocs));
        Collection<ScoreDocPtr> scoreDocs(docs->scoreDocs);
        for (int32_t j = 0; j < scoreDocs.size(); ++j) // merge scoreDocs into hq
        {
            ScoreDocPtr scoreDoc(scoreDocs[j]);
            scoreDoc->doc += starts[i]; // convert doc 
            
            SyncLock syncLock(lock);
            if (scoreDoc == hq->addOverflow(scoreDoc))
                break;
        }
        return docs;
    }
    
    MultiIndexSearcherCallableWithSort::MultiIndexSearcherCallableWithSort(SynchronizePtr lock, IndexSearcherPtr searchable, WeightPtr weight, 
                                                                           FilterPtr filter, int32_t nDocs, TopFieldCollectorPtr hq, 
                                                                           SortPtr sort, int32_t i, Collection<int32_t> starts)
    {
        this->lock = lock;
        this->searchable = searchable;
        this->weight = weight;
        this->filter = filter;
        this->nDocs = nDocs;
        this->hq = hq;
        this->i = i;
        this->starts = starts;
        this->sort = sort;
        
        this->fakeScorer = newLucene<FakeScorer>();
    }
    
    MultiIndexSearcherCallableWithSort::~MultiIndexSearcherCallableWithSort()
    {
    }
    
    TopFieldDocsPtr MultiIndexSearcherCallableWithSort::call()
    {
        TopFieldDocsPtr docs(searchable->search(weight, filter, nDocs, sort));
        // If one of the Sort fields is FIELD_DOC, need to fix its values, so that it will break ties by doc Id 
        // properly. Otherwise, it will compare to 'relative' doc Ids, that belong to two different searchables.
        for (int32_t j = 0; j < docs->fields.size(); ++j)
        {
            if (docs->fields[j]->getType() == SortField::DOC)
            {
                // iterate over the score docs and change their fields value
                for (int32_t j2 = 0; j2 < docs->scoreDocs.size(); ++j2)
                {
                    FieldDocPtr fd(boost::static_pointer_cast<FieldDoc>(docs->scoreDocs[j2]));
                    fd->fields[j] = VariantUtils::get<int32_t>(fd->fields[j]) + starts[i];
                }
                break;
            }
        }
        
        {
            SyncLock syncLock(lock);
            hq->setNextReader(searchable->getIndexReader(), starts[i]);
            hq->setScorer(fakeScorer);
            for (Collection<ScoreDocPtr>::iterator scoreDoc = docs->scoreDocs.begin(); scoreDoc != docs->scoreDocs.end(); ++scoreDoc)
            {
                fakeScorer->doc = (*scoreDoc)->doc;
                fakeScorer->_score = (*scoreDoc)->score;
                hq->collect((*scoreDoc)->doc);
            }
        }
        
        return docs;
    }
    
    FakeScorer::FakeScorer() : Scorer(SimilarityPtr(), WeightPtr())
    {
        _score = 0;
        doc = 0;
    }
    
    FakeScorer::~FakeScorer()
    {
    }
    
    int32_t FakeScorer::advance(int32_t target)
    {
        boost::throw_exception(UnsupportedOperationException());
        return 0;
    }
    
    int32_t FakeScorer::docID()
    {
        return doc;
    }
    
    double FakeScorer::freq()
    {
        boost::throw_exception(UnsupportedOperationException());
        return 0;
    }
    
    int32_t FakeScorer::nextDoc()
    {
        boost::throw_exception(UnsupportedOperationException());
        return 0;
    }
    
    double FakeScorer::score()
    {
        return _score;
    }
}
