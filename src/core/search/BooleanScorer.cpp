/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "BooleanScorer.h"
#include "_BooleanScorer.h"
#include "Similarity.h"
#include "Weight.h"

namespace Lucene
{
    BooleanScorer::BooleanScorer(WeightPtr weight, bool disableCoord, SimilarityPtr similarity, int32_t minNrShouldMatch, 
                                 Collection<ScorerPtr> optionalScorers, Collection<ScorerPtr> prohibitedScorers, int32_t maxCoord) : Scorer(weight)
    {
        this->bucketTable = newLucene<BucketTable>();
        this->prohibitedMask = 0;
        this->nextMask = 1;
        this->minNrShouldMatch = minNrShouldMatch;
        this->end = 0;
        this->doc = -1;
                
        if (optionalScorers && !optionalScorers.empty())
        {
            for (Collection<ScorerPtr>::iterator scorer = optionalScorers.begin(); scorer != optionalScorers.end(); ++scorer)
            {
                if ((*scorer)->nextDoc() != NO_MORE_DOCS)
                    scorers = newLucene<SubScorer>(*scorer, false, false, bucketTable->newCollector(0), scorers);
            }
        }
        
        if (prohibitedScorers && !prohibitedScorers.empty())
        {
            for (Collection<ScorerPtr>::iterator scorer = prohibitedScorers.begin(); scorer != prohibitedScorers.end(); ++scorer)
            {
                int32_t mask = nextMask;
                nextMask = nextMask << 1;
                prohibitedMask |= mask; // update prohibited mask
                if ((*scorer)->nextDoc() != NO_MORE_DOCS)
                    scorers = newLucene<SubScorer>(*scorer, false, true, bucketTable->newCollector(mask), scorers);
            }
        }
        
        coordFactors = DoubleArray::newInstance(optionalScorers.size() + 1);
        for (int32_t i = 0; i < coordFactors.size(); ++i)
            coordFactors[i] = disableCoord ? 1.0 : similarity->coord(i, maxCoord); 
    }
    
    BooleanScorer::~BooleanScorer()
    {
    }
    
    bool BooleanScorer::score(CollectorPtr collector, int32_t max, int32_t firstDocID)
    {
        bool more = false;
        BucketPtr tmp;
        BucketScorerPtr bs(newLucene<BucketScorer>(weight));
        // The internal loop will set the score and doc before calling collect.
        collector->setScorer(bs);
        do
        {
            bucketTable->first.reset();
            
            while (current) // more queued
            {
                // check prohibited and required
                if ((current->bits & prohibitedMask) == 0)
                {
                    if (current->doc >= max)
                    {
                        tmp = current;
                        current = current->_next.lock();
                        tmp->_next = bucketTable->first;
                        bucketTable->first = tmp;
                        continue;
                    }
                    
                    if (current->coord >= minNrShouldMatch)
                    {
                        bs->_score = current->score * coordFactors[current->coord];
                        bs->doc = current->doc;
                        bs->_freq = current->coord;
                        collector->collect(current->doc);
                    }
                }
                
                current = current->_next.lock(); // pop the queue
            }
            
            if (bucketTable->first)
            {
                current = bucketTable->first;
                bucketTable->first = current->_next.lock();
                return true;
            }
            
            // refill the queue
            more = false;
            end += BucketTable::SIZE;
            
            for (SubScorerPtr sub(scorers); sub; sub = sub->next)
            {
                int32_t subScorerDocID = sub->scorer->docID();
                if (subScorerDocID != NO_MORE_DOCS)
                {
                    if (sub->scorer->score(sub->collector, end, subScorerDocID))
                        more = true;
                }
            }
            current = bucketTable->first;
        }
        while (current || more);
        
        return false;
    }
    
    int32_t BooleanScorer::advance(int32_t target)
    {
        boost::throw_exception(UnsupportedOperationException());
        return 0;
    }
    
    int32_t BooleanScorer::docID()
    {
        return doc;
    }
    
    int32_t BooleanScorer::nextDoc()
    {
        bool more = false;
        do
        {
            while (bucketTable->first) // more queued
            {
                current = bucketTable->first;
                bucketTable->first = current->_next.lock(); // pop the queue
                
                // check prohibited and required and minNrShouldMatch
                if ((current->bits & prohibitedMask) == 0 && current->coord >= minNrShouldMatch)
                {
                    doc = current->doc;
                    return doc;
                }
            }
            
            // refill the queue
            more = false;
            end += BucketTable::SIZE;
            
            for (SubScorerPtr sub(scorers); sub; sub = sub->next)
            {
                int32_t subScorerDocID = sub->scorer->docID();
                if (subScorerDocID != NO_MORE_DOCS)
                {
                    if (sub->scorer->score(sub->collector, end, subScorerDocID))
                        more = true;
                }
            }
        }
        while (bucketTable->first || more);
        
        doc = NO_MORE_DOCS;
        return doc;
    }
    
