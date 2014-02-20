/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef ENCODER_H
#define ENCODER_H

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene {

/// Encodes original text. The Encoder works with the {@link Formatter} to generate output.
class LPPCONTRIBAPI Encoder {
public:
    virtual ~Encoder();
    LUCENE_INTERFACE(Encoder);

public:
    virtual String encodeText(const String& originalText);
};

}

#endif
