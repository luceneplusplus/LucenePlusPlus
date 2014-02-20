/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SIMPLEHTMLENCODER_H
#define SIMPLEHTMLENCODER_H

#include "Encoder.h"

namespace Lucene {

/// Simple {@link Encoder} implementation to escape text for HTML output.
class LPPCONTRIBAPI SimpleHTMLEncoder : public Encoder, public LuceneObject {
public:
    virtual ~SimpleHTMLEncoder();
    LUCENE_CLASS(SimpleHTMLEncoder);

public:
    virtual String encodeText(const String& originalText);

    /// Encode string into HTML
    static String htmlEncode(const String& plainText);
};

}

#endif
