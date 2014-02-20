/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include "LuceneContrib.h"
#include "PriorityQueue.h"

namespace Lucene {

/// Class used to markup highlighted terms found in the best sections of a text, using configurable
/// {@link Fragmenter}, {@link Scorer}, {@link Formatter}, {@link Encoder} and tokenizers.
class LPPCONTRIBAPI Highlighter : public LuceneObject {
public:
    Highlighter(const HighlighterScorerPtr& fragmentScorer);
    Highlighter(const FormatterPtr& formatter, const HighlighterScorerPtr& fragmentScorer);
    Highlighter(const FormatterPtr& formatter, const EncoderPtr& encoder, const HighlighterScorerPtr& fragmentScorer);

    virtual ~Highlighter();

    LUCENE_CLASS(Highlighter);

public:
    static const int32_t DEFAULT_MAX_CHARS_TO_ANALYZE;

protected:
    int32_t maxDocCharsToAnalyze;
    FormatterPtr formatter;
    EncoderPtr encoder;
    FragmenterPtr textFragmenter;
    HighlighterScorerPtr fragmentScorer;

public:
    /// Highlights chosen terms in a text, extracting the most relevant section.  This is a convenience
    /// method that calls {@link #getBestFragment(TokenStreamPtr, const String&)}
    ///
    /// @param analyzer The analyzer that will be used to split text into chunks
    /// @param text Text to highlight terms in
    /// @param fieldName Name of field used to influence analyzer's tokenization policy
    /// @return highlighted text fragment or null if no terms found
    String getBestFragment(const AnalyzerPtr& analyzer, const String& fieldName, const String& text);

    /// Highlights chosen terms in a text, extracting the most relevant section.  The document text is
    /// analyzed in chunks to record hit statistics across the document. After accumulating stats, the
    /// fragment with the highest score is returned.
    ///
    /// @param tokenStream A stream of tokens identified in the text parameter, including offset
    /// information.  This is typically produced by an analyzer re-parsing a document's text. Some
    /// work may be done on retrieving TokenStreams more efficiently by adding support for storing
    /// original text position data in the Lucene index but this support is not currently available.
    /// @param text Text to highlight terms in
    /// @return highlighted text fragment or null if no terms found
    String getBestFragment(const TokenStreamPtr& tokenStream, const String& text);

    /// Highlights chosen terms in a text, extracting the most relevant sections.  This is a convenience
    /// method that calls {@link #getBestFragments(TokenStreamPtr, const String&, int32_t)}
    ///
    /// @param analyzer The analyzer that will be used to split text into chunks
    /// @param fieldName The name of the field being highlighted (used by analyzer)
    /// @param text Text to highlight terms in
    /// @param maxNumFragments The maximum number of fragments.
    /// @return highlighted text fragments (between 0 and maxNumFragments number of fragments)
    Collection<String> getBestFragments(const AnalyzerPtr& analyzer, const String& fieldName, const String& text, int32_t maxNumFragments);

    /// Highlights chosen terms in a text, extracting the most relevant sections.  The document text is
    /// analyzed in chunks to record hit statistics across the document. After accumulating stats, the
    /// fragments with the highest scores are returned as an array of strings in order of score (contiguous
    /// fragments are merged into one in their original order to improve readability)
    ///
    /// @param text Text to highlight terms in
    /// @param maxNumFragments The maximum number of fragments.
    /// @return highlighted Text fragments (between 0 and maxNumFragments number of fragments)
    Collection<String> getBestFragments(const TokenStreamPtr& tokenStream, const String& text, int32_t maxNumFragments);

    /// Low level api to get the most relevant (formatted) sections of the document.
    /// This method has been made public to allow visibility of score information held in TextFragment objects.
    Collection<TextFragmentPtr> getBestTextFragments(const TokenStreamPtr& tokenStream, const String& text, bool merge, int32_t maxNumFragments);

    /// Improves readability of a score-sorted list of TextFragments by merging any fragments that were
    /// contiguous in the original text into one larger fragment with the correct order.  This will leave
    /// a "null" in the array entry for the lesser scored fragment.
    ///
    /// @param frag An array of document fragments in descending score
    void mergeContiguousFragments(Collection<TextFragmentPtr> frag);

    /// Highlights terms in the  text , extracting the most relevant sections and concatenating the chosen
    /// fragments with a separator (typically "...").  The document text is analyzed in chunks to record
    /// hit statistics across the document.  After accumulating stats, the fragments with the highest scores
    /// are returned in order as "separator" delimited strings.
    ///
    /// @param text Text to highlight terms in
    /// @param maxNumFragments The maximum number of fragments.
    /// @param separator The separator used to intersperse the document fragments (typically "...")
    /// @return highlighted text
    String getBestFragments(const TokenStreamPtr& tokenStream, const String& text, int32_t maxNumFragments, const String& separator);

    int32_t getMaxDocCharsToAnalyze();
    void setMaxDocCharsToAnalyze(int32_t maxDocCharsToAnalyze);
    FragmenterPtr getTextFragmenter();
    void setTextFragmenter(const FragmenterPtr& fragmenter);

    /// @return Object used to score each text fragment
    HighlighterScorerPtr getFragmentScorer();

    void setFragmentScorer(const HighlighterScorerPtr& scorer);

    EncoderPtr getEncoder();
    void setEncoder(const EncoderPtr& encoder);
};

class LPPCONTRIBAPI FragmentQueue : public PriorityQueue<TextFragmentPtr> {
public:
    FragmentQueue(int32_t size);
    virtual ~FragmentQueue();

    LUCENE_CLASS(FragmentQueue);

protected:
    virtual bool lessThan(const TextFragmentPtr& first, const TextFragmentPtr& second);
};

}

#endif
