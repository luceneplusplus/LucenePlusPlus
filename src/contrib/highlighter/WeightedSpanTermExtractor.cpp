/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "WeightedSpanTermExtractor.h"
#include "IndexReader.h"
#include "IndexSearcher.h"
#include "BooleanQuery.h"
#include "BooleanClause.h"
#include "PhraseQuery.h"
#include "Term.h"
#include "SpanQuery.h"
#include "SpanTermQuery.h"
#include "SpanNearQuery.h"
#include "TermQuery.h"
#include "FilteredQuery.h"
#include "DisjunctionMaxQuery.h"
#include "MultiTermQuery.h"
#include "MultiPhraseQuery.h"
#include "WeightedSpanTerm.h"
#include "CachingTokenFilter.h"
#include "Spans.h"
#include "FieldMaskingSpanQuery.h"
#include "SpanFirstQuery.h"
#include "SpanNotQuery.h"
#include "SpanOrQuery.h"
#include "MemoryIndex.h"
#include "MiscUtils.h"

namespace Lucene {

WeightedSpanTermExtractor::WeightedSpanTermExtractor(const String& defaultField) {
    this->defaultField = defaultField;
    this->expandMultiTermQuery = false;
    this->cachedTokenStream = false;
    this->wrapToCaching = true;
    this->readers = MapStringIndexReader::newInstance();
}

WeightedSpanTermExtractor::~WeightedSpanTermExtractor() {
}

void WeightedSpanTermExtractor::closeReaders() {
    for (MapStringIndexReader::iterator reader = readers.begin(); reader != readers.end(); ++reader) {
        try {
            reader->second->close();
        } catch (...) {
        }
    }
}

void WeightedSpanTermExtractor::extract(const QueryPtr& query, const MapWeightedSpanTermPtr& terms) {
    QueryPtr _query(query);
    if (MiscUtils::typeOf<BooleanQuery>(_query)) {
        Collection<BooleanClausePtr> queryClauses(boost::dynamic_pointer_cast<BooleanQuery>(_query)->getClauses());
        for (int32_t i = 0; i < queryClauses.size(); ++i) {
            if (!queryClauses[i]->isProhibited()) {
                extract(queryClauses[i]->getQuery(), terms);
            }
        }
    } else if (MiscUtils::typeOf<PhraseQuery>(_query)) {
        PhraseQueryPtr phraseQuery(boost::dynamic_pointer_cast<PhraseQuery>(_query));
        Collection<TermPtr> phraseQueryTerms(phraseQuery->getTerms());
        Collection<SpanQueryPtr> clauses(Collection<SpanQueryPtr>::newInstance(phraseQueryTerms.size()));
        for (int32_t i = 0; i < phraseQueryTerms.size(); ++i) {
            clauses[i] = newLucene<SpanTermQuery>(phraseQueryTerms[i]);
        }
        int32_t slop = phraseQuery->getSlop();
        Collection<int32_t> positions(phraseQuery->getPositions());
        // add largest position increment to slop
        if (!positions.empty()) {
            int32_t lastPos = positions[0];
            int32_t largestInc = 0;
            int32_t sz = positions.size();
            for (int32_t i = 1; i < sz; ++i) {
                int32_t pos = positions[i];
                int32_t inc = pos - lastPos;
                if (inc > largestInc) {
                    largestInc = inc;
                }
                lastPos = pos;
            }
            if (largestInc > 1) {
                slop += largestInc;
            }
        }

        bool inorder = (slop == 0);

        SpanNearQueryPtr sp(newLucene<SpanNearQuery>(clauses, slop, inorder));
        sp->setBoost(_query->getBoost());
        extractWeightedSpanTerms(terms, sp);
    } else if (MiscUtils::typeOf<TermQuery>(_query)) {
        extractWeightedTerms(terms, _query);
    } else if (MiscUtils::typeOf<SpanQuery>(_query)) {
        extractWeightedSpanTerms(terms, boost::dynamic_pointer_cast<SpanQuery>(_query));
    } else if (MiscUtils::typeOf<FilteredQuery>(_query)) {
        extract(boost::dynamic_pointer_cast<FilteredQuery>(_query)->getQuery(), terms);
    } else if (MiscUtils::typeOf<DisjunctionMaxQuery>(_query)) {
        DisjunctionMaxQueryPtr dmq(boost::dynamic_pointer_cast<DisjunctionMaxQuery>(_query));
        for (Collection<QueryPtr>::iterator q = dmq->begin(); q != dmq->end(); ++q) {
            extract(*q, terms);
        }
    } else if (MiscUtils::typeOf<MultiTermQuery>(_query) && expandMultiTermQuery) {
        MultiTermQueryPtr mtq(boost::dynamic_pointer_cast<MultiTermQuery>(_query));
        if (mtq->getRewriteMethod() != MultiTermQuery::SCORING_BOOLEAN_QUERY_REWRITE()) {
            mtq = boost::dynamic_pointer_cast<MultiTermQuery>(mtq->clone());
            mtq->setRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_QUERY_REWRITE());
            _query = mtq;
        }
        FakeReaderPtr fReader(newLucene<FakeReader>());
        MultiTermQuery::SCORING_BOOLEAN_QUERY_REWRITE()->rewrite(fReader, mtq);
        if (!fReader->field.empty()) {
            IndexReaderPtr ir(getReaderForField(fReader->field));
            extract(_query->rewrite(ir), terms);
        }
    } else if (MiscUtils::typeOf<MultiPhraseQuery>(_query)) {
        MultiPhraseQueryPtr mpq(boost::dynamic_pointer_cast<MultiPhraseQuery>(_query));
        Collection< Collection<TermPtr> > termArrays(mpq->getTermArrays());
        Collection<int32_t> positions(mpq->getPositions());
        if (!positions.empty()) {
            int32_t maxPosition = positions[positions.size() - 1];
            for (int32_t i = 0; i < positions.size() - 1; ++i) {
                if (positions[i] > maxPosition) {
                    maxPosition = positions[i];
                }
            }

            Collection< Collection<SpanQueryPtr> > disjunctLists(Collection< Collection<SpanQueryPtr> >::newInstance(maxPosition + 1));
            int32_t distinctPositions = 0;
            for (int32_t i = 0; i < termArrays.size(); ++i) {
                Collection<TermPtr> termArray(termArrays[i]);
                Collection<SpanQueryPtr> disjuncts(disjunctLists[positions[i]]);
                if (!disjuncts) {
                    disjunctLists[positions[i]] = Collection<SpanQueryPtr>::newInstance();
                    disjuncts = disjunctLists[positions[i]];
                    ++distinctPositions;
                }
                for (int32_t j = 0; j < termArray.size(); ++j) {
                    disjuncts.add(newLucene<SpanTermQuery>(termArray[j]));
                }
            }

            int32_t positionGaps = 0;
            int32_t position = 0;
            Collection<SpanQueryPtr> clauses(Collection<SpanQueryPtr>::newInstance(distinctPositions));
            for (int32_t i = 0; i < disjunctLists.size(); ++i) {
                Collection<SpanQueryPtr> disjuncts(disjunctLists[i]);
                if (disjuncts) {
                    clauses[position++] = newLucene<SpanOrQuery>(disjuncts);
                } else {
                    ++positionGaps;
                }
            }

            int32_t slop = mpq->getSlop();
            bool inorder = (slop == 0);

            SpanNearQueryPtr sp(newLucene<SpanNearQuery>(clauses, slop + positionGaps, inorder));
            sp->setBoost(_query->getBoost());
            extractWeightedSpanTerms(terms, sp);
        }
    }
}

