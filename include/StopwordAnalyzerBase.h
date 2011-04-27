/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef STOPWORDANALYZERBASE_H
#define STOPWORDANALYZERBASE_H

#include "ReusableAnalyzerBase.h"

namespace Lucene
{
    /// Base class for Analyzers that need to make use of stopword sets.
    class LPPAPI StopwordAnalyzerBase : public ReusableAnalyzerBase
    {
    public:
        /// Creates a new instance initialized with the given stopword set
        /// @param version the Lucene version for cross version compatibility
        /// @param stopwords the analyzer's stopword set
        StopwordAnalyzerBase(LuceneVersion::Version version, HashSet<String> stopwords);
        
        /// Creates a new Analyzer with an empty stopword set
        /// @param version the Lucene version for cross version compatibility
        StopwordAnalyzerBase(LuceneVersion::Version version);
        
        virtual ~StopwordAnalyzerBase();
        
        LUCENE_CLASS(StopwordAnalyzerBase);
    
    protected:
        CharArraySetPtr stopwords;
        LuceneVersion::Version matchVersion;
    
    public:
        /// @return the analyzer's stopword set or an empty set if the analyzer has no stopwords
        virtual CharArraySetPtr getStopwordSet();

    protected:
        /// Creates a CharArraySet from a file resource associated with a class.
        /// @param ignoreCase true if the set should ignore the case of the stopwords, otherwise false
        /// @param resource name of the resource file associated with the given class 
        /// @param comment comment string to ignore in the stopword file
        /// @return a CharArraySet containing the distinct stopwords from the given file
        static CharArraySetPtr loadStopwordSet(bool ignoreCase, const String& resource, const String& comment);
    };
}

#endif
