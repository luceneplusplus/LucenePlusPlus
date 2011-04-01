/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef TOPFIELDCOLLECTOR_H
#define TOPFIELDCOLLECTOR_H

#include "TopDocsCollector.h"

namespace Lucene
{
    /// A {@link Collector} that sorts by {@link SortField} using {@link FieldComparator}s.
    ///
    /// See the {@link #create(SortPtr, int32_t, bool, bool, bool, bool)} method for instantiating a TopFieldCollector.
    class LPPAPI TopFieldCollector : public TopDocsCollector
    {
    public:
        TopFieldCollector(HitQueueBasePtr pq, int32_t numHits, bool fillFields);
        virtual ~TopFieldCollector();
    
        LUCENE_CLASS(TopFieldCollector);
    
    protected:
        bool fillFields;
        
        /// Stores the maximum score value encountered, needed for normalizing.  If document scores are not tracked, 
        /// this value is initialized to NaN.
        double maxScore;
        
        int32_t numHits;
        FieldValueHitQueueEntryPtr bottom;
        bool queueFull;
        int32_t docBase;
    
    public:
        /// Creates a new {@link TopFieldCollector} from the given arguments.
        ///
        /// NOTE: The instances returned by this method pre-allocate a full array of length numHits.
        ///
        /// @param sort The sort criteria (SortFields).
        /// @param numHits The number of results to collect.
        /// @param fillFields Specifies whether the actual field values should be returned on the results (FieldDoc).
        /// @param trackDocScores Specifies whether document scores should be tracked and set on the results. Note 
        /// that if set to false, then the results' scores will be set to NaN.  Setting this to true affects 
        /// performance, as it incurs the score computation on each competitive result.  Therefore if document scores 
        /// are not required by the application, it is recommended to set it to false.
        /// @param trackMaxScore Specifies whether the query's maxScore should be tracked and set on the resulting 
        /// {@link TopDocs}.  Note that if set to false, {@link TopDocs#getMaxScore()} returns NaN. Setting this to
        /// true affects performance as it incurs the score computation on each result. Also, setting this true 
        /// automatically sets trackDocScores to true as well.
        /// @param docsScoredInOrder Specifies whether documents are scored in doc Id order or not by the given 
        /// {@link Scorer} in {@link #setScorer(ScorerPtr)}. 
        /// @return a {@link TopFieldCollector} instance which will sort the results by the sort criteria.
        static TopFieldCollectorPtr create(SortPtr sort, int32_t numHits, bool fillFields, bool trackDocScores, bool trackMaxScore, bool docsScoredInOrder);
        
        virtual void add(int32_t slot, int32_t doc, double score);
        
        virtual bool acceptsDocsOutOfOrder();
    
    protected:
        static const Collection<ScoreDocPtr> EMPTY_SCOREDOCS();
        
        /// Only the following callback methods need to be overridden since topDocs(int32_t, int32_t) calls them to 
        /// return the results.
        virtual void populateResults(Collection<ScoreDocPtr> results, int32_t howMany);
        
        virtual TopDocsPtr newTopDocs(Collection<ScoreDocPtr> results, int32_t start);
    };
    
    /// Implements a TopFieldCollector over one SortField criteria, without tracking document scores and maxScore.
    class LPPAPI OneComparatorNonScoringCollector : public TopFieldCollector
    {
    public:
        OneComparatorNonScoringCollector(FieldValueHitQueuePtr queue, int32_t numHits, bool fillFields);
        virtual ~OneComparatorNonScoringCollector();
    
        LUCENE_CLASS(OneComparatorNonScoringCollector);
    
    public:
        FieldComparatorPtr comparator;
        int32_t reverseMul;
    
    public:
        virtual void initialize();        
        virtual void updateBottom(int32_t doc);
        virtual void collect(int32_t doc);
        virtual void setNextReader(IndexReaderPtr reader, int32_t docBase);
        virtual void setScorer(ScorerPtr scorer);
    };
    
