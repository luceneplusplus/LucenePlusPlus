/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef TOKENSOURCES_H
#define TOKENSOURCES_H

#include "LuceneContrib.h"
#include "TokenStream.h"

namespace Lucene {

/// Hides implementation issues associated with obtaining a TokenStream for use with the highlighter - can obtain
/// from TermFreqVectors with offsets and (optionally) positions or from Analyzer class re-parsing the stored content.
class LPPCONTRIBAPI TokenSources : public LuceneObject {
public:
    virtual ~TokenSources();
    LUCENE_CLASS(TokenSources);

public:
    /// A convenience method that tries to first get a TermPositionVector for the specified docId, then, falls back to
    /// using the passed in {@link Document} to retrieve the TokenStream.  This is useful when you already have the
    /// document, but would prefer to use the vector first.
    /// @param reader The {@link IndexReader} to use to try and get the vector from.
    /// @param docId The docId to retrieve.
    /// @param field The field to retrieve on the document.
    /// @param doc The document to fall back on.
    /// @param analyzer The analyzer to use for creating the TokenStream if the vector doesn't exist.
    /// @return The {@link TokenStream} for the {@link Fieldable} on the {@link Document}
    static TokenStreamPtr getAnyTokenStream(const IndexReaderPtr& reader, int32_t docId, const String& field, const DocumentPtr& doc, const AnalyzerPtr& analyzer);

    /// A convenience method that tries a number of approaches to getting a token stream.  The cost of finding there
    /// are no termVectors in the index is minimal (1000 invocations still registers 0 ms). So this "lazy" (flexible?)
    /// approach to coding is probably acceptable
    static TokenStreamPtr getAnyTokenStream(const IndexReaderPtr& reader, int32_t docId, const String& field, const AnalyzerPtr& analyzer);

    static TokenStreamPtr getTokenStream(const TermPositionVectorPtr& tpv);

    /// Low level api.
    /// Returns a token stream or null if no offset info available in index.  This can be used to feed the highlighter
    /// with a pre-parsed token stream.
    ///
    /// In my tests the speeds to recreate 1000 token streams using this method are:
    /// - with TermVector offset only data stored - 420  milliseconds
    /// - with TermVector offset AND position data stored - 271 milliseconds
    /// (nb timings for TermVector with position data are based on a tokenizer with contiguous positions - no overlaps
    /// or gaps)  The cost of not using TermPositionVector to store pre-parsed content and using an analyzer to re-parse
    /// the original content:
    /// - reanalyzing the original content - 980 milliseconds
    ///
    /// The re-analyze timings will typically vary depending on -
    /// 1) The complexity of the analyzer code (timings above were using a stemmer/lowercaser/stopword combo)
    /// 2) The  number of other fields (Lucene reads ALL fields off the disk when accessing just one document field -
    /// can cost dear!)
    /// 3) Use of compression on field storage - could be faster due to compression (less disk IO) or slower (more CPU
    /// burn) depending on the content.
    ///
    /// @param tpv
    /// @param tokenPositionsGuaranteedContiguous true if the token position numbers have no overlaps or gaps. If looking
    /// to eek out the last drops of performance, set to true.  If in doubt, set to false.
    static TokenStreamPtr getTokenStream(const TermPositionVectorPtr& tpv, bool tokenPositionsGuaranteedContiguous);

    static TokenStreamPtr getTokenStream(const IndexReaderPtr& reader, int32_t docId, const String& field);
    static TokenStreamPtr getTokenStream(const IndexReaderPtr& reader, int32_t docId, const String& field, const AnalyzerPtr& analyzer);
    static TokenStreamPtr getTokenStream(const DocumentPtr& doc, const String& field, const AnalyzerPtr& analyzer);
    static TokenStreamPtr getTokenStream(const String& field, const String& contents, const AnalyzerPtr& analyzer);
};

/// an object used to iterate across an array of tokens
class LPPCONTRIBAPI StoredTokenStream : public TokenStream {
public:
    StoredTokenStream(Collection<TokenPtr> tokens);
    virtual ~StoredTokenStream();

    LUCENE_CLASS(StoredTokenStream);

public:
    Collection<TokenPtr> tokens;
    int32_t currentToken;
    TermAttributePtr termAtt;
    OffsetAttributePtr offsetAtt;

public:
    virtual bool incrementToken();
};

}

#endif
