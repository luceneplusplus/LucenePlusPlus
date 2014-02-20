/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef FORMATTER_H
#define FORMATTER_H

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene {

/// Processes terms found in the original text, typically by applying some form of mark-up to highlight
/// terms in HTML search results pages.
class LPPCONTRIBAPI Formatter {
public:
    virtual ~Formatter();
    LUCENE_INTERFACE(Formatter);

public:
    /// @param originalText The section of text being considered for markup
    /// @param tokenGroup contains one or several overlapping Tokens along with their scores and positions.
    virtual String highlightTerm(const String& originalText, const TokenGroupPtr& tokenGroup);
};

}

#endif
