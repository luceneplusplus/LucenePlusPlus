/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef MEMORYINDEX_H
#define MEMORYINDEX_H

#include "LuceneContrib.h"
#include "IndexReader.h"
#include "TermEnum.h"
#include "Collector.h"
#include "TermPositions.h"
#include "TermPositionVector.h"

namespace Lucene {

/// High-performance single-document main memory Lucene fulltext search index.
///
/// Overview
///
/// This class is a replacement/substitute for a large subset of {@link RAMDirectory} functionality.
/// It is designed to enable maximum efficiency for on-the-fly matchmaking combining structured and
/// fuzzy fulltext search in realtime streaming applications such as Nux XQuery based XML message
/// queues, publish-subscribe systems for Blogs/newsfeeds, text chat, data acquisition and
/// distribution systems, application level routers, firewalls, classifiers, etc.  Rather than
/// targeting fulltext search of infrequent queries over huge persistent data archives (historic
/// search), this class targets fulltext search of huge numbers of queries over comparatively small
/// transient realtime data (prospective search).
///
/// For example as in
/// <pre>
/// double score = search(const String& text, const QueryPtr& query)
/// </pre>
///
/// Each instance can hold at most one Lucene "document", with a document containing zero or more
/// "fields", each field having a name and a fulltext value.  The fulltext value is tokenized
/// (split and transformed) into zero or more index terms (aka words) on addField(), according to
/// the policy implemented by an Analyzer.  For example, Lucene analyzers can split on whitespace,
/// normalize to lower case for case insensitivity, ignore common terms with little discriminatory
/// value such as "he", "in", "and" (stop words), reduce the terms to their natural linguistic root
/// form such as "fishing" being reduced to "fish" (stemming), resolve synonyms/inflexions/thesauri
/// (upon indexing and/or querying), etc.
///
/// Note that a Lucene query selects on the field names and associated (indexed) tokenized terms,
/// not on the original fulltext(s) - the latter are not stored but rather thrown away immediately
/// after tokenization.
///
/// For some interesting background information on search technology, see Bob Wyman's <a target="_blank"
/// href="http://bobwyman.pubsub.com/main/2005/05/mary_hodder_poi.html">Prospective Search</a>,
/// Jim Gray's <a target="_blank" href="http://www.acmqueue.org/modules.php?name=Content&pa=showpage&pid=293&page=4">
/// A Call to Arms - Custom subscriptions</a>, and Tim Bray's <a target="_blank"
/// href="http://www.tbray.org/ongoing/When/200x/2003/07/30/OnSearchTOC">On Search, the Series</a>.
///
///
/// Example Usage
/// <pre>
/// AnalyzerPtr analyzer = newLucene<SimpleAnalyzer>();
/// MemoryIndexPtr index = newLucene<MemoryIndex>();
/// index->addField(L"content", L"Readings about Salmons and other select Alaska fishing Manuals", analyzer);
/// index->addField(L"author", L"Tales of James", analyzer);
/// QueryParserPtr parser = newLucene<QueryParser>(L"content", analyzer);
/// double score = index->search(parser->parse(L"+author:james +salmon~ +fish* manual~"));
/// if (score > 0.0)
/// {
///     // it's a match
/// }
/// else
/// {
///     // no match found
/// }
/// </pre>
///
///
/// Performance Notes
///
/// Internally there's a new data structure geared towards efficient indexing and searching, plus
/// the necessary support code to seamlessly plug into the Lucene framework.
///
/// This class performs very well for very small texts (eg. 10 chars) as well as for large texts
/// (eg. 10 MB) and everything in between.  Typically, it is about 10-100 times faster than
/// RAMDirectory.  Note that RAMDirectory has particularly large efficiency overheads for small to
/// medium sized texts, both in time and space.  Indexing a field with N tokens takes O(N) in the
/// best case, and O(N logN) in the worst case.  Memory consumption is probably larger than for
/// RAMDirectory.
///
class LPPCONTRIBAPI MemoryIndex : public LuceneObject {
public:
    /// Constructs an empty instance that can optionally store the start and end character offset
    /// of each token term in the text.  This can be useful for highlighting of hit locations with
    /// the Lucene highlighter package.  Private until the highlighter package matures, so that
    /// this can actually be meaningfully integrated.
    /// @param storeOffsets Whether or not to store the start and end character offset of each
    /// token term in the text.
    MemoryIndex(bool storeOffsets = false);

    virtual ~MemoryIndex();

    LUCENE_CLASS(MemoryIndex);

protected:
    /// info for each field
    MapStringMemoryIndexInfo fields;

    /// fields sorted ascending by fieldName; lazily computed on demand
    CollectionStringMemoryIndexInfo sortedFields;

    /// pos: positions[3 * i], startOffset: positions[3 * i + 1], endOffset: positions[3 * i + 2]
    int32_t stride;

