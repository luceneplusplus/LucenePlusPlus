/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef STOPFILTER_H
#define STOPFILTER_H

#include "FilteringTokenFilter.h"

namespace Lucene
{
    /// Removes stop words from a token stream.
    ///
    /// You must specify the required {@link Version} compatibility when creating StopFilter:
    /// <ul>
    ///    <li> As of 3.1, StopFilter correctly handles Unicode 4.0 supplementary characters in 
    ///    stopwords and position increments are preserved
    /// </ul>
    class LPPAPI StopFilter : public FilteringTokenFilter
    {
    public:
        /// Construct a token stream filtering the given input.  If stopWords is an instance of 
        /// {@link CharArraySet} (true if makeStopSet() was used to construct the set) it will 
        /// be directly used and ignoreCase will be ignored since CharArraySet directly controls 
        /// case sensitivity.
        ///
        /// If stopWords is not an instance of {@link CharArraySet}, a new CharArraySet will be 
        /// constructed and ignoreCase will be used to specify the case sensitivity of that set.
        ///
        /// @param enablePositionIncrements true if token positions should record the removed 
        /// stop words
        /// @param input Input TokenStream
        /// @param stopWords A Set of Strings or char[] or any other toString()-able set 
        /// representing the stopwords
        /// @param ignoreCase if true, all words are lower cased first
        /// @deprecated use {@link #StopFilter(Version, TokenStream, Set, boolean)} instead
        StopFilter(bool enablePositionIncrements, TokenStreamPtr input, HashSet<String> stopWords, bool ignoreCase = false);
        
        /// Construct a token stream filtering the given input.  If stopWords is an instance of 
        /// {@link CharArraySet} (true if makeStopSet() was used to construct the set) it will 
        /// be directly used and ignoreCase will be ignored since CharArraySet directly controls 
        /// case sensitivity.
        ///
        /// If stopWords is not an instance of {@link CharArraySet}, a new CharArraySet will be 
        /// constructed and ignoreCase will be used to specify the case sensitivity of that set.
        ///
        /// @param enablePositionIncrements true if token positions should record the removed 
        /// stop words
        /// @param input Input TokenStream
        /// @param stopWords A Set of Strings or char[] or any other toString()-able set 
        /// representing the stopwords
        /// @param ignoreCase if true, all words are lower cased first
        /// @deprecated use {@link #StopFilter(Version, TokenStream, Set, boolean)} instead
        StopFilter(bool enablePositionIncrements, TokenStreamPtr input, CharArraySetPtr stopWords);
        
        /// Construct a token stream filtering the given input. If stopWords is an instance of 
        /// {@link CharArraySet} (true if makeStopSet() was used to construct the set) it will 
        /// be directly used and ignoreCase will be ignored since CharArraySet directly controls 
        /// case sensitivity.
        ///
        /// If stopWords is not an instance of {@link CharArraySet}, a new CharArraySet will be 
        /// constructed and ignoreCase will be used to specify the case sensitivity of that set.
        ///
        /// @param matchVersion Lucene version to enable correct Unicode 4.0 behavior in the stop
        /// set if Version > 3.0.
        /// @param input Input TokenStream
        /// @param stopWords A Set of Strings or char[] or any other toString()-able set
        /// representing the stopwords
        /// @param ignoreCase if true, all words are lower cased first
        StopFilter(LuceneVersion::Version matchVersion, TokenStreamPtr input, HashSet<String> stopWords, bool ignoreCase = false);
        
        /// Construct a token stream filtering the given input. If stopWords is an instance of 
        /// {@link CharArraySet} (true if makeStopSet() was used to construct the set) it will 
        /// be directly used and ignoreCase will be ignored since CharArraySet directly controls 
        /// case sensitivity.
        ///
        /// If stopWords is not an instance of {@link CharArraySet}, a new CharArraySet will be 
        /// constructed and ignoreCase will be used to specify the case sensitivity of that set.
        ///
        /// @param matchVersion Lucene version to enable correct Unicode 4.0 behavior in the stop
        /// set if Version > 3.0.
        /// @param input Input TokenStream
        /// @param stopWords A Set of Strings or char[] or any other toString()-able set
        /// representing the stopwords
        StopFilter(LuceneVersion::Version matchVersion, TokenStreamPtr input, CharArraySetPtr stopWords);
        
        /// Construct a token stream filtering the given input. If stopWords is an instance of 
        /// {@link CharArraySet} (true if makeStopSet() was used to construct the set) it will 
        /// be directly used and ignoreCase will be ignored since CharArraySet directly controls 
        /// case sensitivity.
        ///
        /// If stopWords is not an instance of {@link CharArraySet}, a new CharArraySet will be 
        /// constructed and ignoreCase will be used to specify the case sensitivity of that set.
        ///
        /// @param matchVersion Lucene version to enable correct Unicode 4.0 behavior in the stop
        /// set if Version > 3.0.
        /// @param enablePositionIncrements true if token positions should record the removed 
        /// stop words
        /// @param input Input TokenStream
        /// @param stopWords A Set of Strings or char[] or any other toString()-able set
        /// representing the stopwords
        StopFilter(LuceneVersion::Version matchVersion, bool enablePositionIncrements, TokenStreamPtr input, CharArraySetPtr stopWords);
        
        virtual ~StopFilter();
        
        LUCENE_CLASS(StopFilter);
    
    protected:
        CharArraySetPtr stopWords;
        CharTermAttributePtr termAtt;
    
    public:
        /// Creates a stopword set from the given stopword array.
        /// @param stopWords An array of stopwords
        /// @param ignoreCase If true, all words are lower cased first.  
        /// @return a Set containing the words
        static CharArraySetPtr makeStopSet(LuceneVersion::Version matchVersion, Collection<String> stopWords, bool ignoreCase = false);
        
        /// Returns version-dependent default for enablePositionIncrements.  Analyzers that embed StopFilter use this 
        /// method when creating the StopFilter.  Prior to 2.9, this returns false.  On 2.9 or later, it returns true.
        /// @deprecated use {@link #StopFilter(Version, TokenStream, Set)} instead
        static bool getEnablePositionIncrementsVersionDefault(LuceneVersion::Version matchVersion);
        
    protected:
        /// Construct StopFilter and initialize values
        void ConstructStopFilter(LuceneVersion::Version matchVersion, bool enablePositionIncrements, TokenStreamPtr input, CharArraySetPtr stopWords);
        
        /// Returns the next input Token whose term() is not a stop word.
        virtual bool accept();
    };
}

#endif