    double BooleanScorer::score()
    {
        return current->score * coordFactors[current->coord];
    }
    
    void BooleanScorer::score(CollectorPtr collector)
    {
        score(collector, INT_MAX, nextDoc());
    }
    
    String BooleanScorer::toString()
    {
        StringStream buffer;
        buffer << L"boolean(";
        for (SubScorerPtr sub(scorers); sub; sub = sub->next)
            buffer << sub->scorer->toString() << L" ";
        buffer << L")";
        return buffer.str();
    }
    
    void BooleanScorer::visitSubScorers(QueryPtr parent, BooleanClause::Occur relationship, ScorerVisitorPtr visitor)
    {
        Scorer::visitSubScorers(parent, relationship, visitor);
        QueryPtr q(weight->getQuery());
        SubScorerPtr sub(scorers);
        while (sub)
        {
            if (!sub->prohibited)
                relationship = BooleanClause::SHOULD;
            else
                relationship = BooleanClause::MUST_NOT;
            sub->scorer->visitSubScorers(q, relationship, visitor);
            sub = sub->next;
        }
    }
    
    BooleanScorerCollector::BooleanScorerCollector(int32_t mask, BucketTablePtr bucketTable)
    {
        this->mask = mask;
        this->_bucketTable = bucketTable;
    }
    
    BooleanScorerCollector::~BooleanScorerCollector()
    {
    }
    
    void BooleanScorerCollector::collect(int32_t doc)
    {
        BucketTablePtr table(_bucketTable);
        int32_t i = doc & BucketTable::MASK;
        BucketPtr bucket(table->buckets[i]);
        if (!bucket)
        {
            bucket = newLucene<Bucket>();
            table->buckets[i] = bucket;
        }
        
        if (bucket->doc != doc) // invalid bucket
        {
            bucket->doc = doc; // set doc
            bucket->score = ScorerPtr(_scorer)->score(); // initialize score
            bucket->bits = mask; // initialize mask
            bucket->coord = 1; // initialize coord
            
            bucket->_next = table->first; // push onto valid list
            table->first = bucket;
        }
        else
        {
            bucket->score += ScorerPtr(_scorer)->score(); // increment score
            bucket->bits |= mask; // add bits in mask
            ++bucket->coord; // increment coord
        }
    }
    
    void BooleanScorerCollector::setNextReader(IndexReaderPtr reader, int32_t docBase)
    {
        // not needed by this implementation
    }
    
    void BooleanScorerCollector::setScorer(ScorerPtr scorer)
    {
        this->_scorer = scorer;
    }
    
    bool BooleanScorerCollector::acceptsDocsOutOfOrder()
    {
        return true;
    }
    
    BucketScorer::BucketScorer(WeightPtr weight) : Scorer(weight)
    {
        _score = 0;
        doc = NO_MORE_DOCS;
        this->_freq = 0;
    }
    
    BucketScorer::~BucketScorer()
    {
    }
    
    int32_t BucketScorer::advance(int32_t target)
    {
        return NO_MORE_DOCS;
    }
    
    int32_t BucketScorer::docID()
    {
        return doc;
    }
    
    double BucketScorer::freq()
    {
        return _freq;
    }
    
    int32_t BucketScorer::nextDoc()
    {
        return NO_MORE_DOCS;
    }
    
    double BucketScorer::score()
    {
        return _score;
    }
    
    Bucket::Bucket()
    {
        doc = -1;
        score = 0;
        bits = 0;
        coord = 0;
    }
    
    Bucket::~Bucket()
    {
    }
    
    const int32_t BucketTable::SIZE = 1 << 11;
    const int32_t BucketTable::MASK = BucketTable::SIZE - 1;
    
    BucketTable::BucketTable()
    {
        buckets = Collection<BucketPtr>::newInstance(SIZE);
    }
    
    BucketTable::~BucketTable()
    {
    }
    
    CollectorPtr BucketTable::newCollector(int32_t mask)
    {
        return newLucene<BooleanScorerCollector>(mask, shared_from_this());
    }
    
    int32_t BucketTable::size()
    {
        return SIZE;
    }
    
    SubScorer::SubScorer(ScorerPtr scorer, bool required, bool prohibited, CollectorPtr collector, SubScorerPtr next)
    {
        if (required)
            boost::throw_exception(IllegalArgumentException(L"this scorer cannot handle required=true"));
        this->scorer = scorer;
        this->prohibited = prohibited;
        this->collector = collector;
        this->next = next;
    }
    
    SubScorer::~SubScorer()
    {
    }
}