    /// Implements a TopFieldCollector over one SortField criteria, without tracking document scores and maxScore, 
    /// and assumes out of orderness in doc Ids collection.
    class LPPAPI OutOfOrderOneComparatorNonScoringCollector : public OneComparatorNonScoringCollector
    {
    public:
        OutOfOrderOneComparatorNonScoringCollector(FieldValueHitQueuePtr queue, int32_t numHits, bool fillFields);
        virtual ~OutOfOrderOneComparatorNonScoringCollector();
    
        LUCENE_CLASS(OutOfOrderOneComparatorNonScoringCollector);
    
    public:
        virtual void collect(int32_t doc);
        virtual bool acceptsDocsOutOfOrder();
    };
    
    /// Implements a TopFieldCollector over one SortField criteria, while tracking document scores but no maxScore.
    class LPPAPI OneComparatorScoringNoMaxScoreCollector : public OneComparatorNonScoringCollector
    {
    public:
        OneComparatorScoringNoMaxScoreCollector(FieldValueHitQueuePtr queue, int32_t numHits, bool fillFields);
        virtual ~OneComparatorScoringNoMaxScoreCollector();
    
        LUCENE_CLASS(OneComparatorScoringNoMaxScoreCollector);
    
    public:
        ScorerPtr scorer;
    
    public:
        virtual void updateBottom(int32_t doc, double score);
        virtual void collect(int32_t doc);
        virtual void setScorer(ScorerPtr scorer);
    };
    
    /// Implements a TopFieldCollector over one SortField criteria, while tracking document scores but no maxScore, 
    /// and assumes out of orderness in doc Ids collection.
    class LPPAPI OutOfOrderOneComparatorScoringNoMaxScoreCollector : public OneComparatorScoringNoMaxScoreCollector
    {
    public:
        OutOfOrderOneComparatorScoringNoMaxScoreCollector(FieldValueHitQueuePtr queue, int32_t numHits, bool fillFields);
        virtual ~OutOfOrderOneComparatorScoringNoMaxScoreCollector();
    
        LUCENE_CLASS(OutOfOrderOneComparatorScoringNoMaxScoreCollector);
    
    public:
        virtual void collect(int32_t doc);
        virtual bool acceptsDocsOutOfOrder();
    };
    
    /// Implements a TopFieldCollector over one SortField criteria, with tracking document scores and maxScore.
    class LPPAPI OneComparatorScoringMaxScoreCollector : public OneComparatorNonScoringCollector
    {
    public:
        OneComparatorScoringMaxScoreCollector(FieldValueHitQueuePtr queue, int32_t numHits, bool fillFields);
        virtual ~OneComparatorScoringMaxScoreCollector();
    
        LUCENE_CLASS(OneComparatorScoringMaxScoreCollector);
    
    public:
        ScorerPtr scorer;
    
    public:
        virtual void updateBottom(int32_t doc, double score);
        virtual void collect(int32_t doc);
        virtual void setScorer(ScorerPtr scorer);
    };
    
    /// Implements a TopFieldCollector over one SortField criteria, with tracking document scores and maxScore, 
    /// and assumes out of orderness in doc Ids collection.
    class LPPAPI OutOfOrderOneComparatorScoringMaxScoreCollector : public OneComparatorScoringMaxScoreCollector
    {
    public:
        OutOfOrderOneComparatorScoringMaxScoreCollector(FieldValueHitQueuePtr queue, int32_t numHits, bool fillFields);
        virtual ~OutOfOrderOneComparatorScoringMaxScoreCollector();
    
        LUCENE_CLASS(OutOfOrderOneComparatorScoringMaxScoreCollector);
    
    public:
        virtual void collect(int32_t doc);
        virtual bool acceptsDocsOutOfOrder();
    };
    
    /// Implements a TopFieldCollector over multiple SortField criteria, without tracking document scores and maxScore.
    class LPPAPI MultiComparatorNonScoringCollector : public TopFieldCollector
    {
    public:
        MultiComparatorNonScoringCollector(FieldValueHitQueuePtr queue, int32_t numHits, bool fillFields);
        virtual ~MultiComparatorNonScoringCollector();
    
        LUCENE_CLASS(MultiComparatorNonScoringCollector);
    
    public:
        Collection<FieldComparatorPtr> comparators;
        Collection<int32_t> reverseMul;
    
