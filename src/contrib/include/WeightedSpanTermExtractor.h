/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef WEIGHTEDSPANTERMEXTRACTOR_H
#define WEIGHTEDSPANTERMEXTRACTOR_H

#include "LuceneContrib.h"
#include "FilterIndexReader.h"
#include "MapWeightedSpanTerm.h"

namespace Lucene
{
    /// Class used to extract {@link WeightedSpanTerm}s from a {@link Query} based on whether {@link Term}s 
    /// from the {@link Query} are contained in a supplied {@link TokenStream}.
    class LPPCONTRIBAPI WeightedSpanTermExtractor : public LuceneObject
    {
    public:
        WeightedSpanTermExtractor(const String& defaultField = L"");
        virtual ~WeightedSpanTermExtractor();
        
        LUCENE_CLASS(WeightedSpanTermExtractor);
    
    protected:
        String fieldName;
        TokenStreamPtr tokenStream;
        MapStringIndexReader readers;
        String defaultField;
        bool expandMultiTermQuery;
        bool cachedTokenStream;
        bool wrapToCaching;
    
    protected:
        void closeReaders();
        
        /// Fills a Map with {@link WeightedSpanTerm}s using the terms from the supplied Query.
        ///
        /// @param query Query to extract Terms from
        /// @param terms Map to place created WeightedSpanTerms in
        void extract(QueryPtr query, MapWeightedSpanTermPtr terms);
        
        /// Fills a Map with {@link WeightedSpanTerm}s using the terms from the supplied SpanQuery.
        ///
        /// @param terms Map to place created WeightedSpanTerms in.
        /// @param spanQuery SpanQuery to extract Terms from
        void extractWeightedSpanTerms(MapWeightedSpanTermPtr terms, SpanQueryPtr spanQuery);
        
        /// Fills a Map with {@link WeightedSpanTerm}s using the terms from the supplied Query.
        /// @param terms Map to place created WeightedSpanTerms in
        /// @param query Query to extract Terms from
        void extractWeightedTerms(MapWeightedSpanTermPtr terms, QueryPtr query);
        
        /// Necessary to implement matches for queries against defaultField
        bool fieldNameComparator(const String& fieldNameToCheck);
        
        IndexReaderPtr getReaderForField(const String& field);
        
        void collectSpanQueryFields(SpanQueryPtr spanQuery, HashSet<String> fieldNames);
        bool mustRewriteQuery(SpanQueryPtr spanQuery);

    public:
        /// Creates a Map of WeightedSpanTerms from the given Query and TokenStream.
        ///
        /// @param query That caused hit
        /// @param tokenStream Of text to be highlighted
        /// @return Map containing WeightedSpanTerms
        MapWeightedSpanTermPtr getWeightedSpanTerms(QueryPtr query, TokenStreamPtr tokenStream);
        
        /// Creates a Map of WeightedSpanTerms from the given Query and TokenStream.
        ///
        /// @param query That caused hit
        /// @param tokenStream Of text to be highlighted
        /// @param fieldName Restricts Term's used based on field name
        /// @return Map containing WeightedSpanTerms
        MapWeightedSpanTermPtr getWeightedSpanTerms(QueryPtr query, TokenStreamPtr tokenStream, const String& fieldName);
        
        /// Creates a Map of WeightedSpanTerms from the given Query and TokenStream.  Uses a supplied
        /// IndexReader to properly weight terms (for gradient highlighting).
        ///
        /// @param query That caused hit
        /// @param tokenStream Of text to be highlighted
        /// @param fieldName Restricts Term's used based on field name
        /// @param reader To use for scoring
        /// @return Map containing WeightedSpanTerms
        MapWeightedSpanTermPtr getWeightedSpanTermsWithScores(QueryPtr query, TokenStreamPtr tokenStream, const String& fieldName, IndexReaderPtr reader);
        
        bool getExpandMultiTermQuery();
        void setExpandMultiTermQuery(bool expandMultiTermQuery);
        
        bool isCachedTokenStream();
        TokenStreamPtr getTokenStream();
        
        /// By default, {@link TokenStream}s that are not of the type {@link CachingTokenFilter} 
        /// are wrapped in a {@link CachingTokenFilter} to ensure an efficient reset - if you 
        /// are already using a different caching {@link TokenStream} impl and you don't want 
        /// it to be wrapped, set this to false.
        void setWrapIfNotCachingTokenFilter(bool wrap);
    };
    
    /// This class makes sure that if both position sensitive and insensitive versions of the same 
    /// term are added, the position insensitive one wins.
    class LPPCONTRIBAPI PositionCheckingMap : public MapWeightedSpanTerm
    {
    public:
        virtual ~PositionCheckingMap();
        LUCENE_CLASS(PositionCheckingMap);
    
    public:
        virtual void put(const String& key, WeightedSpanTermPtr val);
    };
    
    /// A fake IndexReader class to extract the field from a MultiTermQuery
    class LPPCONTRIBAPI FakeReader : public FilterIndexReader
    {
    public:
        FakeReader();
        virtual ~FakeReader();
        
        LUCENE_CLASS(FakeReader);
    
    public:
        String field;
        
    protected:
        static IndexReaderPtr EMPTY_MEMORY_INDEX_READER();
    
    public:
        virtual TermEnumPtr terms(TermPtr t);
    };
}

#endif