void WeightedSpanTermExtractor::extractWeightedSpanTerms(const MapWeightedSpanTermPtr& terms, const SpanQueryPtr& spanQuery) {
    HashSet<String> fieldNames(HashSet<String>::newInstance());
    if (fieldName.empty()) {
        collectSpanQueryFields(spanQuery, fieldNames);
    } else {
        fieldNames.add(fieldName);
    }
    // To support the use of the default field name
    if (!defaultField.empty()) {
        fieldNames.add(defaultField);
    }

    MapStringSpanQuery queries(MapStringSpanQuery::newInstance());
    SetTerm nonWeightedTerms(SetTerm::newInstance());

    bool rewriteQuery = mustRewriteQuery(spanQuery);
    if (rewriteQuery) {
        for (HashSet<String>::iterator field = fieldNames.begin(); field != fieldNames.end(); ++field) {
            SpanQueryPtr rewrittenQuery(boost::dynamic_pointer_cast<SpanQuery>(spanQuery->rewrite(getReaderForField(*field))));
            queries.put(*field, rewrittenQuery);
            rewrittenQuery->extractTerms(nonWeightedTerms);
        }
    } else {
        spanQuery->extractTerms(nonWeightedTerms);
    }

    Collection<PositionSpanPtr> spanPositions(Collection<PositionSpanPtr>::newInstance());

    for (HashSet<String>::iterator field = fieldNames.begin(); field != fieldNames.end(); ++field) {
        IndexReaderPtr reader(getReaderForField(*field));
        SpansPtr spans;
        if (rewriteQuery) {
            spans = queries.get(*field)->getSpans(reader);
        } else {
            spans = spanQuery->getSpans(reader);
        }

        // collect span positions
        while (spans->next()) {
            spanPositions.add(newLucene<PositionSpan>(spans->start(), spans->end() - 1));
        }
    }

    if (spanPositions.empty()) {
        // no spans found
        return;
    }

    for (SetTerm::iterator queryTerm = nonWeightedTerms.begin(); queryTerm != nonWeightedTerms.end(); ++queryTerm) {
        if (fieldNameComparator((*queryTerm)->field())) {
            WeightedSpanTermPtr weightedSpanTerm(terms->get((*queryTerm)->text()));
            if (!weightedSpanTerm) {
                weightedSpanTerm = newLucene<WeightedSpanTerm>(spanQuery->getBoost(), (*queryTerm)->text());
                weightedSpanTerm->addPositionSpans(spanPositions);
                weightedSpanTerm->positionSensitive = true;
                terms->put((*queryTerm)->text(), weightedSpanTerm);
            } else {
                if (!spanPositions.empty()) {
                    weightedSpanTerm->addPositionSpans(spanPositions);
                }
            }
        }
    }
}

