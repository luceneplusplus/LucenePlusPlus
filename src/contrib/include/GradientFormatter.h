/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef GRADIENTFORMATTER_H
#define GRADIENTFORMATTER_H

#include "Formatter.h"

namespace Lucene {

/// Formats text with different color intensity depending on the score of the term.
class LPPCONTRIBAPI GradientFormatter : public Formatter, public LuceneObject {
public:
    GradientFormatter(double maxScore, const String& minForegroundColor, const String& maxForegroundColor, const String& minBackgroundColor, const String& maxBackgroundColor);
    virtual ~GradientFormatter();

    LUCENE_CLASS(GradientFormatter);

protected:
    double maxScore;
    bool highlightForeground;
    bool highlightBackground;

public:
    int32_t fgRMin;
    int32_t fgGMin;
    int32_t fgBMin;

    int32_t fgRMax;
    int32_t fgGMax;
    int32_t fgBMax;

    int32_t bgRMin;
    int32_t bgGMin;
    int32_t bgBMin;

    int32_t bgRMax;
    int32_t bgGMax;
    int32_t bgBMax;

public:
    virtual String highlightTerm(const String& originalText, const TokenGroupPtr& tokenGroup);

protected:
    String getForegroundColorString(double score);
    String getBackgroundColorString(double score);
    int32_t getColorVal(int32_t colorMin, int32_t colorMax, double score);

    static String intToHex(int32_t i);

    /// Converts a hex string into an int.
    static int32_t hexToInt(const String& hex);
};

}

#endif
