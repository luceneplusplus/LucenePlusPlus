/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SPANGRADIENTFORMATTER_H
#define SPANGRADIENTFORMATTER_H

#include "GradientFormatter.h"

namespace Lucene {

/// Formats text with different color intensity depending on the score of the term using the
/// span tag.  GradientFormatter uses a bgcolor argument to the font tag which doesn't work
/// in Mozilla, thus this class.
/// @see GradientFormatter
class LPPCONTRIBAPI SpanGradientFormatter : public GradientFormatter {
public:
    SpanGradientFormatter(double maxScore, const String& minForegroundColor, const String& maxForegroundColor, const String& minBackgroundColor, const String& maxBackgroundColor);
    virtual ~SpanGradientFormatter();

    LUCENE_CLASS(SpanGradientFormatter);

public:
    virtual String highlightTerm(const String& originalText, const TokenGroupPtr& tokenGroup);
};

}

#endif
