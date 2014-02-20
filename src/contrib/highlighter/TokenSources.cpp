/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "TokenSources.h"
#include "IndexReader.h"
#include "Document.h"
#include "Analyzer.h"
#include "TokenStream.h"
#include "TermFreqVector.h"
#include "TermPositionVector.h"
#include "TermAttribute.h"
#include "OffsetAttribute.h"
#include "TermVectorOffsetInfo.h"
#include "Token.h"
#include "StringReader.h"
#include "StringUtils.h"

namespace Lucene {

TokenSources::~TokenSources() {
}

TokenStreamPtr TokenSources::getAnyTokenStream(const IndexReaderPtr& reader, int32_t docId, const String& field, const DocumentPtr& doc, const AnalyzerPtr& analyzer) {
    TokenStreamPtr ts;
    TermFreqVectorPtr tfv(reader->getTermFreqVector(docId, field));
    if (tfv) {
        if (boost::dynamic_pointer_cast<TermPositionVector>(tfv)) {
            ts = getTokenStream(boost::dynamic_pointer_cast<TermPositionVector>(tfv));
        }
    }
    // No token info stored so fall back to analyzing raw content
    if (!ts) {
        ts = getTokenStream(doc, field, analyzer);
    }
    return ts;
}

TokenStreamPtr TokenSources::getAnyTokenStream(const IndexReaderPtr& reader, int32_t docId, const String& field, const AnalyzerPtr& analyzer) {
    TokenStreamPtr ts;
    TermFreqVectorPtr tfv(reader->getTermFreqVector(docId, field));
    if (tfv) {
        if (boost::dynamic_pointer_cast<TermPositionVector>(tfv)) {
            ts = getTokenStream(boost::dynamic_pointer_cast<TermPositionVector>(tfv));
        }
    }
    // No token info stored so fall back to analyzing raw content
    if (!ts) {
        ts = getTokenStream(reader, docId, field, analyzer);
    }
    return ts;
}

TokenStreamPtr TokenSources::getTokenStream(const TermPositionVectorPtr& tpv) {
    // assumes the worst and makes no assumptions about token position sequences.
    return getTokenStream(tpv, false);
}

struct lessTokenOffset {
    inline bool operator()(const TokenPtr& first, const TokenPtr& second) const {
        if (first->startOffset() < second->startOffset()) {
            return true;
        }
        return (first->startOffset() > second->endOffset());
    }
};

TokenStreamPtr TokenSources::getTokenStream(const TermPositionVectorPtr& tpv, bool tokenPositionsGuaranteedContiguous) {
    // code to reconstruct the original sequence of Tokens
    Collection<String> terms(tpv->getTerms());
    Collection<int32_t> freq(tpv->getTermFrequencies());
    int32_t totalTokens = 0;

    for (int32_t t = 0; t < freq.size(); ++t) {
        totalTokens += freq[t];
    }

    Collection<TokenPtr> tokensInOriginalOrder(Collection<TokenPtr>::newInstance(totalTokens));
    Collection<TokenPtr> unsortedTokens;
    for (int32_t t = 0; t < freq.size(); ++t) {
        Collection<TermVectorOffsetInfoPtr> offsets(tpv->getOffsets(t));
        if (!offsets) {
            return TokenStreamPtr();
        }
        Collection<int32_t> pos;
        if (tokenPositionsGuaranteedContiguous) {
            // try get the token position info to speed up assembly of tokens into sorted sequence
            pos = tpv->getTermPositions(t);
        }
        if (!pos) {
            // tokens NOT stored with positions or not guaranteed contiguous - must add to list and sort later
            if (!unsortedTokens) {
                unsortedTokens = Collection<TokenPtr>::newInstance();
            }
            for (int32_t tp = 0; tp < offsets.size(); ++tp) {
                TokenPtr token(newLucene<Token>(offsets[tp]->getStartOffset(), offsets[tp]->getEndOffset()));
                token->setTermBuffer(terms[t]);
                unsortedTokens.add(token);
            }
        } else {
            // We have positions stored and a guarantee that the token position information is contiguous

            // This may be fast BUT wont work if Tokenizers used which create >1 token in same position or
            // creates jumps in position numbers - this code would fail under those circumstances

            // Tokens stored with positions - can use this to index straight into sorted array
            for (int32_t tp = 0; tp < pos.size(); ++tp) {
                TokenPtr token(newLucene<Token>(terms[t], offsets[tp]->getStartOffset(), offsets[tp]->getEndOffset()));
                tokensInOriginalOrder[pos[tp]] = token;
            }
        }
    }
    // If the field has been stored without position data we must perform a sort
    if (unsortedTokens) {
        tokensInOriginalOrder = unsortedTokens;
        std::sort(tokensInOriginalOrder.begin(), tokensInOriginalOrder.end(), lessTokenOffset());
    }
    return newLucene<StoredTokenStream>(tokensInOriginalOrder);
}

TokenStreamPtr TokenSources::getTokenStream(const IndexReaderPtr& reader, int32_t docId, const String& field) {
    TermFreqVectorPtr tfv(reader->getTermFreqVector(docId, field));
    if (!tfv) {
        boost::throw_exception(IllegalArgumentException(field + L" in doc #" + StringUtils::toString(docId) + L"does not have any term position data stored"));
    }

    if (boost::dynamic_pointer_cast<TermPositionVector>(tfv)) {
        TermPositionVectorPtr tpv(boost::dynamic_pointer_cast<TermPositionVector>(reader->getTermFreqVector(docId, field)));
        return getTokenStream(tpv);
    }
    boost::throw_exception(IllegalArgumentException(field + L" in doc #" + StringUtils::toString(docId) + L"does not have any term position data stored"));
    return TokenStreamPtr();
}

TokenStreamPtr TokenSources::getTokenStream(const IndexReaderPtr& reader, int32_t docId, const String& field, const AnalyzerPtr& analyzer) {
    DocumentPtr doc(reader->document(docId));
    return getTokenStream(doc, field, analyzer);
}

TokenStreamPtr TokenSources::getTokenStream(const DocumentPtr& doc, const String& field, const AnalyzerPtr& analyzer) {
    String contents(doc->get(field));
    if (contents.empty()) {
        boost::throw_exception(IllegalArgumentException(L"Field " + field + L" in document is not stored and cannot be analyzed"));
    }
    return getTokenStream(field, contents, analyzer);
}

TokenStreamPtr TokenSources::getTokenStream(const String& field, const String& contents, const AnalyzerPtr& analyzer) {
    return analyzer->tokenStream(field, newLucene<StringReader>(contents));
}

StoredTokenStream::StoredTokenStream(Collection<TokenPtr> tokens) {
    this->tokens = tokens;
    this->termAtt = addAttribute<TermAttribute>();
    this->offsetAtt = addAttribute<OffsetAttribute>();
}

StoredTokenStream::~StoredTokenStream() {
}

bool StoredTokenStream::incrementToken() {
    if (currentToken >= tokens.size()) {
        return false;
    }
    clearAttributes();
    TokenPtr token(tokens[currentToken++]);
    termAtt->setTermBuffer(token->term());
    offsetAtt->setOffset(token->startOffset(), token->endOffset());
    return true;
}

}
