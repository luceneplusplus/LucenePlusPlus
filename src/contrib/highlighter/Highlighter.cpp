/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "Highlighter.h"
#include "HighlighterScorer.h"
#include "SimpleHTMLFormatter.h"
#include "DefaultEncoder.h"
#include "Scorer.h"
#include "TokenStream.h"
#include "StringReader.h"
#include "Analyzer.h"
#include "TermAttribute.h"
#include "OffsetAttribute.h"
#include "PositionIncrementAttribute.h"
#include "TextFragment.h"
#include "TokenGroup.h"
#include "SimpleFragmenter.h"
#include "StringUtils.h"

namespace Lucene {

const int32_t Highlighter::DEFAULT_MAX_CHARS_TO_ANALYZE = 50 * 1024;

Highlighter::Highlighter(const HighlighterScorerPtr& fragmentScorer) {
    this->formatter = newLucene<SimpleHTMLFormatter>();
    this->encoder = newLucene<DefaultEncoder>();
    this->fragmentScorer = fragmentScorer;
    this->maxDocCharsToAnalyze = DEFAULT_MAX_CHARS_TO_ANALYZE;
    this->textFragmenter = newLucene<SimpleFragmenter>();
}

Highlighter::Highlighter(const FormatterPtr& formatter, const HighlighterScorerPtr& fragmentScorer) {
    this->formatter = formatter;
    this->encoder = newLucene<DefaultEncoder>();
    this->fragmentScorer = fragmentScorer;
    this->maxDocCharsToAnalyze = DEFAULT_MAX_CHARS_TO_ANALYZE;
    this->textFragmenter = newLucene<SimpleFragmenter>();
}

Highlighter::Highlighter(const FormatterPtr& formatter, const EncoderPtr& encoder, const HighlighterScorerPtr& fragmentScorer) {
    this->formatter = formatter;
    this->encoder = encoder;
    this->fragmentScorer = fragmentScorer;
    this->maxDocCharsToAnalyze = DEFAULT_MAX_CHARS_TO_ANALYZE;
    this->textFragmenter = newLucene<SimpleFragmenter>();
}

Highlighter::~Highlighter() {
}

String Highlighter::getBestFragment(const AnalyzerPtr& analyzer, const String& fieldName, const String& text) {
    TokenStreamPtr tokenStream(analyzer->tokenStream(fieldName, newLucene<StringReader>(text)));
    return getBestFragment(tokenStream, text);
}

String Highlighter::getBestFragment(const TokenStreamPtr& tokenStream, const String& text) {
    Collection<String> results(getBestFragments(tokenStream,text, 1));
    return results.empty() ? L"" : results[0];
}

Collection<String> Highlighter::getBestFragments(const AnalyzerPtr& analyzer, const String& fieldName, const String& text, int32_t maxNumFragments) {
    TokenStreamPtr tokenStream(analyzer->tokenStream(fieldName, newLucene<StringReader>(text)));
    return getBestFragments(tokenStream, text, maxNumFragments);
}

Collection<String> Highlighter::getBestFragments(const TokenStreamPtr& tokenStream, const String& text, int32_t maxNumFragments) {
    maxNumFragments = std::max((int32_t)1, maxNumFragments); //sanity check

    Collection<TextFragmentPtr> frag(getBestTextFragments(tokenStream, text, true, maxNumFragments));

    // Get text
    Collection<String> fragTexts(Collection<String>::newInstance());
    for (int32_t i = 0; i < frag.size(); ++i) {
        if (frag[i] && frag[i]->getScore() > 0) {
            fragTexts.add(frag[i]->toString());
        }
    }
    return fragTexts;
}

Collection<TextFragmentPtr> Highlighter::getBestTextFragments(const TokenStreamPtr& tokenStream, const String& text, bool merge, int32_t maxNumFragments) {
    Collection<TextFragmentPtr> docFrags(Collection<TextFragmentPtr>::newInstance());
    StringBufferPtr newText(newLucene<StringBuffer>());

    TokenStreamPtr _tokenStream(tokenStream);
    TermAttributePtr termAtt(_tokenStream->addAttribute<TermAttribute>());
    OffsetAttributePtr offsetAtt(_tokenStream->addAttribute<OffsetAttribute>());
    _tokenStream->addAttribute<PositionIncrementAttribute>();
    _tokenStream->reset();

    TextFragmentPtr currentFrag(newLucene<TextFragment>(newText, newText->length(), docFrags.size()));
    TokenStreamPtr newStream(fragmentScorer->init(_tokenStream));
    if (newStream) {
        _tokenStream = newStream;
    }
    fragmentScorer->startFragment(currentFrag);
    docFrags.add(currentFrag);

    FragmentQueuePtr fragQueue(newLucene<FragmentQueue>(maxNumFragments));
    Collection<TextFragmentPtr> frag;

    LuceneException finally;
    try {
        textFragmenter->start(text, _tokenStream);
        TokenGroupPtr tokenGroup(newLucene<TokenGroup>(_tokenStream));
        String tokenText;
        int32_t startOffset = 0;
        int32_t endOffset = 0;
        int32_t lastEndOffset = 0;

        for (bool next = _tokenStream->incrementToken(); next && offsetAtt->startOffset() < maxDocCharsToAnalyze; next = _tokenStream->incrementToken()) {
            if (offsetAtt->endOffset() > (int32_t)text.length() || offsetAtt->startOffset() > (int32_t)text.length()) {
                boost::throw_exception(RuntimeException(L"InvalidTokenOffsets: Token " + termAtt->term() + L" exceeds length of provided text sized " + StringUtils::toString(text.length())));
            }

            if (tokenGroup->numTokens > 0 && tokenGroup->isDistinct()) {
                // the current token is distinct from previous tokens - markup the cached token group info
                startOffset = tokenGroup->matchStartOffset;
                endOffset = tokenGroup->matchEndOffset;
                tokenText = text.substr(startOffset, endOffset - startOffset);
                String markedUpText(formatter->highlightTerm(encoder->encodeText(tokenText), tokenGroup));
                // store any whitespace etc from between this and last group
                if (startOffset > lastEndOffset) {
                    newText->append(encoder->encodeText(text.substr(lastEndOffset, startOffset - lastEndOffset)));
                }
                newText->append(markedUpText);
                lastEndOffset = std::max(endOffset, lastEndOffset);
                tokenGroup->clear();

                // check if current token marks the start of a new fragment
                if (textFragmenter->isNewFragment()) {
                    currentFrag->setScore(fragmentScorer->getFragmentScore());
                    // record stats for a new fragment
                    currentFrag->textEndPos = newText->length();
                    currentFrag = newLucene<TextFragment>(newText, newText->length(), docFrags.size());
                    fragmentScorer->startFragment(currentFrag);
                    docFrags.add(currentFrag);
                }
            }

            tokenGroup->addToken(fragmentScorer->getTokenScore());
        }

        currentFrag->setScore(fragmentScorer->getFragmentScore());

        if (tokenGroup->numTokens > 0) {
            // flush the accumulated text (same code as in above loop)
            startOffset = tokenGroup->matchStartOffset;
            endOffset = tokenGroup->matchEndOffset;
            tokenText = text.substr(startOffset, endOffset - startOffset);
            String markedUpText(formatter->highlightTerm(encoder->encodeText(tokenText), tokenGroup));
            // store any whitespace etc from between this and last group
            if (startOffset > lastEndOffset) {
                newText->append(encoder->encodeText(text.substr(lastEndOffset, startOffset - lastEndOffset)));
            }
            newText->append(markedUpText);
            lastEndOffset = std::max(lastEndOffset, endOffset);
        }

        // Test what remains of the original text beyond the point where we stopped analyzing
        if (lastEndOffset < (int32_t)text.length() && (int32_t)text.length() <= maxDocCharsToAnalyze) {
            // append it to the last fragment
            newText->append(encoder->encodeText(text.substr(lastEndOffset)));
        }

        currentFrag->textEndPos = newText->length();

        // sort the most relevant sections of the text
        for (Collection<TextFragmentPtr>::iterator i = docFrags.begin(); i != docFrags.end(); ++i) {
            fragQueue->addOverflow(*i);
        }

        // return the most relevant fragments
        frag = Collection<TextFragmentPtr>::newInstance(fragQueue->size());
        for (int32_t i = frag.size() - 1; i >= 0; --i) {
            frag[i] = fragQueue->pop();
        }

        // merge any contiguous fragments to improve readability
        if (merge) {
            mergeContiguousFragments(frag);
            Collection<TextFragmentPtr> fragTexts(Collection<TextFragmentPtr>::newInstance());
            for (int32_t i = 0; i < frag.size(); ++i) {
                if (frag[i] && frag[i]->getScore() > 0) {
                    fragTexts.add(frag[i]);
                }
            }
            frag = fragTexts;
        }
    } catch (LuceneException& e) {
        finally = e;
    }
    if (_tokenStream) {
        try {
            _tokenStream->close();
        } catch (...) {
        }
    }
    finally.throwException();
    return frag;
}

void Highlighter::mergeContiguousFragments(Collection<TextFragmentPtr> frag) {
    if (frag.size() > 1) {
        bool mergingStillBeingDone = false;
        do {
            mergingStillBeingDone = false; // initialise loop control flag
            // for each fragment, scan other frags looking for contiguous blocks
            for (int32_t i = 0; i < frag.size(); ++i) {
                if (!frag[i]) {
                    continue;
                }
                // merge any contiguous blocks
                for (int32_t x = 0; x < frag.size(); ++x) {
                    if (!frag[x]) {
                        continue;
                    }
                    if (!frag[i]) {
                        break;
                    }
                    TextFragmentPtr frag1;
                    TextFragmentPtr frag2;
                    int32_t frag1Num = 0;
                    int32_t frag2Num = 0;
                    int32_t bestScoringFragNum = 0;
                    int32_t worstScoringFragNum = 0;
                    // if blocks are contiguous
                    if (frag[i]->follows(frag[x])) {
                        frag1 = frag[x];
                        frag1Num = x;
                        frag2 = frag[i];
                        frag2Num = i;
                    } else if (frag[x]->follows(frag[i])) {
                        frag1 = frag[i];
                        frag1Num = i;
                        frag2 = frag[x];
                        frag2Num = x;
                    }

                    // merging required
                    if (frag1) {
                        if (frag1->getScore() > frag2->getScore()) {
                            bestScoringFragNum = frag1Num;
                            worstScoringFragNum = frag2Num;
                        } else {
                            bestScoringFragNum = frag2Num;
                            worstScoringFragNum = frag1Num;
                        }
                        frag1->merge(frag2);
                        frag[worstScoringFragNum].reset();
                        mergingStillBeingDone = true;
                        frag[bestScoringFragNum] = frag1;
                    }
                }
            }
        } while (mergingStillBeingDone);
    }
}

String Highlighter::getBestFragments(const TokenStreamPtr& tokenStream, const String& text, int32_t maxNumFragments, const String& separator) {
    Collection<String> sections(getBestFragments(tokenStream, text, maxNumFragments));
    StringStream result;
    for (int32_t i = 0; i < sections.size(); ++i) {
        if (i > 0) {
            result << separator;
        }
        result << sections[i];
    }
    return result.str();
}

int32_t Highlighter::getMaxDocCharsToAnalyze() {
    return maxDocCharsToAnalyze;
}

void Highlighter::setMaxDocCharsToAnalyze(int32_t maxDocCharsToAnalyze) {
    this->maxDocCharsToAnalyze = maxDocCharsToAnalyze;
}

FragmenterPtr Highlighter::getTextFragmenter() {
    return textFragmenter;
}

void Highlighter::setTextFragmenter(const FragmenterPtr& fragmenter) {
    textFragmenter = fragmenter;
}

HighlighterScorerPtr Highlighter::getFragmentScorer() {
    return fragmentScorer;
}

void Highlighter::setFragmentScorer(const HighlighterScorerPtr& scorer) {
    fragmentScorer = scorer;
}

EncoderPtr Highlighter::getEncoder() {
    return encoder;
}

void Highlighter::setEncoder(const EncoderPtr& encoder) {
    this->encoder = encoder;
}

FragmentQueue::FragmentQueue(int32_t size) : PriorityQueue<TextFragmentPtr>(size) {
}

FragmentQueue::~FragmentQueue() {
}

bool FragmentQueue::lessThan(const TextFragmentPtr& first, const TextFragmentPtr& second) {
    if (first->getScore() == second->getScore()) {
        return first->fragNum > second->fragNum;
    } else {
        return first->getScore() < second->getScore();
    }
}

}
