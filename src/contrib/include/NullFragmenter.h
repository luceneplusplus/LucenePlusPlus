/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef NULLFRAGMENTER_H
#define NULLFRAGMENTER_H

#include "Fragmenter.h"

namespace Lucene {

/// {@link Fragmenter} implementation which does not fragment the text.  This is useful for
/// highlighting the entire content of a document or field.
class LPPCONTRIBAPI NullFragmenter : public Fragmenter, public LuceneObject {
public:
    virtual ~NullFragmenter();
    LUCENE_CLASS(NullFragmenter);

public:
    virtual void start(const String& originalText, const TokenStreamPtr& tokenStream);
    virtual bool isNewFragment();
};

}

#endif