    static const double docBoost;

public:
    /// Convenience method; Tokenizes the given field text and adds the resulting terms to the
    /// index; Equivalent to adding an indexed non-keyword Lucene {@link Field} that is {@link
    /// Field::INDEX_ANALYZED tokenized}, {@link Field::STORE_NO not stored}, {@link
    /// Field::TERM_VECTOR_WITH_POSITIONS termVectorStored with positions} (or {@link
    /// Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS termVectorStored with positions and offsets})
    /// @param fieldName A name to be associated with the text
    /// @param text The text to tokenize and index.
    /// @param analyzer The analyzer to use for tokenization
    void addField(const String& fieldName, const String& text, const AnalyzerPtr& analyzer);

    /// Iterates over the given token stream and adds the resulting terms to the index;
    /// Equivalent to adding a tokenized, indexed, termVectorStored, unstored, Lucene {@link
    /// Field}.  Finally closes the token stream. Note that untokenized keywords can be added
    /// with this method via  {@link #keywordTokenStream(Collection)}, the Lucene contrib
    /// KeywordTokenizer or similar utilities.
    /// @param fieldName A name to be associated with the text.
    /// @param stream The token stream to retrieve tokens from.
    /// @param boost The boost factor for hits for this field.
    /// @see Field#setBoost(double)
    void addField(const String& fieldName, const TokenStreamPtr& stream, double boost = 1.0);

    /// Creates and returns a searcher that can be used to execute arbitrary Lucene queries
    /// and to collect the resulting query results as hits.
    /// @return a searcher
    IndexSearcherPtr createSearcher();

    /// Convenience method that efficiently returns the relevance score by matching this index
    /// against the given Lucene query expression.
    /// @param query An arbitrary Lucene query to run against this index
    /// @return the relevance score of the matchmaking; A number in the range [0.0 .. 1.0],
    /// with 0.0 indicating no match. The higher the number the better the match.
    double search(const QueryPtr& query);

protected:
    int32_t numPositions(Collection<int32_t> positions);

    /// sorts into ascending order (on demand), reusing memory along the way
    void sortFields();

    friend class MemoryIndexReader;
    friend class MemoryIndexInfo;
    friend class MemoryIndexTermEnum;
    friend class MemoryIndexTermPositions;
    friend class MemoryIndexTermPositionVector;
};

/// Index data structure for a field; Contains the tokenized term texts and their positions.
class LPPCONTRIBAPI MemoryIndexInfo : public LuceneObject {
public:
    MemoryIndexInfo(MapStringIntCollection terms, int32_t numTokens, int32_t numOverlapTokens, double boost);
    virtual ~MemoryIndexInfo();

    LUCENE_CLASS(MemoryIndexInfo);

protected:
    /// Term strings and their positions for this field
    MapStringIntCollection terms;

    /// Terms sorted ascending by term text; computed on demand
    CollectionStringIntCollection sortedTerms;

    /// Number of added tokens for this field
    int32_t numTokens;

    /// Number of overlapping tokens for this field
    int32_t numOverlapTokens;

    /// Boost factor for hits for this field
    double boost;

    /// Term for this field's fieldName, lazily computed on demand
    TermPtr _template;

public:
    /// Sorts hashed terms into ascending order, reusing memory along the way.  Note that
    /// sorting is lazily delayed until required (often it's not required at all).
    void sortTerms();

    /// Note that the frequency can be calculated as numPosition(getPositions(x))
    Collection<int32_t> getPositions(const String& term);

    /// Note that the frequency can be calculated as numPosition(getPositions(x))
    Collection<int32_t> getPositions(int32_t pos);

    double getBoost();

    friend class MemoryIndexReader;
    friend class MemoryIndexTermEnum;
    friend class MemoryIndexTermPositions;
    friend class MemoryIndexTermPositionVector;
};

/// Search support for Lucene framework integration; implements all methods required by the
/// Lucene IndexReader contracts.
class LPPCONTRIBAPI MemoryIndexReader : public IndexReader {
public:
    MemoryIndexReader(const MemoryIndexPtr& memoryIndex);
    virtual ~MemoryIndexReader();

    LUCENE_CLASS(MemoryIndexReader);

public:
    static TermPtr MATCH_ALL_TERM();

protected:
    MemoryIndexPtr memoryIndex;
    SearcherWeakPtr _searcher; // needed to find searcher.getSimilarity()

    /// cache norms to avoid repeated expensive calculations
    ByteArray cachedNorms;
    String cachedFieldName;
    SimilarityPtr cachedSimilarity;

protected:
    MemoryIndexInfoPtr getInfo(const String& fieldName);
    MemoryIndexInfoPtr getInfo(int32_t pos);

