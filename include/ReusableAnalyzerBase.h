/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef REUSABLEANALYZERBASE_H
#define REUSABLEANALYZERBASE_H

#include "Analyzer.h"

namespace Lucene
{
    /// An convenience subclass of Analyzer that makes it easy to implement {@link TokenStream} reuse.
    ///
    /// ReusableAnalyzerBase is a simplification of Analyzer that supports easy reuse for the most 
    /// common use-cases. Analyzers such as {@link PerFieldAnalyzerWrapper} that behave differently 
    /// depending upon the field name need to subclass Analyzer directly instead.
    ///
    /// To prevent consistency problems, this class does not allow subclasses to extend {@link 
    /// #reusableTokenStream(String, Reader)} or {@link #tokenStream(String, Reader)} directly. 
    /// Instead, subclasses must implement {@link #createComponents(String, Reader)}.
    class LPPAPI ReusableAnalyzerBase : public Analyzer
    {
    public:
        virtual ~ReusableAnalyzerBase();
        
        LUCENE_CLASS(ReusableAnalyzerBase);
    
    protected:
        /// Creates a new {@link TokenStreamComponents} instance for this analyzer.
        /// @param fieldName the name of the fields content passed to the {@link TokenStreamComponents} 
        /// sink as a reader
        /// @param reader the reader passed to the {@link Tokenizer} constructor 
        /// @return the {@link TokenStreamComponents} for this analyzer.
        virtual TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader) = 0;
        
        /// Override this if you want to add a CharFilter chain.
        virtual ReaderPtr initReader(ReaderPtr reader);
    
    public:
        /// This method uses {@link #createComponents(String, Reader)} to obtain an instance of {@link 
        /// TokenStreamComponents}. It returns the sink of the components and stores the components 
        /// internally. Subsequent calls to this method will reuse the previously stored components if 
        /// and only if the {@link TokenStreamComponents#reset(Reader)} method returned true. 
        /// Otherwise a new instance of {@link TokenStreamComponents} is created.
        /// @param fieldName the name of the field the created TokenStream is used for
        /// @param reader the reader the streams source reads from
        virtual TokenStreamPtr reusableTokenStream(const String& fieldName, ReaderPtr reader);
        
        /// This method uses {@link #createComponents(String, Reader)} to obtain an instance of {@link 
        /// TokenStreamComponents} and returns the sink of the components. Each calls to this method 
        /// will create a new instance of {@link TokenStreamComponents}. Created {@link TokenStream} 
        /// instances are never reused.
        /// @param fieldName the name of the field the created TokenStream is used for
        /// @param reader the reader the streams source reads from
        virtual TokenStreamPtr tokenStream(const String& fieldName, ReaderPtr reader);
    };
    
    /// This class encapsulates the outer components of a token stream. It provides access to the source 
    /// ({@link Tokenizer}) and the outer end (sink), an instance of {@link TokenFilter} which also 
    /// serves as the {@link TokenStream} returned by {@link Analyzer#tokenStream(String, Reader)} and
    /// {@link Analyzer#reusableTokenStream(String, Reader)}.
    class LPPAPI TokenStreamComponents : public LuceneObject
    {
    public:
        /// Creates a new {@link TokenStreamComponents} instance.
        /// @param source the analyzer's tokenizer
        /// @param result the analyzer's resulting token stream
        TokenStreamComponents(TokenizerPtr source, TokenStreamPtr result);
        
        /// Creates a new {@link TokenStreamComponents} instance.
        /// @param source the analyzer's tokenizer
        TokenStreamComponents(TokenizerPtr source);
        
        virtual ~TokenStreamComponents();
        
        LUCENE_CLASS(TokenStreamComponents);
    
    protected:
        TokenizerPtr source;
        TokenStreamPtr sink;

    protected:
        /// Resets the encapsulated components with the given reader. This method by default returns true 
        /// indicating that the components have been reset successfully. Subclasses of {@link 
        /// ReusableAnalyzerBase} might use their own {@link TokenStreamComponents} returning false if
        /// the components cannot be reset.
        /// @param reader a reader to reset the source component
        /// @return true if the components were reset, otherwise false
        virtual bool reset(ReaderPtr reader);
        
        /// Returns the sink {@link TokenStream}
        /// @return the sink {@link TokenStream}
        virtual TokenStreamPtr getTokenStream();
        
        friend class ReusableAnalyzerBase;
    };
}

#endif
