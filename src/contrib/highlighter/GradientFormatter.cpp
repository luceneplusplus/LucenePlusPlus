/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "GradientFormatter.h"
#include "TokenGroup.h"
#include "StringUtils.h"

namespace Lucene {

GradientFormatter::GradientFormatter(double maxScore, const String& minForegroundColor, const String& maxForegroundColor, const String& minBackgroundColor, const String& maxBackgroundColor) {
    highlightForeground = (!minForegroundColor.empty()  && !maxForegroundColor.empty());
    if (highlightForeground) {
        if (minForegroundColor.length() != 7) {
            boost::throw_exception(IllegalArgumentException(L"minForegroundColor is not 7 bytes long eg a hex RGB value such as #FFFFFF"));
        }
        if (maxForegroundColor.length() != 7) {
            boost::throw_exception(IllegalArgumentException(L"maxForegroundColor is not 7 bytes long eg a hex RGB value such as #FFFFFF"));
        }

        fgRMin = hexToInt(minForegroundColor.substr(1, 2));
        fgGMin = hexToInt(minForegroundColor.substr(3, 2));
        fgBMin = hexToInt(minForegroundColor.substr(5, 2));

        fgRMax = hexToInt(maxForegroundColor.substr(1, 2));
        fgGMax = hexToInt(maxForegroundColor.substr(3, 2));
        fgBMax = hexToInt(maxForegroundColor.substr(5, 2));
    }

    highlightBackground = (!minBackgroundColor.empty()  && !maxBackgroundColor.empty());
    if (highlightBackground) {
        if (minBackgroundColor.length() != 7) {
            boost::throw_exception(IllegalArgumentException(L"minBackgroundColor is not 7 bytes long eg a hex RGB value such as #FFFFFF"));
        }
        if (maxBackgroundColor.length() != 7) {
            boost::throw_exception(IllegalArgumentException(L"maxBackgroundColor is not 7 bytes long eg a hex RGB value such as #FFFFFF"));
        }

        bgRMin = hexToInt(minBackgroundColor.substr(1, 2));
        bgGMin = hexToInt(minBackgroundColor.substr(3, 2));
        bgBMin = hexToInt(minBackgroundColor.substr(5, 2));

        bgRMax = hexToInt(maxBackgroundColor.substr(1, 2));
        bgGMax = hexToInt(maxBackgroundColor.substr(3, 2));
        bgBMax = hexToInt(maxBackgroundColor.substr(5, 2));
    }

    this->maxScore = maxScore;
}

GradientFormatter::~GradientFormatter() {
}

String GradientFormatter::highlightTerm(const String& originalText, const TokenGroupPtr& tokenGroup) {
    if (tokenGroup->getTotalScore() == 0) {
        return originalText;
    }
    double score = tokenGroup->getTotalScore();
    if (score == 0.0) {
        return originalText;
    }
    StringStream buffer;
    buffer << L"<font ";
    if (highlightForeground) {
        buffer << L"color=\"" << getForegroundColorString(score) << L"\" ";
    }
    if (highlightBackground) {
        buffer << L"bgcolor=\"" << getBackgroundColorString(score) << L"\" ";
    }
    buffer << L">" << originalText << L"</font>";
    return buffer.str();
}

String GradientFormatter::getForegroundColorString(double score) {
    int32_t rVal = getColorVal(fgRMin, fgRMax, score);
    int32_t gVal = getColorVal(fgGMin, fgGMax, score);
    int32_t bVal = getColorVal(fgBMin, fgBMax, score);
    StringStream buffer;
    buffer << L"#" << intToHex(rVal) << intToHex(gVal) << intToHex(bVal);
    return buffer.str();
}

String GradientFormatter::getBackgroundColorString(double score) {
    int32_t rVal = getColorVal(bgRMin, bgRMax, score);
    int32_t gVal = getColorVal(bgGMin, bgGMax, score);
    int32_t bVal = getColorVal(bgBMin, bgBMax, score);
    StringStream buffer;
    buffer << L"#" << intToHex(rVal) << intToHex(gVal) << intToHex(bVal);
    return buffer.str();
}

int32_t GradientFormatter::getColorVal(int32_t colorMin, int32_t colorMax, double score) {
    if (colorMin == colorMax) {
        return colorMin;
    }
    double scale = std::abs((double)(colorMin - colorMax));
    double relScorePercent = std::min(maxScore, score) / maxScore;
    double colScore = scale * relScorePercent;
    return std::min(colorMin, colorMax) + (int32_t)colScore;
}

String GradientFormatter::intToHex(int32_t i) {
    static const wchar_t* hexDigits = L"0123456789abcdef";
    StringStream buffer;
    buffer << hexDigits[(i & 0xf0) >> 4] << hexDigits[i & 0x0f];
    return buffer.str();
}

int32_t GradientFormatter::hexToInt(const String& hex) {
    int32_t len = (int32_t)hex.length();
    if (len > 16) {
        boost::throw_exception(NumberFormatException());
    }
    int32_t l = 0;
    for (int32_t i = 0; i < len; ++i) {
        l <<= 4;
        int32_t c = (int32_t)StringUtils::toLong(hex.substr(i, 1), 16);
        if (c < 0) {
            boost::throw_exception(NumberFormatException());
        }
        l |= c;
    }
    return l;
}

}