    SimilarityPtr getSimilarity();
    void setSearcher(const SearcherPtr& searcher);

public:
    virtual int32_t docFreq(const TermPtr& t);
    virtual TermEnumPtr terms();
    virtual TermEnumPtr terms(const TermPtr& t);
    virtual TermPositionsPtr termPositions();
    virtual TermDocsPtr termDocs();
    virtual Collection<TermFreqVectorPtr> getTermFreqVectors(int32_t docNumber);
    virtual void getTermFreqVector(int32_t docNumber, const String& field, const TermVectorMapperPtr& mapper);
    virtual void getTermFreqVector(int32_t docNumber, const TermVectorMapperPtr& mapper);
    virtual TermFreqVectorPtr getTermFreqVector(int32_t docNumber, const String& field);
    virtual ByteArray norms(const String& field);
    virtual void norms(const String& field, ByteArray norms, int32_t offset);
    virtual void doSetNorm(int32_t doc, const String& field, uint8_t value);
    virtual int32_t numDocs();
    virtual int32_t maxDoc();
    virtual DocumentPtr document(int32_t n);
    virtual DocumentPtr document(int32_t n, const FieldSelectorPtr& fieldSelector);
    virtual bool isDeleted(int32_t n);
    virtual bool hasDeletions();
    virtual void doDelete(int32_t docNum);
    virtual void doUndeleteAll();
    virtual void doCommit(MapStringString commitUserData);
    virtual void doClose();
    virtual HashSet<String> getFieldNames(FieldOption fieldOption);

    friend class MemoryIndex;
    friend class MemoryIndexTermEnum;
    friend class MemoryIndexTermPositions;
    friend class MemoryIndexTermPositionVector;
};

class LPPCONTRIBAPI MemoryIndexTermEnum : public TermEnum {
public:
    MemoryIndexTermEnum(const MemoryIndexReaderPtr& reader, int32_t ix, int32_t jx);
    virtual ~MemoryIndexTermEnum();

    LUCENE_CLASS(MemoryIndexTermEnum);

protected:
    MemoryIndexReaderWeakPtr _reader;
    int32_t i;
    int32_t j;

public:
    virtual bool next();
    virtual TermPtr term();
    virtual int32_t docFreq();
    virtual void close();

protected:
    TermPtr createTerm(const MemoryIndexInfoPtr& info, int32_t pos, const String& text);
};

class LPPCONTRIBAPI MemoryIndexCollector : public Collector {
public:
    MemoryIndexCollector(Collection<double> scores);
    virtual ~MemoryIndexCollector();

    LUCENE_CLASS(MemoryIndexCollector);

protected:
    Collection<double> scores;
    ScorerPtr scorer;

public:
    virtual void collect(int32_t doc);
    virtual void setScorer(const ScorerPtr& scorer);
    virtual bool acceptsDocsOutOfOrder();
    virtual void setNextReader(const IndexReaderPtr& reader, int32_t docBase);
};

class LPPCONTRIBAPI MemoryIndexTermPositions : public TermPositions, public LuceneObject {
public:
    MemoryIndexTermPositions(const MemoryIndexReaderPtr& reader);
    virtual ~MemoryIndexTermPositions();

    LUCENE_CLASS(MemoryIndexTermPositions);

protected:
    MemoryIndexReaderWeakPtr _reader;
    bool hasNext;
    int32_t cursor;
    Collection<int32_t> current;
    TermPtr term;

public:
    virtual void seek(const TermPtr& term);
    virtual void seek(const TermEnumPtr& termEnum);
    virtual int32_t doc();
    virtual int32_t freq();
    virtual bool next();
    virtual int32_t read(Collection<int32_t> docs, Collection<int32_t> freqs);
    virtual bool skipTo(int32_t target);
    virtual void close();

    virtual int32_t nextPosition();
    virtual int32_t getPayloadLength();
    virtual ByteArray getPayload(ByteArray data, int32_t offset);
    virtual bool isPayloadAvailable();
};

class MemoryIndexTermPositionVector : public TermPositionVector, public LuceneObject {
public:
    MemoryIndexTermPositionVector(const MemoryIndexReaderPtr& reader, const MemoryIndexInfoPtr& info, const String& fieldName);
    virtual ~MemoryIndexTermPositionVector();

    LUCENE_CLASS(MemoryIndexTermPositionVector);

protected:
    MemoryIndexReaderWeakPtr _reader;
    CollectionStringIntCollection sortedTerms;
    String fieldName;

public:
    virtual String getField();
    virtual int32_t size();
    virtual Collection<String> getTerms();
    virtual Collection<int32_t> getTermFrequencies();
    virtual int32_t indexOf(const String& term);
    virtual Collection<int32_t> indexesOf(Collection<String> terms, int32_t start, int32_t length);

    virtual Collection<int32_t> getTermPositions(int32_t index);
    virtual Collection<TermVectorOffsetInfoPtr> getOffsets(int32_t index);
};

}

#endif