    public:
        virtual void initialize();
        virtual void updateBottom(int32_t doc);
        virtual void collect(int32_t doc);
        virtual void setNextReader(IndexReaderPtr reader, int32_t docBase);
        virtual void setScorer(ScorerPtr scorer);
    };
    
    /// Implements a TopFieldCollector over multiple SortField criteria, without tracking document scores and maxScore.
    class LPPAPI OutOfOrderMultiComparatorNonScoringCollector : public MultiComparatorNonScoringCollector
    {
    public:
        OutOfOrderMultiComparatorNonScoringCollector(FieldValueHitQueuePtr queue, int32_t numHits, bool fillFields);
        virtual ~OutOfOrderMultiComparatorNonScoringCollector();
    
        LUCENE_CLASS(OutOfOrderMultiComparatorNonScoringCollector);
    
    public:
        virtual void collect(int32_t doc);
        virtual bool acceptsDocsOutOfOrder();
    };
    
    /// Implements a TopFieldCollector over multiple SortField criteria, with tracking document scores and maxScore.
    class LPPAPI MultiComparatorScoringMaxScoreCollector : public MultiComparatorNonScoringCollector
    {
    public:
        MultiComparatorScoringMaxScoreCollector(FieldValueHitQueuePtr queue, int32_t numHits, bool fillFields);
        virtual ~MultiComparatorScoringMaxScoreCollector();
    
        LUCENE_CLASS(MultiComparatorScoringMaxScoreCollector);
    
    public:
        ScorerWeakPtr _scorer;
    
    public:
        virtual void updateBottom(int32_t doc, double score);
        virtual void collect(int32_t doc);
        virtual void setScorer(ScorerPtr scorer);
    };
    
    /// Implements a TopFieldCollector over multiple SortField criteria, without tracking document scores and maxScore.
    class LPPAPI OutOfOrderMultiComparatorScoringMaxScoreCollector : public MultiComparatorScoringMaxScoreCollector
    {
    public:
        OutOfOrderMultiComparatorScoringMaxScoreCollector(FieldValueHitQueuePtr queue, int32_t numHits, bool fillFields);
        virtual ~OutOfOrderMultiComparatorScoringMaxScoreCollector();
    
        LUCENE_CLASS(OutOfOrderMultiComparatorScoringMaxScoreCollector);
    
    public:
        virtual void collect(int32_t doc);
        virtual bool acceptsDocsOutOfOrder();
    };
    
    /// Implements a TopFieldCollector over multiple SortField criteria, with tracking document scores and maxScore.
    class LPPAPI MultiComparatorScoringNoMaxScoreCollector : public MultiComparatorNonScoringCollector
    {
    public:
        MultiComparatorScoringNoMaxScoreCollector(FieldValueHitQueuePtr queue, int32_t numHits, bool fillFields);
        virtual ~MultiComparatorScoringNoMaxScoreCollector();
    
        LUCENE_CLASS(MultiComparatorScoringNoMaxScoreCollector);
    
    public:
        ScorerWeakPtr _scorer;
    
    public:
        virtual void updateBottom(int32_t doc, double score);
        virtual void collect(int32_t doc);
        virtual void setScorer(ScorerPtr scorer);
    };
    
    /// Implements a TopFieldCollector over multiple SortField criteria, with tracking document scores and maxScore, 
    /// and assumes out of orderness in doc Ids collection.
    class LPPAPI OutOfOrderMultiComparatorScoringNoMaxScoreCollector : public MultiComparatorScoringNoMaxScoreCollector
    {
    public:
        OutOfOrderMultiComparatorScoringNoMaxScoreCollector(FieldValueHitQueuePtr queue, int32_t numHits, bool fillFields);
        virtual ~OutOfOrderMultiComparatorScoringNoMaxScoreCollector();
    
        LUCENE_CLASS(OutOfOrderMultiComparatorScoringNoMaxScoreCollector);
    
    public:
        virtual void collect(int32_t doc);
        virtual void setScorer(ScorerPtr scorer);
        virtual bool acceptsDocsOutOfOrder();
    };
}

#endif
