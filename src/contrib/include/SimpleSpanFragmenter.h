/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SIMPLESPANFRAGMENTER_H
#define SIMPLESPANFRAGMENTER_H

#include "Fragmenter.h"

namespace Lucene {

/// {@link Fragmenter} implementation which breaks text up into same-size fragments but
/// does not split up {@link Spans}.  This is a simple sample class.
class LPPCONTRIBAPI SimpleSpanFragmenter : public Fragmenter, public LuceneObject {
public:
    /// @param queryScorer QueryScorer that was used to score hits
    SimpleSpanFragmenter(const QueryScorerPtr& queryScorer);

    /// @param queryScorer QueryScorer that was used to score hits
    /// @param fragmentSize size in bytes of each fragment
    SimpleSpanFragmenter(const QueryScorerPtr& queryScorer, int32_t fragmentSize);

    virtual ~SimpleSpanFragmenter();

    LUCENE_CLASS(SimpleSpanFragmenter);

protected:
    static const int32_t DEFAULT_FRAGMENT_SIZE;

    int32_t fragmentSize;
    int32_t currentNumFrags;
    int32_t position;
    QueryScorerPtr queryScorer;
    int32_t waitForPos;
    int32_t textSize;
    TermAttributePtr termAtt;
    PositionIncrementAttributePtr posIncAtt;
    OffsetAttributePtr offsetAtt;

public:
    virtual bool isNewFragment();
    virtual void start(const String& originalText, const TokenStreamPtr& tokenStream);
};

}

#endif
