/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "SpanGradientFormatter.h"
#include "TokenGroup.h"

namespace Lucene {

SpanGradientFormatter::SpanGradientFormatter(double maxScore, const String& minForegroundColor, const String& maxForegroundColor, const String& minBackgroundColor, const String& maxBackgroundColor) :
    GradientFormatter(maxScore, minForegroundColor, maxForegroundColor, minBackgroundColor, maxBackgroundColor) {
}

SpanGradientFormatter::~SpanGradientFormatter() {
}

String SpanGradientFormatter::highlightTerm(const String& originalText, const TokenGroupPtr& tokenGroup) {
    if (tokenGroup->getTotalScore() == 0) {
        return originalText;
    }
    double score = tokenGroup->getTotalScore();
    if (score == 0.0) {
        return originalText;
    }
    StringStream buffer;
    buffer << L"<span style=\"";
    if (highlightForeground) {
        buffer << L"color: " << getForegroundColorString(score) << L"; ";
    }
    if (highlightBackground) {
        buffer << L"background: " << getBackgroundColorString(score) << L"; ";
    }
    buffer << L"\">" << originalText << L"</span>";
    return buffer.str();
}

}
