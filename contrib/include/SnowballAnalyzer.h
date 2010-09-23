/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneContrib.h"
#include "Analyzer.h"

namespace Lucene
{
	/// Filters {@link StandardTokenizer} with {@link StandardFilter}, {@link LowerCaseFilter}, {@link StopFilter} 
	/// and {@link SnowballFilter}.
	///
	/// NOTE: This class uses the same {@link LuceneVersion#Version} dependent settings as {@link StandardAnalyzer}.
	class LPPAPI SnowballAnalyzer : public Analyzer
	{
	public:
	    /// Builds the named analyzer with no stop words.
	    SnowballAnalyzer(LuceneVersion::Version matchVersion, const String& name);
	    
	    /// Builds an analyzer with the given stop words.
	    SnowballAnalyzer(LuceneVersion::Version matchVersion, const String& name, HashSet<String> stopwords);
	    
	    virtual ~SnowballAnalyzer();
	    
	    LUCENE_CLASS(SnowballAnalyzer);
	
    protected:
        /// Contains the stopwords used with the StopFilter.
        HashSet<String> stopSet;
        
        String name;        
        LuceneVersion::Version matchVersion;
    
    public:
        /// Returns an unmodifiable instance of the default stop-words set.
	    static HashSet<String> getDefaultStopSet();
	    
	    /// Constructs a {@link StandardTokenizer} filtered by a {@link StandardFilter}, a {@link LowerCaseFilter}, 
	    /// a {@link StopFilter} and a {@link SnowballFilter}.
	    virtual TokenStreamPtr tokenStream(const String& fieldName, ReaderPtr reader);
	    
	    /// Returns a (possibly reused) {@link StandardTokenizer} filtered by a {@link StandardFilter}, a {@link 
	    /// LowerCaseFilter}, a {@link StopFilter} and a {@link SnowballFilter}.
	    virtual TokenStreamPtr reusableTokenStream(const String& fieldName, ReaderPtr reader);
	};
	
	class LPPAPI SnowballAnalyzerSavedStreams : public LuceneObject
	{
	public:
	    virtual ~SnowballAnalyzerSavedStreams();
	    
	    LUCENE_CLASS(SnowballAnalyzerSavedStreams);

    public:
        TokenizerPtr source;
        TokenStreamPtr result;
	};
}
