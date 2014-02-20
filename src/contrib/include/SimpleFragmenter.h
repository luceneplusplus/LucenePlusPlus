/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SIMPLEFRAGMENTER_H
#define SIMPLEFRAGMENTER_H

#include "Fragmenter.h"

namespace Lucene {

/// {@link Fragmenter} implementation which breaks text up into same-size fragments with
/// no concerns over spotting sentence boundaries.
class LPPCONTRIBAPI SimpleFragmenter : public Fragmenter, public LuceneObject {
public:
    SimpleFragmenter();
    SimpleFragmenter(int32_t fragmentSize);

    virtual ~SimpleFragmenter();

    LUCENE_CLASS(SimpleFragmenter);

protected:
    static const int32_t DEFAULT_FRAGMENT_SIZE;
    int32_t currentNumFrags;
    int32_t fragmentSize;
    OffsetAttributePtr offsetAtt;

public:
    virtual void start(const String& originalText, const TokenStreamPtr& tokenStream);
    virtual bool isNewFragment();

    /// @return size in number of characters of each fragment
    int32_t getFragmentSize();

    /// @param size size in characters of each fragment
    void setFragmentSize(int32_t size);
};

}

#endif