void WeightedSpanTermExtractor::extractWeightedTerms(const MapWeightedSpanTermPtr& terms, const QueryPtr& query) {
    SetTerm nonWeightedTerms(SetTerm::newInstance());
    query->extractTerms(nonWeightedTerms);

    for (SetTerm::iterator queryTerm = nonWeightedTerms.begin(); queryTerm != nonWeightedTerms.end(); ++queryTerm) {
        if (fieldNameComparator((*queryTerm)->field())) {
            WeightedSpanTermPtr weightedSpanTerm(newLucene<WeightedSpanTerm>(query->getBoost(), (*queryTerm)->text()));
            terms->put((*queryTerm)->text(), weightedSpanTerm);
        }
    }
}

bool WeightedSpanTermExtractor::fieldNameComparator(const String& fieldNameToCheck) {
    return (fieldName.empty() || fieldNameToCheck == fieldName || fieldNameToCheck == defaultField);
}

IndexReaderPtr WeightedSpanTermExtractor::getReaderForField(const String& field) {
    if (wrapToCaching && !cachedTokenStream && !MiscUtils::typeOf<CachingTokenFilter>(tokenStream)) {
        tokenStream = newLucene<CachingTokenFilter>(tokenStream);
        cachedTokenStream = true;
    }
    IndexReaderPtr reader(readers.get(field));
    if (!reader) {
        MemoryIndexPtr indexer(newLucene<MemoryIndex>());
        indexer->addField(field, tokenStream);
        tokenStream->reset();
        IndexSearcherPtr searcher(indexer->createSearcher());
        reader = searcher->getIndexReader();
        readers.put(field, reader);
    }
    return reader;
}

