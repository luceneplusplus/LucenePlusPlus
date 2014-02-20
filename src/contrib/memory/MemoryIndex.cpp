/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "MemoryIndex.h"
#include "TokenStream.h"
#include "Analyzer.h"
#include "StringReader.h"
#include "TermAttribute.h"
#include "PositionIncrementAttribute.h"
#include "OffsetAttribute.h"
#include "IndexSearcher.h"
#include "Term.h"
#include "Scorer.h"
#include "TermFreqVector.h"
#include "TermVectorOffsetInfo.h"
#include "TermVectorMapper.h"
#include "Similarity.h"
#include "FieldInvertState.h"
#include "Document.h"
#include "MiscUtils.h"

namespace Lucene {

const double MemoryIndex::docBoost = 1.0;

MemoryIndex::MemoryIndex(bool storeOffsets) {
    stride = storeOffsets ? 3 : 1;
    fields = MapStringMemoryIndexInfo::newInstance();
}

MemoryIndex::~MemoryIndex() {
}

void MemoryIndex::addField(const String& fieldName, const String& text, const AnalyzerPtr& analyzer) {
    if (fieldName.empty()) {
        boost::throw_exception(IllegalArgumentException(L"fieldName must not be empty"));
    }
    if (text.empty()) {
        boost::throw_exception(IllegalArgumentException(L"text must not be empty"));
    }
    if (!analyzer) {
        boost::throw_exception(IllegalArgumentException(L"analyzer must not be null"));
    }

    TokenStreamPtr stream(analyzer->tokenStream(fieldName, newLucene<StringReader>(text)));
    addField(fieldName, stream);
}

void MemoryIndex::addField(const String& fieldName, const TokenStreamPtr& stream, double boost) {
    LuceneException finally;
    try {
        if (fieldName.empty()) {
            boost::throw_exception(IllegalArgumentException(L"fieldName must not be empty"));
        }
        if (!stream) {
            boost::throw_exception(IllegalArgumentException(L"token stream must not be null"));
        }
        if (boost <= 0.0) {
            boost::throw_exception(IllegalArgumentException(L"boost factor must be greater than 0.0"));
        }
        if (fields.contains(fieldName)) {
            boost::throw_exception(IllegalArgumentException(L"field must not be added more than once"));
        }

        MapStringIntCollection terms(MapStringIntCollection::newInstance());
        int32_t numTokens = 0;
        int32_t numOverlapTokens = 0;
        int32_t pos = -1;

        TermAttributePtr termAtt(stream->addAttribute<TermAttribute>());
        PositionIncrementAttributePtr posIncrAttribute(stream->addAttribute<PositionIncrementAttribute>());
        OffsetAttributePtr offsetAtt(stream->addAttribute<OffsetAttribute>());

        stream->reset();
        while (stream->incrementToken()) {
            String term(termAtt->term());
            if (term.empty()) {
                continue;    // nothing to do
            }
            ++numTokens;
            int32_t posIncr = posIncrAttribute->getPositionIncrement();
            if (posIncr == 0) {
                ++numOverlapTokens;
            }
            pos += posIncr;

            Collection<int32_t> positions(terms.get(term));
            if (!positions) {
                // term not seen before
                positions = Collection<int32_t>::newInstance();
                terms.put(term, positions);
            }
            positions.add(pos);
            if (stride != 1) {
                positions.add(offsetAtt->startOffset());
                positions.add(offsetAtt->endOffset());
            }
        }
        stream->end();

        // ensure infos.numTokens > 0 invariant; needed for correct operation of terms()
        if (numTokens > 0) {
            boost = boost * docBoost; // see DocumentWriter.addDocument(...)
            fields.put(fieldName, newLucene<MemoryIndexInfo>(terms, numTokens, numOverlapTokens, boost));
            sortedFields.reset();    // invalidate sorted view, if any
        }
    } catch (IOException& e) {
        // can never happen
        boost::throw_exception(RuntimeException(e.getError()));
    } catch (LuceneException& e) {
        finally = e;
    }
    try {
        if (stream) {
            stream->close();
        }
    } catch (IOException& e) {
        boost::throw_exception(RuntimeException(e.getError()));
    }
    finally.throwException();
}

IndexSearcherPtr MemoryIndex::createSearcher() {
    MemoryIndexReaderPtr reader(newLucene<MemoryIndexReader>(shared_from_this()));
    IndexSearcherPtr searcher(newLucene<IndexSearcher>(reader)); // ensures no auto-close
    reader->setSearcher(searcher); // to later get hold of searcher.getSimilarity()
    return searcher;
}

double MemoryIndex::search(const QueryPtr& query) {
    if (!query) {
        boost::throw_exception(IllegalArgumentException(L"query must not be null"));
    }

    SearcherPtr searcher(createSearcher());
    LuceneException finally;
    try {
        Collection<double> scores = Collection<double>::newInstance(1);
        scores[0] = 0.0; // inits to 0.0 (no match)
        searcher->search(query, newLucene<MemoryIndexCollector>(scores));
        return scores[0];
    } catch (IOException& e) {
        // can never happen
        boost::throw_exception(RuntimeException(e.getError()));
    } catch (LuceneException& e) {
        finally = e;
    }
    finally.throwException();
    return 0; // silence static analyzers
}

int32_t MemoryIndex::numPositions(Collection<int32_t> positions) {
    return (positions.size() / stride);
}

struct lessField {
    inline bool operator()(const PairStringMemoryIndexInfo& first, const PairStringMemoryIndexInfo& second) const {
        return (first.first < second.first);
    }
};

void MemoryIndex::sortFields() {
    if (!sortedFields) {
        sortedFields = CollectionStringMemoryIndexInfo::newInstance(fields.begin(), fields.end());
        std::sort(sortedFields.begin(), sortedFields.end(), lessField());
    }
}

MemoryIndexInfo::MemoryIndexInfo(MapStringIntCollection terms, int32_t numTokens, int32_t numOverlapTokens, double boost) {
    this->terms = terms;
    this->numTokens = numTokens;
    this->numOverlapTokens = numOverlapTokens;
    this->boost = boost;
}

MemoryIndexInfo::~MemoryIndexInfo() {
}

struct lessTerm {
    inline bool operator()(const PairStringIntCollection& first, const PairStringIntCollection& second) const {
        return (first.first < second.first);
    }
};

void MemoryIndexInfo::sortTerms() {
    if (!sortedTerms) {
        sortedTerms = CollectionStringIntCollection::newInstance(terms.begin(), terms.end());
        std::sort(sortedTerms.begin(), sortedTerms.end(), lessTerm());
    }
}

Collection<int32_t> MemoryIndexInfo::getPositions(const String& term) {
    return terms.get(term);
}

Collection<int32_t> MemoryIndexInfo::getPositions(int32_t pos) {
    return sortedTerms[pos].second;
}

double MemoryIndexInfo::getBoost() {
    return boost;
}

MemoryIndexReader::MemoryIndexReader(const MemoryIndexPtr& memoryIndex) {
    this->memoryIndex = memoryIndex;
}

MemoryIndexReader::~MemoryIndexReader() {
}

TermPtr MemoryIndexReader::MATCH_ALL_TERM() {
    static TermPtr _MATCH_ALL_TERM;
    if (!_MATCH_ALL_TERM) {
        _MATCH_ALL_TERM = newLucene<Term>(L"");
        CycleCheck::addStatic(_MATCH_ALL_TERM);
    }
    return _MATCH_ALL_TERM;
}

MemoryIndexInfoPtr MemoryIndexReader::getInfo(const String& fieldName) {
    return memoryIndex->fields.get(fieldName);
}

MemoryIndexInfoPtr MemoryIndexReader::getInfo(int32_t pos) {
    return memoryIndex->sortedFields[pos].second;
}

int32_t MemoryIndexReader::docFreq(const TermPtr& t) {
    MemoryIndexInfoPtr info(getInfo(t->field()));
    int32_t freq = 0;
    if (info) {
        freq = info->getPositions(t->text()) ? 1 : 0;
    }
    return freq;
}

TermEnumPtr MemoryIndexReader::terms() {
    return terms(MATCH_ALL_TERM());
}

TermEnumPtr MemoryIndexReader::terms(const TermPtr& t) {
    int32_t i = 0; // index into info.sortedTerms
    int32_t j = 0; // index into sortedFields

    memoryIndex->sortFields();
    if (memoryIndex->sortedFields.size() == 1 && memoryIndex->sortedFields[0].first == t->field()) {
        j = 0;    // fast path
    } else {
        CollectionStringMemoryIndexInfo::iterator search = std::lower_bound(memoryIndex->sortedFields.begin(), memoryIndex->sortedFields.end(), std::make_pair(t->field(), MemoryIndexInfoPtr()), lessField());
        int32_t keyPos = std::distance(memoryIndex->sortedFields.begin(), search);
        j = (search == memoryIndex->sortedFields.end() || t->field() < search->first) ? -(keyPos + 1) : keyPos;
    }

    if (j < 0) { // not found; choose successor
        j = -j - 1;
        i = 0;
        if (j < memoryIndex->sortedFields.size()) {
            getInfo(j)->sortTerms();
        }
    } else { // found
        MemoryIndexInfoPtr info(getInfo(j));
        info->sortTerms();
        CollectionStringIntCollection::iterator search = std::lower_bound(info->sortedTerms.begin(), info->sortedTerms.end(), std::make_pair(t->text(), Collection<int32_t>()), lessTerm());
        int32_t keyPos = std::distance(info->sortedTerms.begin(), search);
        i = (search == info->sortedTerms.end() || t->text() < search->first) ? -(keyPos + 1) : keyPos;
        if (i < 0) { // not found; choose successor
            i = -i - 1;
            if (i >= info->sortedTerms.size()) { // move to next successor
                ++j;
                i = 0;
                if (j < memoryIndex->sortedFields.size()) {
                    getInfo(j)->sortTerms();
                }
            }
        }
    }

    return newLucene<MemoryIndexTermEnum>(shared_from_this(), i, j);
}

TermPositionsPtr MemoryIndexReader::termPositions() {
    return newLucene<MemoryIndexTermPositions>(shared_from_this());
}

TermDocsPtr MemoryIndexReader::termDocs() {
    return termPositions();
}

Collection<TermFreqVectorPtr> MemoryIndexReader::getTermFreqVectors(int32_t docNumber) {
    Collection<TermFreqVectorPtr> vectors(Collection<TermFreqVectorPtr>::newInstance());
    for (MapStringMemoryIndexInfo::iterator fieldName = memoryIndex->fields.begin(); fieldName != memoryIndex->fields.end(); ++fieldName) {
        vectors.add(getTermFreqVector(docNumber, fieldName->first));
    }
    return vectors;
}

void MemoryIndexReader::getTermFreqVector(int32_t docNumber, const TermVectorMapperPtr& mapper) {
    for (MapStringMemoryIndexInfo::iterator fieldName = memoryIndex->fields.begin(); fieldName != memoryIndex->fields.end(); ++fieldName) {
        getTermFreqVector(docNumber, fieldName->first, mapper);
    }
}

void MemoryIndexReader::getTermFreqVector(int32_t docNumber, const String& field, const TermVectorMapperPtr& mapper) {
    MemoryIndexInfoPtr info(getInfo(field));
    if (!info) {
        return;
    }
    info->sortTerms();
    mapper->setExpectations(field, info->sortedTerms.size(), memoryIndex->stride != 1, true);
    for (int32_t i = info->sortedTerms.size(); --i >=0;) {
        Collection<int32_t> positions(info->sortedTerms[i].second);
        int32_t size = positions.size();
        Collection<TermVectorOffsetInfoPtr> offsets(Collection<TermVectorOffsetInfoPtr>::newInstance(size / memoryIndex->stride));
        for (int32_t k = 0, j = 1; j < size; ++k, j += memoryIndex->stride) {
            int32_t start = positions[j];
            int32_t end = positions[j + 1];
            offsets[k] = newLucene<TermVectorOffsetInfo>(start, end);
        }
        mapper->map(info->sortedTerms[i].first, memoryIndex->numPositions(info->sortedTerms[i].second), offsets, info->sortedTerms[i].second);
    }
}

TermFreqVectorPtr MemoryIndexReader::getTermFreqVector(int32_t docNumber, const String& field) {
    MemoryIndexInfoPtr info(getInfo(field));
    if (!info) {
        return TermFreqVectorPtr();
    }
    info->sortTerms();
    return newLucene<MemoryIndexTermPositionVector>(shared_from_this(), info, field);
}

SimilarityPtr MemoryIndexReader::getSimilarity() {
    SearcherPtr searcher(_searcher.lock());
    if (searcher) {
        return searcher->getSimilarity();
    }
    return Similarity::getDefault();
}

void MemoryIndexReader::setSearcher(const SearcherPtr& searcher) {
    _searcher = searcher;
}

ByteArray MemoryIndexReader::norms(const String& field) {
    ByteArray norms(cachedNorms);
    SimilarityPtr sim(getSimilarity());
    if (field != cachedFieldName || sim != cachedSimilarity) { // not cached?
        MemoryIndexInfoPtr info(getInfo(field));
        int32_t numTokens = info ? info->numTokens : 0;
        int32_t numOverlapTokens = info ? info->numOverlapTokens : 0;
        double boost = info ? info->getBoost() : 1.0;
        FieldInvertStatePtr invertState(newLucene<FieldInvertState>(0, numTokens, numOverlapTokens, 0, boost));
        double n = sim->computeNorm(field, invertState);
        uint8_t norm = Similarity::encodeNorm(n);
        norms = ByteArray::newInstance(1);
        norms[0] = norm;

        // cache it for future reuse
        cachedNorms = norms;
        cachedFieldName = field;
        cachedSimilarity = sim;
    }
    return norms;
}

void MemoryIndexReader::norms(const String& field, ByteArray norms, int32_t offset) {
    ByteArray _norms(this->norms(field));
    MiscUtils::arrayCopy(_norms.get(), 0, norms.get(), offset, _norms.size());
}

void MemoryIndexReader::doSetNorm(int32_t doc, const String& field, uint8_t value) {
    boost::throw_exception(UnsupportedOperationException());
}

int32_t MemoryIndexReader::numDocs() {
    return memoryIndex->fields.empty() ? 0 : 1;
}

int32_t MemoryIndexReader::maxDoc() {
    return 1;
}

DocumentPtr MemoryIndexReader::document(int32_t n) {
    return newLucene<Document>(); // there are no stored fields
}

DocumentPtr MemoryIndexReader::document(int32_t n, const FieldSelectorPtr& fieldSelector) {
    return newLucene<Document>(); // there are no stored fields
}

bool MemoryIndexReader::isDeleted(int32_t n) {
    return false;
}

bool MemoryIndexReader::hasDeletions() {
    return false;
}

void MemoryIndexReader::doDelete(int32_t docNum) {
    boost::throw_exception(UnsupportedOperationException());
}

void MemoryIndexReader::doUndeleteAll() {
    boost::throw_exception(UnsupportedOperationException());
}

void MemoryIndexReader::doCommit(MapStringString commitUserData) {
}

void MemoryIndexReader::doClose() {
}

HashSet<String> MemoryIndexReader::getFieldNames(FieldOption fieldOption) {
    static HashSet<String> emptySet;
    if (!emptySet) {
        emptySet = HashSet<String>::newInstance();
    }
    if (fieldOption == FIELD_OPTION_UNINDEXED) {
        return emptySet;
    }
    if (fieldOption == FIELD_OPTION_INDEXED_NO_TERMVECTOR) {
        return emptySet;
    }
    if (fieldOption == FIELD_OPTION_TERMVECTOR_WITH_OFFSET && memoryIndex->stride == 1) {
        return emptySet;
    }
    if (fieldOption == FIELD_OPTION_TERMVECTOR_WITH_POSITION_OFFSET && memoryIndex->stride == 1) {
        return emptySet;
    }
    HashSet<String> fieldSet(HashSet<String>::newInstance());
    for (MapStringMemoryIndexInfo::iterator field = memoryIndex->fields.begin(); field != memoryIndex->fields.end(); ++field) {
        fieldSet.add(field->first);
    }
    return fieldSet;
}

MemoryIndexTermEnum::MemoryIndexTermEnum(const MemoryIndexReaderPtr& reader, int32_t ix, int32_t jx) {
    _reader = reader;
    i = ix;
    j = jx;
}

MemoryIndexTermEnum::~MemoryIndexTermEnum() {
}

bool MemoryIndexTermEnum::next() {
    MemoryIndexReaderPtr reader(_reader);
    if (j >= reader->memoryIndex->sortedFields.size()) {
        return false;
    }
    MemoryIndexInfoPtr info(reader->getInfo(j));
    if (++i < info->sortedTerms.size()) {
        return true;
    }

    // move to successor
    ++j;
    i = 0;
    if (j >= reader->memoryIndex->sortedFields.size()) {
        return false;
    }
    reader->getInfo(j)->sortTerms();
    return true;
}

TermPtr MemoryIndexTermEnum::term() {
    MemoryIndexReaderPtr reader(_reader);
    if (j >= reader->memoryIndex->sortedFields.size()) {
        return TermPtr();
    }
    MemoryIndexInfoPtr info(reader->getInfo(j));
    if (i >= info->sortedTerms.size()) {
        return TermPtr();
    }
    return createTerm(info, j, info->sortedTerms[i].first);
}

int32_t MemoryIndexTermEnum::docFreq() {
    MemoryIndexReaderPtr reader(_reader);
    if (j >= reader->memoryIndex->sortedFields.size()) {
        return 0;
    }
    MemoryIndexInfoPtr info(reader->getInfo(j));
    if (i >= info->sortedTerms.size()) {
        return 0;
    }
    return reader->memoryIndex->numPositions(info->getPositions(i));
}

void MemoryIndexTermEnum::close() {
}

TermPtr MemoryIndexTermEnum::createTerm(const MemoryIndexInfoPtr& info, int32_t pos, const String& text) {
    TermPtr _template(info->_template);
    if (!_template) { // not yet cached?
        MemoryIndexReaderPtr reader(_reader);
        String fieldName(reader->memoryIndex->sortedFields[pos].first);
        _template = newLucene<Term>(fieldName);
        info->_template = _template;
    }
    return _template->createTerm(text);
}

MemoryIndexCollector::MemoryIndexCollector(Collection<double> scores) {
    this->scores = scores;
}

MemoryIndexCollector::~MemoryIndexCollector() {
}

void MemoryIndexCollector::collect(int32_t doc) {
    scores[0] = scorer->score();
}

void MemoryIndexCollector::setScorer(const ScorerPtr& scorer) {
    this->scorer = scorer;
}

bool MemoryIndexCollector::acceptsDocsOutOfOrder() {
    return true;
}

void MemoryIndexCollector::setNextReader(const IndexReaderPtr& reader, int32_t docBase) {
}

MemoryIndexTermPositions::MemoryIndexTermPositions(const MemoryIndexReaderPtr& reader) {
    _reader = reader;
    hasNext = false;
    cursor = 0;
}

MemoryIndexTermPositions::~MemoryIndexTermPositions() {
}

void MemoryIndexTermPositions::seek(const TermPtr& term) {
    this->term = term;
    if (!term) {
        hasNext = true;    // term == null means match all docs
    } else {
        MemoryIndexReaderPtr reader(_reader);
        MemoryIndexInfoPtr info(reader->getInfo(term->field()));
        current = info ? info->getPositions(term->text()) : Collection<int32_t>();
        hasNext = current;
        cursor = 0;
    }
}

void MemoryIndexTermPositions::seek(const TermEnumPtr& termEnum) {
    seek(termEnum->term());
}

int32_t MemoryIndexTermPositions::doc() {
    return 0;
}

int32_t MemoryIndexTermPositions::freq() {
    MemoryIndexReaderPtr reader(_reader);
    int32_t freq = current ? reader->memoryIndex->numPositions(current) : (term ? 0 : 1);
    return freq;
}

bool MemoryIndexTermPositions::next() {
    bool _next = hasNext;
    hasNext = false;
    return _next;
}

int32_t MemoryIndexTermPositions::read(Collection<int32_t> docs, Collection<int32_t> freqs) {
    if (!hasNext) {
        return 0;
    }
    hasNext = false;
    docs[0] = 0;
    freqs[0] = freq();
    return 1;
}

bool MemoryIndexTermPositions::skipTo(int32_t target) {
    return next();
}

void MemoryIndexTermPositions::close() {
}

int32_t MemoryIndexTermPositions::nextPosition() {
    // implements TermPositions
    MemoryIndexReaderPtr reader(_reader);
    int32_t pos = current[cursor];
    cursor += reader->memoryIndex->stride;
    return pos;
}

int32_t MemoryIndexTermPositions::getPayloadLength() {
    boost::throw_exception(UnsupportedOperationException());
}

ByteArray MemoryIndexTermPositions::getPayload(ByteArray data, int32_t offset) {
    boost::throw_exception(UnsupportedOperationException());
    return ByteArray();
}

bool MemoryIndexTermPositions::isPayloadAvailable() {
    return false; // unsupported
}

MemoryIndexTermPositionVector::MemoryIndexTermPositionVector(const MemoryIndexReaderPtr& reader, const MemoryIndexInfoPtr& info, const String& fieldName) {
    this->_reader = reader;
    this->sortedTerms = info->sortedTerms;
    this->fieldName = fieldName;
}

MemoryIndexTermPositionVector::~MemoryIndexTermPositionVector() {
}

String MemoryIndexTermPositionVector::getField() {
    return fieldName;
}

int32_t MemoryIndexTermPositionVector::size() {
    return sortedTerms.size();
}

Collection<String> MemoryIndexTermPositionVector::getTerms() {
    Collection<String> terms(Collection<String>::newInstance(sortedTerms.size()));
    for (int32_t i = sortedTerms.size(); --i >= 0;) {
        terms[i] = sortedTerms[i].first;
    }
    return terms;
}

Collection<int32_t> MemoryIndexTermPositionVector::getTermFrequencies() {
    MemoryIndexReaderPtr reader(_reader);
    Collection<int32_t> freqs(Collection<int32_t>::newInstance(sortedTerms.size()));
    for (int32_t i = sortedTerms.size(); --i >= 0;) {
        freqs[i] = reader->memoryIndex->numPositions(sortedTerms[i].second);
    }
    return freqs;
}

int32_t MemoryIndexTermPositionVector::indexOf(const String& term) {
    CollectionStringIntCollection::iterator search = std::lower_bound(sortedTerms.begin(), sortedTerms.end(), std::make_pair(term, Collection<int32_t>()), lessTerm());
    return (search == sortedTerms.end() || term < search->first) ? -1 : std::distance(sortedTerms.begin(), search);
}

Collection<int32_t> MemoryIndexTermPositionVector::indexesOf(Collection<String> terms, int32_t start, int32_t length) {
    Collection<int32_t> indexes(Collection<int32_t>::newInstance(length));
    for (int32_t i = 0; i < length; ++i) {
        indexes[i] = indexOf(terms[start++]);
    }
    return indexes;
}

Collection<int32_t> MemoryIndexTermPositionVector::getTermPositions(int32_t index) {
    return sortedTerms[index].second;
}

Collection<TermVectorOffsetInfoPtr> MemoryIndexTermPositionVector::getOffsets(int32_t index) {
    MemoryIndexReaderPtr reader(_reader);
    if (reader->memoryIndex->stride == 1) {
        return Collection<TermVectorOffsetInfoPtr>();    // no offsets stored
    }

    Collection<int32_t> positions(sortedTerms[index].second);
    int32_t size = positions.size();
    Collection<TermVectorOffsetInfoPtr> offsets(Collection<TermVectorOffsetInfoPtr>::newInstance(size / reader->memoryIndex->stride));
    for (int32_t i = 0, j = 1; j < size; ++i, j += reader->memoryIndex->stride) {
        int32_t start = positions[j];
        int32_t end = positions[j + 1];
        offsets[i] = newLucene<TermVectorOffsetInfo>(start, end);
    }
    return offsets;
}

}
