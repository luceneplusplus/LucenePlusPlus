/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "Searcher.h"
#include "Similarity.h"
#include "Query.h"

namespace Lucene
{
    Searcher::Searcher()
    {
        similarity = Similarity::getDefault();
    }
    
    Searcher::~Searcher()
    {
    }
    
    TopFieldDocsPtr Searcher::search(QueryPtr query, FilterPtr filter, int32_t n, SortPtr sort)
    {
        return search(createWeight(query), filter, n, sort);
    }
    
    void Searcher::search(QueryPtr query, CollectorPtr results)
    {
        search(createWeight(query), FilterPtr(), results);
    }
    
    void Searcher::search(QueryPtr query, FilterPtr filter, CollectorPtr results)
    {
        search(createWeight(query), filter, results);
    }
    
    TopDocsPtr Searcher::search(QueryPtr query, FilterPtr filter, int32_t n)
    {
        return search(createWeight(query), filter, n);
    }
    
    TopDocsPtr Searcher::search(QueryPtr query, int32_t n)
    {
        return search(query, FilterPtr(), n);
    }
    
    ExplanationPtr Searcher::explain(QueryPtr query, int32_t doc)
    {
        return explain(createWeight(query), doc);
    }
    
    void Searcher::setSimilarity(SimilarityPtr similarity)
    {
        this->similarity = similarity;
    }
    
    SimilarityPtr Searcher::getSimilarity()
    {
        return this->similarity;
    }
    
    WeightPtr Searcher::createWeight(QueryPtr query)
    {
        return query->weight(shared_from_this());
    }
    
    Collection<int32_t> Searcher::docFreqs(Collection<TermPtr> terms)
    {
        Collection<int32_t> result(Collection<int32_t>::newInstance(terms.size()));
        for (int32_t i = 0; i < terms.size(); ++i)
            result[i] = docFreq(terms[i]);
        return result;
    }
}
