/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "SimpleHTMLEncoder.h"

namespace Lucene {

SimpleHTMLEncoder::~SimpleHTMLEncoder() {
}

String SimpleHTMLEncoder::encodeText(const String& originalText) {
    return htmlEncode(originalText);
}

String SimpleHTMLEncoder::htmlEncode(const String& plainText) {
    if (plainText.empty()) {
        return L"";
    }

    StringStream result;

    for (int32_t index = 0; index < (int32_t)plainText.length(); ++index) {
        wchar_t ch = plainText[index];

        switch (ch) {
        case L'\"':
            result << L"&quot;";
            break;
        case L'&':
            result << L"&amp;";
            break;
        case L'<':
            result << L"&lt;";
            break;
        case L'>':
            result << L"&gt;";
            break;
        default:
            if (ch < 128) {
                result << ch;
            } else {
                result << L"&#" << (int32_t)ch << L";";
            }
            break;
        }
    }

    return result.str();
}

}