MapWeightedSpanTermPtr WeightedSpanTermExtractor::getWeightedSpanTerms(const QueryPtr& query, const TokenStreamPtr& tokenStream) {
    return getWeightedSpanTerms(query, tokenStream, L"");
}

MapWeightedSpanTermPtr WeightedSpanTermExtractor::getWeightedSpanTerms(const QueryPtr& query, const TokenStreamPtr& tokenStream, const String& fieldName) {
    if (!fieldName.empty()) {
        this->fieldName = fieldName;
    } else {
        this->fieldName.clear();
    }

    MapWeightedSpanTermPtr terms(newLucene<PositionCheckingMap>());
    this->tokenStream = tokenStream;

    LuceneException finally;
    try {
        extract(query, terms);
    } catch (LuceneException& e) {
        finally = e;
    }
    closeReaders();
    finally.throwException();
    return terms;
}

MapWeightedSpanTermPtr WeightedSpanTermExtractor::getWeightedSpanTermsWithScores(const QueryPtr& query, const TokenStreamPtr& tokenStream, const String& fieldName, const IndexReaderPtr& reader) {
    if (!fieldName.empty()) {
        this->fieldName = fieldName;
    } else {
        this->fieldName.clear();
    }

    MapWeightedSpanTermPtr terms(newLucene<PositionCheckingMap>());
    extract(query, terms);

    int32_t totalNumDocs = reader->numDocs();

    LuceneException finally;
    try {
        for (MapStringWeightedSpanTerm::iterator weightedSpanTerm = terms->begin(); weightedSpanTerm != terms->end(); ++weightedSpanTerm) {
            int32_t docFreq = reader->docFreq(newLucene<Term>(fieldName, weightedSpanTerm->second->term));
            // docFreq counts deletes
            if (totalNumDocs < docFreq) {
                docFreq = totalNumDocs;
            }
            // IDF algorithm taken from DefaultSimilarity class
            double idf = (double)(std::log((double)totalNumDocs / (double)(docFreq + 1)) + 1.0);
            weightedSpanTerm->second->weight *= idf;
        }
    } catch (LuceneException& e) {
        finally = e;
    }
    closeReaders();
    finally.throwException();
    return terms;
}

void WeightedSpanTermExtractor::collectSpanQueryFields(const SpanQueryPtr& spanQuery, HashSet<String> fieldNames) {
    if (MiscUtils::typeOf<FieldMaskingSpanQuery>(spanQuery)) {
        collectSpanQueryFields(boost::dynamic_pointer_cast<FieldMaskingSpanQuery>(spanQuery)->getMaskedQuery(), fieldNames);
    } else if (MiscUtils::typeOf<SpanFirstQuery>(spanQuery)) {
        collectSpanQueryFields(boost::dynamic_pointer_cast<SpanFirstQuery>(spanQuery)->getMatch(), fieldNames);
    } else if (MiscUtils::typeOf<SpanNearQuery>(spanQuery)) {
        Collection<SpanQueryPtr> clauses(boost::dynamic_pointer_cast<SpanNearQuery>(spanQuery)->getClauses());
        for (Collection<SpanQueryPtr>::iterator clause = clauses.begin(); clause != clauses.end(); ++clause) {
            collectSpanQueryFields(*clause, fieldNames);
        }
    } else if (MiscUtils::typeOf<SpanNotQuery>(spanQuery)) {
        collectSpanQueryFields(boost::dynamic_pointer_cast<SpanNotQuery>(spanQuery)->getInclude(), fieldNames);
    } else if (MiscUtils::typeOf<SpanOrQuery>(spanQuery)) {
        Collection<SpanQueryPtr> clauses(boost::dynamic_pointer_cast<SpanOrQuery>(spanQuery)->getClauses());
        for (Collection<SpanQueryPtr>::iterator clause = clauses.begin(); clause != clauses.end(); ++clause) {
            collectSpanQueryFields(*clause, fieldNames);
        }
    } else {
        fieldNames.add(spanQuery->getField());
    }
}

bool WeightedSpanTermExtractor::mustRewriteQuery(const SpanQueryPtr& spanQuery) {
    if (!expandMultiTermQuery) {
        return false;    // Will throw UnsupportedOperationException in case of a SpanRegexQuery.
    } else if (MiscUtils::typeOf<FieldMaskingSpanQuery>(spanQuery)) {
        return mustRewriteQuery(boost::dynamic_pointer_cast<FieldMaskingSpanQuery>(spanQuery)->getMaskedQuery());
    } else if (MiscUtils::typeOf<SpanFirstQuery>(spanQuery)) {
        return mustRewriteQuery(boost::dynamic_pointer_cast<SpanFirstQuery>(spanQuery)->getMatch());
    } else if (MiscUtils::typeOf<SpanNearQuery>(spanQuery)) {
        Collection<SpanQueryPtr> clauses(boost::dynamic_pointer_cast<SpanNearQuery>(spanQuery)->getClauses());
        for (Collection<SpanQueryPtr>::iterator clause = clauses.begin(); clause != clauses.end(); ++clause) {
            if (mustRewriteQuery(*clause)) {
                return true;
            }
        }
        return false;
    } else if (MiscUtils::typeOf<SpanNotQuery>(spanQuery)) {
        SpanNotQueryPtr spanNotQuery(boost::dynamic_pointer_cast<SpanNotQuery>(spanQuery));
        return mustRewriteQuery(spanNotQuery->getInclude()) || mustRewriteQuery(spanNotQuery->getExclude());
    } else if (MiscUtils::typeOf<SpanOrQuery>(spanQuery)) {
        Collection<SpanQueryPtr> clauses(boost::dynamic_pointer_cast<SpanOrQuery>(spanQuery)->getClauses());
        for (Collection<SpanQueryPtr>::iterator clause = clauses.begin(); clause != clauses.end(); ++clause) {
            if (mustRewriteQuery(*clause)) {
                return true;
            }
        }
        return false;
    } else if (MiscUtils::typeOf<SpanTermQuery>(spanQuery)) {
        return false;
    } else {
        return true;
    }
}

bool WeightedSpanTermExtractor::getExpandMultiTermQuery() {
    return expandMultiTermQuery;
}

void WeightedSpanTermExtractor::setExpandMultiTermQuery(bool expandMultiTermQuery) {
    this->expandMultiTermQuery = expandMultiTermQuery;
}

bool WeightedSpanTermExtractor::isCachedTokenStream() {
    return cachedTokenStream;
}

TokenStreamPtr WeightedSpanTermExtractor::getTokenStream() {
    return tokenStream;
}

void WeightedSpanTermExtractor::setWrapIfNotCachingTokenFilter(bool wrap) {
    this->wrapToCaching = wrap;
}

PositionCheckingMap::~PositionCheckingMap() {
}

void PositionCheckingMap::put(const String& key, const WeightedSpanTermPtr& val) {
    MapStringWeightedSpanTerm::iterator prev = map.find(key);
    if (prev == map.end()) {
        map.put(key, val);
        return;
    }
    bool positionSensitive = prev->second->positionSensitive;
    prev->second = val;
    if (!positionSensitive) {
        prev->second->positionSensitive = false;
    }
}

FakeReader::FakeReader() : FilterIndexReader(EMPTY_MEMORY_INDEX_READER()) {
}

FakeReader::~FakeReader() {
}

IndexReaderPtr FakeReader::EMPTY_MEMORY_INDEX_READER() {
    static IndexReaderPtr _EMPTY_MEMORY_INDEX_READER;
    if (!_EMPTY_MEMORY_INDEX_READER) {
        _EMPTY_MEMORY_INDEX_READER = newLucene<MemoryIndex>()->createSearcher()->getIndexReader();
        CycleCheck::addStatic(_EMPTY_MEMORY_INDEX_READER);
    }
    return _EMPTY_MEMORY_INDEX_READER;
}

TermEnumPtr FakeReader::terms(const TermPtr& t) {
    // only set first fieldname
    if (t && field.empty()) {
        field = t->field();
    }
    return FilterIndexReader::terms(t);
}

}
