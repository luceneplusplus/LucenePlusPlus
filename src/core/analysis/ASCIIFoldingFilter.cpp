/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "ASCIIFoldingFilter.h"
#include "CharTermAttribute.h"
#include "MiscUtils.h"

namespace Lucene
{
    ASCIIFoldingFilter::ASCIIFoldingFilter(TokenStreamPtr input) : TokenFilter(input)
    {
        output = CharArray::newInstance(512);
        outputPos = 0;
        termAtt = addAttribute<CharTermAttribute>();
    }
    
    ASCIIFoldingFilter::~ASCIIFoldingFilter()
    {
    }
    
    bool ASCIIFoldingFilter::incrementToken()
    {
        if (input->incrementToken())
        {
            wchar_t* buffer = termAtt->bufferArray();
            int32_t length = termAtt->length();
            
            // If no characters actually require rewriting then we just return token as-is
            for (int32_t i = 0; i < length; ++i)
            {
                wchar_t c = buffer[i];
                if (c >= 0x0080)
                {
                    foldToASCII(buffer, length);
                    termAtt->copyBuffer(output.get(), 0, outputPos);
                    break;
                }
            }
            return true;
        }
        else
            return false;
    }
    
    void ASCIIFoldingFilter::foldToASCII(const wchar_t* input, int32_t length)
    {
        // Worst-case length required
        int32_t maxSizeNeeded = 4 * length;
        if (output.size() < maxSizeNeeded)
            MiscUtils::grow(output, maxSizeNeeded);
        
        outputPos = foldToASCII(input, 0, this->output.get(), 0, length);
    }
    
    int32_t ASCIIFoldingFilter::foldToASCII(const wchar_t* input, int32_t inputPos, wchar_t* output, int32_t outputPos, int32_t length)
    {
        int32_t end = inputPos + length;
        
        for (int32_t pos = inputPos; pos < end; ++pos)
        {
            wchar_t c = input[pos];
            
            // Quick test: if it's not in range then just keep current character
            if (c < 0x0080)
                output[outputPos++] = c;
            else
            {
                switch (c)
                {
                    case 0x00c0: // [LATIN CAPITAL LETTER A WITH GRAVE]
                    case 0x00c1: // [LATIN CAPITAL LETTER A WITH ACUTE]
                    case 0x00c2: // [LATIN CAPITAL LETTER A WITH CIRCUMFLEX]
                    case 0x00c3: // [LATIN CAPITAL LETTER A WITH TILDE]
                    case 0x00c4: // [LATIN CAPITAL LETTER A WITH DIAERESIS]
                    case 0x00c5: // [LATIN CAPITAL LETTER A WITH RING ABOVE]
                    case 0x0100: // [LATIN CAPITAL LETTER A WITH MACRON]
                    case 0x0102: // [LATIN CAPITAL LETTER A WITH BREVE]
                    case 0x0104: // [LATIN CAPITAL LETTER A WITH OGONEK]
                    case 0x018f: // [LATIN CAPITAL LETTER SCHWA]
                    case 0x01cd: // [LATIN CAPITAL LETTER A WITH CARON]
                    case 0x01de: // [LATIN CAPITAL LETTER A WITH DIAERESIS AND MACRON]
                    case 0x01e0: // [LATIN CAPITAL LETTER A WITH DOT ABOVE AND MACRON]
                    case 0x01fa: // [LATIN CAPITAL LETTER A WITH RING ABOVE AND ACUTE]
                    case 0x0200: // [LATIN CAPITAL LETTER A WITH DOUBLE GRAVE]
                    case 0x0202: // [LATIN CAPITAL LETTER A WITH INVERTED BREVE]
                    case 0x0226: // [LATIN CAPITAL LETTER A WITH DOT ABOVE]
                    case 0x023a: // [LATIN CAPITAL LETTER A WITH STROKE]
                    case 0x1d00: // [LATIN LETTER SMALL CAPITAL A]
                    case 0x1e00: // [LATIN CAPITAL LETTER A WITH RING BELOW]
                    case 0x1ea0: // [LATIN CAPITAL LETTER A WITH DOT BELOW]
                    case 0x1ea2: // [LATIN CAPITAL LETTER A WITH HOOK ABOVE]
                    case 0x1ea4: // [LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND ACUTE]
                    case 0x1ea6: // [LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND GRAVE]
                    case 0x1ea8: // [LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND HOOK ABOVE]
                    case 0x1eaa: // [LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND TILDE]
                    case 0x1eac: // [LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND DOT BELOW]
                    case 0x1eae: // [LATIN CAPITAL LETTER A WITH BREVE AND ACUTE]
                    case 0x1eb0: // [LATIN CAPITAL LETTER A WITH BREVE AND GRAVE]
                    case 0x1eb2: // [LATIN CAPITAL LETTER A WITH BREVE AND HOOK ABOVE]
                    case 0x1eb4: // [LATIN CAPITAL LETTER A WITH BREVE AND TILDE]
                    case 0x1eb6: // [LATIN CAPITAL LETTER A WITH BREVE AND DOT BELOW]
                    case 0x24b6: // [CIRCLED LATIN CAPITAL LETTER A]
                    case 0xff21: // [FULLWIDTH LATIN CAPITAL LETTER A]
                        output[outputPos++] = L'A';
                        break;
                    case 0x00e0: // [LATIN SMALL LETTER A WITH GRAVE]
                    case 0x00e1: // [LATIN SMALL LETTER A WITH ACUTE]
                    case 0x00e2: // [LATIN SMALL LETTER A WITH CIRCUMFLEX]
                    case 0x00e3: // [LATIN SMALL LETTER A WITH TILDE]
                    case 0x00e4: // [LATIN SMALL LETTER A WITH DIAERESIS]
                    case 0x00e5: // [LATIN SMALL LETTER A WITH RING ABOVE]
                    case 0x0101: // [LATIN SMALL LETTER A WITH MACRON]
                    case 0x0103: // [LATIN SMALL LETTER A WITH BREVE]
                    case 0x0105: // [LATIN SMALL LETTER A WITH OGONEK]
                    case 0x01ce: // [LATIN SMALL LETTER A WITH CARON]
                    case 0x01df: // [LATIN SMALL LETTER A WITH DIAERESIS AND MACRON]
                    case 0x01e1: // [LATIN SMALL LETTER A WITH DOT ABOVE AND MACRON]
                    case 0x01fb: // [LATIN SMALL LETTER A WITH RING ABOVE AND ACUTE]
                    case 0x0201: // [LATIN SMALL LETTER A WITH DOUBLE GRAVE]
                    case 0x0203: // [LATIN SMALL LETTER A WITH INVERTED BREVE]
                    case 0x0227: // [LATIN SMALL LETTER A WITH DOT ABOVE]
                    case 0x0250: // [LATIN SMALL LETTER TURNED A]
                    case 0x0259: // [LATIN SMALL LETTER SCHWA]
                    case 0x025a: // [LATIN SMALL LETTER SCHWA WITH HOOK]
                    case 0x1d8f: // [LATIN SMALL LETTER A WITH RETROFLEX HOOK]
                    case 0x1d95: // [LATIN SMALL LETTER SCHWA WITH RETROFLEX HOOK]
                    case 0x1e01: // [LATIN SMALL LETTER A WITH RING BELOW]
                    case 0x1e9a: // [LATIN SMALL LETTER A WITH RIGHT HALF RING]
                    case 0x1ea1: // [LATIN SMALL LETTER A WITH DOT BELOW]
                    case 0x1ea3: // [LATIN SMALL LETTER A WITH HOOK ABOVE]
                    case 0x1ea5: // [LATIN SMALL LETTER A WITH CIRCUMFLEX AND ACUTE]
                    case 0x1ea7: // [LATIN SMALL LETTER A WITH CIRCUMFLEX AND GRAVE]
                    case 0x1ea9: // [LATIN SMALL LETTER A WITH CIRCUMFLEX AND HOOK ABOVE]
                    case 0x1eab: // [LATIN SMALL LETTER A WITH CIRCUMFLEX AND TILDE]
                    case 0x1ead: // [LATIN SMALL LETTER A WITH CIRCUMFLEX AND DOT BELOW]
                    case 0x1eaf: // [LATIN SMALL LETTER A WITH BREVE AND ACUTE]
                    case 0x1eb1: // [LATIN SMALL LETTER A WITH BREVE AND GRAVE]
                    case 0x1eb3: // [LATIN SMALL LETTER A WITH BREVE AND HOOK ABOVE]
                    case 0x1eb5: // [LATIN SMALL LETTER A WITH BREVE AND TILDE]
                    case 0x1eb7: // [LATIN SMALL LETTER A WITH BREVE AND DOT BELOW]
                    case 0x2090: // [LATIN SUBSCRIPT SMALL LETTER A]
                    case 0x2094: // [LATIN SUBSCRIPT SMALL LETTER SCHWA]
                    case 0x24d0: // [CIRCLED LATIN SMALL LETTER A]
                    case 0x2c65: // [LATIN SMALL LETTER A WITH STROKE]
                    case 0x2c6f: // [LATIN CAPITAL LETTER TURNED A]
                    case 0xff41: // [FULLWIDTH LATIN SMALL LETTER A]
                        output[outputPos++] = L'a';
                        break;
                    case 0xa732: // [LATIN CAPITAL LETTER AA]
                        output[outputPos++] = L'A';
                        output[outputPos++] = L'A';
                        break;
                    case 0x00c6: // [LATIN CAPITAL LETTER AE]
                    case 0x01e2: // [LATIN CAPITAL LETTER AE WITH MACRON]
                    case 0x01fc: // [LATIN CAPITAL LETTER AE WITH ACUTE]
                    case 0x1d01: // [LATIN LETTER SMALL CAPITAL AE]
                        output[outputPos++] = L'A';
                        output[outputPos++] = L'E';
                        break;
                    case 0xa734: // [LATIN CAPITAL LETTER AO]
                        output[outputPos++] = L'A';                    
                        output[outputPos++] = L'O';
                        break;
                    case 0xa736: // [LATIN CAPITAL LETTER AU]
                        output[outputPos++] = L'A';
                        output[outputPos++] = L'U';
                        break;
                    case 0xa738: // [LATIN CAPITAL LETTER AV]
                    case 0xa73a: // [LATIN CAPITAL LETTER AV WITH HORIZONTAL BAR]
                        output[outputPos++] = L'A';
                        output[outputPos++] = L'V';
                        break;
                    case 0xa73c: // [LATIN CAPITAL LETTER AY]
                        output[outputPos++] = L'A';
                        output[outputPos++] = L'Y';
                        break;
                    case 0x249c: // [PARENTHESIZED LATIN SMALL LETTER A]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'a';
                        output[outputPos++] = L')';
                        break;
                    case 0xa733: // [LATIN SMALL LETTER AA]
                        output[outputPos++] = L'a';
                        output[outputPos++] = L'a';
                        break;
                    case 0x00e6: // [LATIN SMALL LETTER AE]
                    case 0x01e3: // [LATIN SMALL LETTER AE WITH MACRON]
                    case 0x01fd: // [LATIN SMALL LETTER AE WITH ACUTE]
                    case 0x1d02: // [LATIN SMALL LETTER TURNED AE]
                        output[outputPos++] = L'a';
                        output[outputPos++] = L'e';
                        break;
                    case 0xa735: // [LATIN SMALL LETTER AO]
                        output[outputPos++] = L'a';
                        output[outputPos++] = L'o';
                        break;
                    case 0xa737: // [LATIN SMALL LETTER AU]
                        output[outputPos++] = L'a';
                        output[outputPos++] = L'u';
                        break;
                    case 0xa739: // [LATIN SMALL LETTER AV]
                    case 0xa73b: // [LATIN SMALL LETTER AV WITH HORIZONTAL BAR]
                        output[outputPos++] = L'a';
                        output[outputPos++] = L'v';
                        break;
                    case 0xa73d: // [LATIN SMALL LETTER AY]
                        output[outputPos++] = L'a';
                        output[outputPos++] = L'y';
                        break;
                    case 0x0181: // [LATIN CAPITAL LETTER B WITH HOOK]
                    case 0x0182: // [LATIN CAPITAL LETTER B WITH TOPBAR]
                    case 0x0243: // [LATIN CAPITAL LETTER B WITH STROKE]
                    case 0x0299: // [LATIN LETTER SMALL CAPITAL B]
                    case 0x1d03: // [LATIN LETTER SMALL CAPITAL BARRED B]
                    case 0x1e02: // [LATIN CAPITAL LETTER B WITH DOT ABOVE]
                    case 0x1e04: // [LATIN CAPITAL LETTER B WITH DOT BELOW]
                    case 0x1e06: // [LATIN CAPITAL LETTER B WITH LINE BELOW]
                    case 0x24b7: // [CIRCLED LATIN CAPITAL LETTER B]
                    case 0xff22: // [FULLWIDTH LATIN CAPITAL LETTER B]
                        output[outputPos++] = L'B';
                        break;
                    case 0x0180: // [LATIN SMALL LETTER B WITH STROKE]
                    case 0x0183: // [LATIN SMALL LETTER B WITH TOPBAR]
                    case 0x0253: // [LATIN SMALL LETTER B WITH HOOK]
                    case 0x1d6c: // [LATIN SMALL LETTER B WITH MIDDLE TILDE]
                    case 0x1d80: // [LATIN SMALL LETTER B WITH PALATAL HOOK]
                    case 0x1e03: // [LATIN SMALL LETTER B WITH DOT ABOVE]
                    case 0x1e05: // [LATIN SMALL LETTER B WITH DOT BELOW]
                    case 0x1e07: // [LATIN SMALL LETTER B WITH LINE BELOW]
                    case 0x24d1: // [CIRCLED LATIN SMALL LETTER B]
                    case 0xff42: // [FULLWIDTH LATIN SMALL LETTER B]
                        output[outputPos++] = L'b';
                        break;
                    case 0x249d: // [PARENTHESIZED LATIN SMALL LETTER B]
                        output[outputPos++] = L'(';                    
                        output[outputPos++] = L'b';
                        output[outputPos++] = L')';
                        break;
                    case 0x00c7: // [LATIN CAPITAL LETTER C WITH CEDILLA]
                    case 0x0106: // [LATIN CAPITAL LETTER C WITH ACUTE]
                    case 0x0108: // [LATIN CAPITAL LETTER C WITH CIRCUMFLEX]
                    case 0x010a: // [LATIN CAPITAL LETTER C WITH DOT ABOVE]
                    case 0x010c: // [LATIN CAPITAL LETTER C WITH CARON]
                    case 0x0187: // [LATIN CAPITAL LETTER C WITH HOOK]
                    case 0x023b: // [LATIN CAPITAL LETTER C WITH STROKE]
                    case 0x0297: // [LATIN LETTER STRETCHED C]
                    case 0x1d04: // [LATIN LETTER SMALL CAPITAL C]
                    case 0x1e08: // [LATIN CAPITAL LETTER C WITH CEDILLA AND ACUTE]
                    case 0x24b8: // [CIRCLED LATIN CAPITAL LETTER C]
                    case 0xff23: // [FULLWIDTH LATIN CAPITAL LETTER C]
                        output[outputPos++] = L'C';
                        break;
                    case 0x00e7: // [LATIN SMALL LETTER C WITH CEDILLA]
                    case 0x0107: // [LATIN SMALL LETTER C WITH ACUTE]
                    case 0x0109: // [LATIN SMALL LETTER C WITH CIRCUMFLEX]
                    case 0x010b: // [LATIN SMALL LETTER C WITH DOT ABOVE]
                    case 0x010d: // [LATIN SMALL LETTER C WITH CARON]
                    case 0x0188: // [LATIN SMALL LETTER C WITH HOOK]
                    case 0x023c: // [LATIN SMALL LETTER C WITH STROKE]
                    case 0x0255: // [LATIN SMALL LETTER C WITH CURL]
                    case 0x1e09: // [LATIN SMALL LETTER C WITH CEDILLA AND ACUTE]
                    case 0x2184: // [LATIN SMALL LETTER REVERSED C]
                    case 0x24d2: // [CIRCLED LATIN SMALL LETTER C]
                    case 0xa73e: // [LATIN CAPITAL LETTER REVERSED C WITH DOT]
                    case 0xa73f: // [LATIN SMALL LETTER REVERSED C WITH DOT]
                    case 0xff43: // [FULLWIDTH LATIN SMALL LETTER C]
                        output[outputPos++] = L'c';
                        break;
                    case 0x249e: // [PARENTHESIZED LATIN SMALL LETTER C]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'c';
                        output[outputPos++] = L')';
                        break;
                    case 0x00d0: // [LATIN CAPITAL LETTER ETH]
                    case 0x010e: // [LATIN CAPITAL LETTER D WITH CARON]
                    case 0x0110: // [LATIN CAPITAL LETTER D WITH STROKE]
                    case 0x0189: // [LATIN CAPITAL LETTER AFRICAN D]
                    case 0x018a: // [LATIN CAPITAL LETTER D WITH HOOK]
                    case 0x018b: // [LATIN CAPITAL LETTER D WITH TOPBAR]
                    case 0x1d05: // [LATIN LETTER SMALL CAPITAL D]
                    case 0x1d06: // [LATIN LETTER SMALL CAPITAL ETH]
                    case 0x1e0a: // [LATIN CAPITAL LETTER D WITH DOT ABOVE]
                    case 0x1e0c: // [LATIN CAPITAL LETTER D WITH DOT BELOW]
                    case 0x1e0e: // [LATIN CAPITAL LETTER D WITH LINE BELOW]
                    case 0x1e10: // [LATIN CAPITAL LETTER D WITH CEDILLA]
                    case 0x1e12: // [LATIN CAPITAL LETTER D WITH CIRCUMFLEX BELOW]
                    case 0x24b9: // [CIRCLED LATIN CAPITAL LETTER D]
                    case 0xa779: // [LATIN CAPITAL LETTER INSULAR D]
                    case 0xff24: // [FULLWIDTH LATIN CAPITAL LETTER D]
                        output[outputPos++] = L'D';
                        break;
                    case 0x00f0: // [LATIN SMALL LETTER ETH]
                    case 0x010f: // [LATIN SMALL LETTER D WITH CARON]
                    case 0x0111: // [LATIN SMALL LETTER D WITH STROKE]
                    case 0x018c: // [LATIN SMALL LETTER D WITH TOPBAR]
                    case 0x0221: // [LATIN SMALL LETTER D WITH CURL]
                    case 0x0256: // [LATIN SMALL LETTER D WITH TAIL]
                    case 0x0257: // [LATIN SMALL LETTER D WITH HOOK]
                    case 0x1d6d: // [LATIN SMALL LETTER D WITH MIDDLE TILDE]
                    case 0x1d81: // [LATIN SMALL LETTER D WITH PALATAL HOOK]
                    case 0x1d91: // [LATIN SMALL LETTER D WITH HOOK AND TAIL]
                    case 0x1e0b: // [LATIN SMALL LETTER D WITH DOT ABOVE]
                    case 0x1e0d: // [LATIN SMALL LETTER D WITH DOT BELOW]
                    case 0x1e0f: // [LATIN SMALL LETTER D WITH LINE BELOW]
                    case 0x1e11: // [LATIN SMALL LETTER D WITH CEDILLA]
                    case 0x1e13: // [LATIN SMALL LETTER D WITH CIRCUMFLEX BELOW]
                    case 0x24d3: // [CIRCLED LATIN SMALL LETTER D]
                    case 0xa77a: // [LATIN SMALL LETTER INSULAR D]
                    case 0xff44: // [FULLWIDTH LATIN SMALL LETTER D]
                        output[outputPos++] = L'd';
                        break;
                    case 0x01c4: // [LATIN CAPITAL LETTER DZ WITH CARON]
                    case 0x01f1: // [LATIN CAPITAL LETTER DZ]
                        output[outputPos++] = L'D';
                        output[outputPos++] = L'Z';
                        break;
                    case 0x01c5: // [LATIN CAPITAL LETTER D WITH SMALL LETTER Z WITH CARON]
                    case 0x01f2: // [LATIN CAPITAL LETTER D WITH SMALL LETTER Z]
                        output[outputPos++] = L'D';
                        output[outputPos++] = L'z';
                        break;
                    case 0x249f: // [PARENTHESIZED LATIN SMALL LETTER D]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'd';
                        output[outputPos++] = L')';
                        break;
                    case 0x0238: // [LATIN SMALL LETTER DB DIGRAPH]
                        output[outputPos++] = L'd';
                        output[outputPos++] = L'b';
                        break;
                    case 0x01c6: // [LATIN SMALL LETTER DZ WITH CARON]
                    case 0x01f3: // [LATIN SMALL LETTER DZ]
                    case 0x02a3: // [LATIN SMALL LETTER DZ DIGRAPH]
                    case 0x02a5: // [LATIN SMALL LETTER DZ DIGRAPH WITH CURL]
                        output[outputPos++] = L'd';
                        output[outputPos++] = L'z';
                        break;
                    case 0x00c8: // [LATIN CAPITAL LETTER E WITH GRAVE]
                    case 0x00c9: // [LATIN CAPITAL LETTER E WITH ACUTE]
                    case 0x00ca: // [LATIN CAPITAL LETTER E WITH CIRCUMFLEX]
                    case 0x00cb: // [LATIN CAPITAL LETTER E WITH DIAERESIS]
                    case 0x0112: // [LATIN CAPITAL LETTER E WITH MACRON]
                    case 0x0114: // [LATIN CAPITAL LETTER E WITH BREVE]
                    case 0x0116: // [LATIN CAPITAL LETTER E WITH DOT ABOVE]
                    case 0x0118: // [LATIN CAPITAL LETTER E WITH OGONEK]
                    case 0x011a: // [LATIN CAPITAL LETTER E WITH CARON]
                    case 0x018e: // [LATIN CAPITAL LETTER REVERSED E]
                    case 0x0190: // [LATIN CAPITAL LETTER OPEN E]
                    case 0x0204: // [LATIN CAPITAL LETTER E WITH DOUBLE GRAVE]
                    case 0x0206: // [LATIN CAPITAL LETTER E WITH INVERTED BREVE]
                    case 0x0228: // [LATIN CAPITAL LETTER E WITH CEDILLA]
                    case 0x0246: // [LATIN CAPITAL LETTER E WITH STROKE]
                    case 0x1d07: // [LATIN LETTER SMALL CAPITAL E]
                    case 0x1e14: // [LATIN CAPITAL LETTER E WITH MACRON AND GRAVE]
                    case 0x1e16: // [LATIN CAPITAL LETTER E WITH MACRON AND ACUTE]
                    case 0x1e18: // [LATIN CAPITAL LETTER E WITH CIRCUMFLEX BELOW]
                    case 0x1e1a: // [LATIN CAPITAL LETTER E WITH TILDE BELOW]
                    case 0x1e1c: // [LATIN CAPITAL LETTER E WITH CEDILLA AND BREVE]
                    case 0x1eb8: // [LATIN CAPITAL LETTER E WITH DOT BELOW]
                    case 0x1eba: // [LATIN CAPITAL LETTER E WITH HOOK ABOVE]
                    case 0x1ebc: // [LATIN CAPITAL LETTER E WITH TILDE]
                    case 0x1ebe: // [LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND ACUTE]
                    case 0x1ec0: // [LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND GRAVE]
                    case 0x1ec2: // [LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND HOOK ABOVE]
                    case 0x1ec4: // [LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND TILDE]
                    case 0x1ec6: // [LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND DOT BELOW]
                    case 0x24ba: // [CIRCLED LATIN CAPITAL LETTER E]
                    case 0x2c7b: // [LATIN LETTER SMALL CAPITAL TURNED E]
                    case 0xff25: // [FULLWIDTH LATIN CAPITAL LETTER E]
                        output[outputPos++] = L'E';
                        break;
                    case 0x00e8: // [LATIN SMALL LETTER E WITH GRAVE]
                    case 0x00e9: // [LATIN SMALL LETTER E WITH ACUTE]
                    case 0x00ea: // [LATIN SMALL LETTER E WITH CIRCUMFLEX]
                    case 0x00eb: // [LATIN SMALL LETTER E WITH DIAERESIS]
                    case 0x0113: // [LATIN SMALL LETTER E WITH MACRON]
                    case 0x0115: // [LATIN SMALL LETTER E WITH BREVE]
                    case 0x0117: // [LATIN SMALL LETTER E WITH DOT ABOVE]
                    case 0x0119: // [LATIN SMALL LETTER E WITH OGONEK]
                    case 0x011b: // [LATIN SMALL LETTER E WITH CARON]
                    case 0x01dd: // [LATIN SMALL LETTER TURNED E]
                    case 0x0205: // [LATIN SMALL LETTER E WITH DOUBLE GRAVE]
                    case 0x0207: // [LATIN SMALL LETTER E WITH INVERTED BREVE]
                    case 0x0229: // [LATIN SMALL LETTER E WITH CEDILLA]
                    case 0x0247: // [LATIN SMALL LETTER E WITH STROKE]
                    case 0x0258: // [LATIN SMALL LETTER REVERSED E]
                    case 0x025b: // [LATIN SMALL LETTER OPEN E]
                    case 0x025c: // [LATIN SMALL LETTER REVERSED OPEN E]
                    case 0x025d: // [LATIN SMALL LETTER REVERSED OPEN E WITH HOOK]
                    case 0x025e: // [LATIN SMALL LETTER CLOSED REVERSED OPEN E]
                    case 0x029a: // [LATIN SMALL LETTER CLOSED OPEN E]
                    case 0x1d08: // [LATIN SMALL LETTER TURNED OPEN E]
                    case 0x1d92: // [LATIN SMALL LETTER E WITH RETROFLEX HOOK]
                    case 0x1d93: // [LATIN SMALL LETTER OPEN E WITH RETROFLEX HOOK]
                    case 0x1d94: // [LATIN SMALL LETTER REVERSED OPEN E WITH RETROFLEX HOOK]
                    case 0x1e15: // [LATIN SMALL LETTER E WITH MACRON AND GRAVE]
                    case 0x1e17: // [LATIN SMALL LETTER E WITH MACRON AND ACUTE]
                    case 0x1e19: // [LATIN SMALL LETTER E WITH CIRCUMFLEX BELOW]
                    case 0x1e1b: // [LATIN SMALL LETTER E WITH TILDE BELOW]
                    case 0x1e1d: // [LATIN SMALL LETTER E WITH CEDILLA AND BREVE]
                    case 0x1eb9: // [LATIN SMALL LETTER E WITH DOT BELOW]
                    case 0x1ebb: // [LATIN SMALL LETTER E WITH HOOK ABOVE]
                    case 0x1ebd: // [LATIN SMALL LETTER E WITH TILDE]
                    case 0x1ebf: // [LATIN SMALL LETTER E WITH CIRCUMFLEX AND ACUTE]
                    case 0x1ec1: // [LATIN SMALL LETTER E WITH CIRCUMFLEX AND GRAVE]
                    case 0x1ec3: // [LATIN SMALL LETTER E WITH CIRCUMFLEX AND HOOK ABOVE]
                    case 0x1ec5: // [LATIN SMALL LETTER E WITH CIRCUMFLEX AND TILDE]
                    case 0x1ec7: // [LATIN SMALL LETTER E WITH CIRCUMFLEX AND DOT BELOW]
                    case 0x2091: // [LATIN SUBSCRIPT SMALL LETTER E]
                    case 0x24d4: // [CIRCLED LATIN SMALL LETTER E]
                    case 0x2c78: // [LATIN SMALL LETTER E WITH NOTCH]
                    case 0xff45: // [FULLWIDTH LATIN SMALL LETTER E]
                        output[outputPos++] = L'e';
                        break;
                    case 0x24a0: // [PARENTHESIZED LATIN SMALL LETTER E]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'e';
                        output[outputPos++] = L')';
                        break;
                    case 0x0191: // [LATIN CAPITAL LETTER F WITH HOOK]
                    case 0x1e1e: // [LATIN CAPITAL LETTER F WITH DOT ABOVE]
                    case 0x24bb: // [CIRCLED LATIN CAPITAL LETTER F]
                    case 0xa730: // [LATIN LETTER SMALL CAPITAL F]
                    case 0xa77b: // [LATIN CAPITAL LETTER INSULAR F]
                    case 0xa7fb: // [LATIN EPIGRAPHIC LETTER REVERSED F]
                    case 0xff26: // [FULLWIDTH LATIN CAPITAL LETTER F]
                        output[outputPos++] = L'F';
                        break;
                    case 0x0192: // [LATIN SMALL LETTER F WITH HOOK]
                    case 0x1d6e: // [LATIN SMALL LETTER F WITH MIDDLE TILDE]
                    case 0x1d82: // [LATIN SMALL LETTER F WITH PALATAL HOOK]
                    case 0x1e1f: // [LATIN SMALL LETTER F WITH DOT ABOVE]
                    case 0x1e9b: // [LATIN SMALL LETTER LONG S WITH DOT ABOVE]
                    case 0x24d5: // [CIRCLED LATIN SMALL LETTER F]
                    case 0xa77c: // [LATIN SMALL LETTER INSULAR F]
                    case 0xff46: // [FULLWIDTH LATIN SMALL LETTER F]
                        output[outputPos++] = L'f';
                        break;
                    case 0x24a1: // [PARENTHESIZED LATIN SMALL LETTER F]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'f';
                        output[outputPos++] = L')';
                        break;
                    case 0xfb00: // [LATIN SMALL LIGATURE FF]
                        output[outputPos++] = L'f';
                        output[outputPos++] = L'f';
                        break;
                    case 0xfb03: // [LATIN SMALL LIGATURE FFI]
                        output[outputPos++] = L'f';
                        output[outputPos++] = L'f';
                        output[outputPos++] = L'i';
                        break;
                    case 0xfb04: // [LATIN SMALL LIGATURE FFL]
                        output[outputPos++] = L'f';
                        output[outputPos++] = L'f';
                        output[outputPos++] = L'l';
                        break;
                    case 0xfb01: // [LATIN SMALL LIGATURE FI]
                        output[outputPos++] = L'f';
                        output[outputPos++] = L'i';
                        break;
                    case 0xfb02: // [LATIN SMALL LIGATURE FL]
                        output[outputPos++] = L'f';
                        output[outputPos++] = L'l';
                        break;
                    case 0x011c: // [LATIN CAPITAL LETTER G WITH CIRCUMFLEX]
                    case 0x011e: // [LATIN CAPITAL LETTER G WITH BREVE]
                    case 0x0120: // [LATIN CAPITAL LETTER G WITH DOT ABOVE]
                    case 0x0122: // [LATIN CAPITAL LETTER G WITH CEDILLA]
                    case 0x0193: // [LATIN CAPITAL LETTER G WITH HOOK]
                    case 0x01e4: // [LATIN CAPITAL LETTER G WITH STROKE]
                    case 0x01e5: // [LATIN SMALL LETTER G WITH STROKE]
                    case 0x01e6: // [LATIN CAPITAL LETTER G WITH CARON]
                    case 0x01e7: // [LATIN SMALL LETTER G WITH CARON]
                    case 0x01f4: // [LATIN CAPITAL LETTER G WITH ACUTE]
                    case 0x0262: // [LATIN LETTER SMALL CAPITAL G]
                    case 0x029b: // [LATIN LETTER SMALL CAPITAL G WITH HOOK]
                    case 0x1e20: // [LATIN CAPITAL LETTER G WITH MACRON]
                    case 0x24bc: // [CIRCLED LATIN CAPITAL LETTER G]
                    case 0xa77d: // [LATIN CAPITAL LETTER INSULAR G]
                    case 0xa77e: // [LATIN CAPITAL LETTER TURNED INSULAR G]
                    case 0xff27: // [FULLWIDTH LATIN CAPITAL LETTER G]
                        output[outputPos++] = L'G';
                        break;
                    case 0x011d: // [LATIN SMALL LETTER G WITH CIRCUMFLEX]
                    case 0x011f: // [LATIN SMALL LETTER G WITH BREVE]
                    case 0x0121: // [LATIN SMALL LETTER G WITH DOT ABOVE]
                    case 0x0123: // [LATIN SMALL LETTER G WITH CEDILLA]
                    case 0x01f5: // [LATIN SMALL LETTER G WITH ACUTE]
                    case 0x0260: // [LATIN SMALL LETTER G WITH HOOK]
                    case 0x0261: // [LATIN SMALL LETTER SCRIPT G]
                    case 0x1d77: // [LATIN SMALL LETTER TURNED G]
                    case 0x1d79: // [LATIN SMALL LETTER INSULAR G]
                    case 0x1d83: // [LATIN SMALL LETTER G WITH PALATAL HOOK]
                    case 0x1e21: // [LATIN SMALL LETTER G WITH MACRON]
                    case 0x24d6: // [CIRCLED LATIN SMALL LETTER G]
                    case 0xa77f: // [LATIN SMALL LETTER TURNED INSULAR G]
                    case 0xff47: // [FULLWIDTH LATIN SMALL LETTER G]
                        output[outputPos++] = L'g';
                        break;
                    case 0x24a2: // [PARENTHESIZED LATIN SMALL LETTER G]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'g';
                        output[outputPos++] = L')';
                        break;
                    case 0x0124: // [LATIN CAPITAL LETTER H WITH CIRCUMFLEX]
                    case 0x0126: // [LATIN CAPITAL LETTER H WITH STROKE]
                    case 0x021e: // [LATIN CAPITAL LETTER H WITH CARON]
                    case 0x029c: // [LATIN LETTER SMALL CAPITAL H]
                    case 0x1e22: // [LATIN CAPITAL LETTER H WITH DOT ABOVE]
                    case 0x1e24: // [LATIN CAPITAL LETTER H WITH DOT BELOW]
                    case 0x1e26: // [LATIN CAPITAL LETTER H WITH DIAERESIS]
                    case 0x1e28: // [LATIN CAPITAL LETTER H WITH CEDILLA]
                    case 0x1e2a: // [LATIN CAPITAL LETTER H WITH BREVE BELOW]
                    case 0x24bd: // [CIRCLED LATIN CAPITAL LETTER H]
                    case 0x2c67: // [LATIN CAPITAL LETTER H WITH DESCENDER]
                    case 0x2c75: // [LATIN CAPITAL LETTER HALF H]
                    case 0xff28: // [FULLWIDTH LATIN CAPITAL LETTER H]
                        output[outputPos++] = L'H';
                        break;
                    case 0x0125: // [LATIN SMALL LETTER H WITH CIRCUMFLEX]
                    case 0x0127: // [LATIN SMALL LETTER H WITH STROKE]
                    case 0x021f: // [LATIN SMALL LETTER H WITH CARON]
                    case 0x0265: // [LATIN SMALL LETTER TURNED H]
                    case 0x0266: // [LATIN SMALL LETTER H WITH HOOK]
                    case 0x02ae: // [LATIN SMALL LETTER TURNED H WITH FISHHOOK]
                    case 0x02af: // [LATIN SMALL LETTER TURNED H WITH FISHHOOK AND TAIL]
                    case 0x1e23: // [LATIN SMALL LETTER H WITH DOT ABOVE]
                    case 0x1e25: // [LATIN SMALL LETTER H WITH DOT BELOW]
                    case 0x1e27: // [LATIN SMALL LETTER H WITH DIAERESIS]
                    case 0x1e29: // [LATIN SMALL LETTER H WITH CEDILLA]
                    case 0x1e2b: // [LATIN SMALL LETTER H WITH BREVE BELOW]
                    case 0x1e96: // [LATIN SMALL LETTER H WITH LINE BELOW]
                    case 0x24d7: // [CIRCLED LATIN SMALL LETTER H]
                    case 0x2c68: // [LATIN SMALL LETTER H WITH DESCENDER]
                    case 0x2c76: // [LATIN SMALL LETTER HALF H]
                    case 0xff48: // [FULLWIDTH LATIN SMALL LETTER H]
                        output[outputPos++] = L'h';
                        break;
                    case 0x01f6: // [LATIN CAPITAL LETTER HWAIR]
                        output[outputPos++] = L'H';
                        output[outputPos++] = L'V';
                        break;
                    case 0x24a3: // [PARENTHESIZED LATIN SMALL LETTER H]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'h';
                        output[outputPos++] = L')';
                        break;
                    case 0x0195: // [LATIN SMALL LETTER HV]
                        output[outputPos++] = L'h';
                        output[outputPos++] = L'v';
                        break;
                    case 0x00cc: // [LATIN CAPITAL LETTER I WITH GRAVE]
                    case 0x00cd: // [LATIN CAPITAL LETTER I WITH ACUTE]
                    case 0x00ce: // [LATIN CAPITAL LETTER I WITH CIRCUMFLEX]
                    case 0x00cf: // [LATIN CAPITAL LETTER I WITH DIAERESIS]
                    case 0x0128: // [LATIN CAPITAL LETTER I WITH TILDE]
                    case 0x012a: // [LATIN CAPITAL LETTER I WITH MACRON]
                    case 0x012c: // [LATIN CAPITAL LETTER I WITH BREVE]
                    case 0x012e: // [LATIN CAPITAL LETTER I WITH OGONEK]
                    case 0x0130: // [LATIN CAPITAL LETTER I WITH DOT ABOVE]
                    case 0x0196: // [LATIN CAPITAL LETTER IOTA]
                    case 0x0197: // [LATIN CAPITAL LETTER I WITH STROKE]
                    case 0x01cf: // [LATIN CAPITAL LETTER I WITH CARON]
                    case 0x0208: // [LATIN CAPITAL LETTER I WITH DOUBLE GRAVE]
                    case 0x020a: // [LATIN CAPITAL LETTER I WITH INVERTED BREVE]
                    case 0x026a: // [LATIN LETTER SMALL CAPITAL I]
                    case 0x1d7b: // [LATIN SMALL CAPITAL LETTER I WITH STROKE]
                    case 0x1e2c: // [LATIN CAPITAL LETTER I WITH TILDE BELOW]
                    case 0x1e2e: // [LATIN CAPITAL LETTER I WITH DIAERESIS AND ACUTE]
                    case 0x1ec8: // [LATIN CAPITAL LETTER I WITH HOOK ABOVE]
                    case 0x1eca: // [LATIN CAPITAL LETTER I WITH DOT BELOW]
                    case 0x24be: // [CIRCLED LATIN CAPITAL LETTER I]
                    case 0xa7fe: // [LATIN EPIGRAPHIC LETTER I LONGA]
                    case 0xff29: // [FULLWIDTH LATIN CAPITAL LETTER I]
                        output[outputPos++] = L'I';
                        break;
                    case 0x00ec: // [LATIN SMALL LETTER I WITH GRAVE]
                    case 0x00ed: // [LATIN SMALL LETTER I WITH ACUTE]
                    case 0x00ee: // [LATIN SMALL LETTER I WITH CIRCUMFLEX]
                    case 0x00ef: // [LATIN SMALL LETTER I WITH DIAERESIS]
                    case 0x0129: // [LATIN SMALL LETTER I WITH TILDE]
                    case 0x012b: // [LATIN SMALL LETTER I WITH MACRON]
                    case 0x012d: // [LATIN SMALL LETTER I WITH BREVE]
                    case 0x012f: // [LATIN SMALL LETTER I WITH OGONEK]
                    case 0x0131: // [LATIN SMALL LETTER DOTLESS I]
                    case 0x01d0: // [LATIN SMALL LETTER I WITH CARON]
                    case 0x0209: // [LATIN SMALL LETTER I WITH DOUBLE GRAVE]
                    case 0x020b: // [LATIN SMALL LETTER I WITH INVERTED BREVE]
                    case 0x0268: // [LATIN SMALL LETTER I WITH STROKE]
                    case 0x1d09: // [LATIN SMALL LETTER TURNED I]
                    case 0x1d62: // [LATIN SUBSCRIPT SMALL LETTER I]
                    case 0x1d7c: // [LATIN SMALL LETTER IOTA WITH STROKE]
                    case 0x1d96: // [LATIN SMALL LETTER I WITH RETROFLEX HOOK]
                    case 0x1e2d: // [LATIN SMALL LETTER I WITH TILDE BELOW]
                    case 0x1e2f: // [LATIN SMALL LETTER I WITH DIAERESIS AND ACUTE]
                    case 0x1ec9: // [LATIN SMALL LETTER I WITH HOOK ABOVE]
                    case 0x1ecb: // [LATIN SMALL LETTER I WITH DOT BELOW]
                    case 0x2071: // [SUPERSCRIPT LATIN SMALL LETTER I]
                    case 0x24d8: // [CIRCLED LATIN SMALL LETTER I]
                    case 0xff49: // [FULLWIDTH LATIN SMALL LETTER I]
                        output[outputPos++] = L'i';
                        break;
                    case 0x0132: // [LATIN CAPITAL LIGATURE IJ]
                        output[outputPos++] = L'I';
                        output[outputPos++] = L'J';
                        break;
                    case 0x24a4: // [PARENTHESIZED LATIN SMALL LETTER I]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'i';
                        output[outputPos++] = L')';
                        break;
                    case 0x0133: // [LATIN SMALL LIGATURE IJ]
                        output[outputPos++] = L'i';
                        output[outputPos++] = L'j';
                        break;
                    case 0x0134: // [LATIN CAPITAL LETTER J WITH CIRCUMFLEX]
                    case 0x0248: // [LATIN CAPITAL LETTER J WITH STROKE]
                    case 0x1d0a: // [LATIN LETTER SMALL CAPITAL J]
                    case 0x24bf: // [CIRCLED LATIN CAPITAL LETTER J]
                    case 0xff2a: // [FULLWIDTH LATIN CAPITAL LETTER J]
                        output[outputPos++] = L'J';
                        break;
                    case 0x0135: // [LATIN SMALL LETTER J WITH CIRCUMFLEX]
                    case 0x01f0: // [LATIN SMALL LETTER J WITH CARON]
                    case 0x0237: // [LATIN SMALL LETTER DOTLESS J]
                    case 0x0249: // [LATIN SMALL LETTER J WITH STROKE]
                    case 0x025f: // [LATIN SMALL LETTER DOTLESS J WITH STROKE]
                    case 0x0284: // [LATIN SMALL LETTER DOTLESS J WITH STROKE AND HOOK]
                    case 0x029d: // [LATIN SMALL LETTER J WITH CROSSED-TAIL]
                    case 0x24d9: // [CIRCLED LATIN SMALL LETTER J]
                    case 0x2c7c: // [LATIN SUBSCRIPT SMALL LETTER J]
                    case 0xff4a: // [FULLWIDTH LATIN SMALL LETTER J]
                        output[outputPos++] = L'j';
                        break;
                    case 0x24a5: // [PARENTHESIZED LATIN SMALL LETTER J]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'j';
                        output[outputPos++] = L')';
                        break;
                    case 0x0136: // [LATIN CAPITAL LETTER K WITH CEDILLA]
                    case 0x0198: // [LATIN CAPITAL LETTER K WITH HOOK]
                    case 0x01e8: // [LATIN CAPITAL LETTER K WITH CARON]
                    case 0x1d0b: // [LATIN LETTER SMALL CAPITAL K]
                    case 0x1e30: // [LATIN CAPITAL LETTER K WITH ACUTE]
                    case 0x1e32: // [LATIN CAPITAL LETTER K WITH DOT BELOW]
                    case 0x1e34: // [LATIN CAPITAL LETTER K WITH LINE BELOW]
                    case 0x24c0: // [CIRCLED LATIN CAPITAL LETTER K]
                    case 0x2c69: // [LATIN CAPITAL LETTER K WITH DESCENDER]
                    case 0xa740: // [LATIN CAPITAL LETTER K WITH STROKE]
                    case 0xa742: // [LATIN CAPITAL LETTER K WITH DIAGONAL STROKE]
                    case 0xa744: // [LATIN CAPITAL LETTER K WITH STROKE AND DIAGONAL STROKE]
                    case 0xff2b: // [FULLWIDTH LATIN CAPITAL LETTER K]
                        output[outputPos++] = L'K';
                        break;
                    case 0x0137: // [LATIN SMALL LETTER K WITH CEDILLA]
                    case 0x0199: // [LATIN SMALL LETTER K WITH HOOK]
                    case 0x01e9: // [LATIN SMALL LETTER K WITH CARON]
                    case 0x029e: // [LATIN SMALL LETTER TURNED K]
                    case 0x1d84: // [LATIN SMALL LETTER K WITH PALATAL HOOK]
                    case 0x1e31: // [LATIN SMALL LETTER K WITH ACUTE]
                    case 0x1e33: // [LATIN SMALL LETTER K WITH DOT BELOW]
                    case 0x1e35: // [LATIN SMALL LETTER K WITH LINE BELOW]
                    case 0x24da: // [CIRCLED LATIN SMALL LETTER K]
                    case 0x2c6a: // [LATIN SMALL LETTER K WITH DESCENDER]
                    case 0xa741: // [LATIN SMALL LETTER K WITH STROKE]
                    case 0xa743: // [LATIN SMALL LETTER K WITH DIAGONAL STROKE]
                    case 0xa745: // [LATIN SMALL LETTER K WITH STROKE AND DIAGONAL STROKE]
                    case 0xff4b: // [FULLWIDTH LATIN SMALL LETTER K]
                        output[outputPos++] = L'k';
                        break;
                    case 0x24a6: // [PARENTHESIZED LATIN SMALL LETTER K]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'k';
                        output[outputPos++] = L')';
                        break;
                    case 0x0139: // [LATIN CAPITAL LETTER L WITH ACUTE]
                    case 0x013b: // [LATIN CAPITAL LETTER L WITH CEDILLA]
                    case 0x013d: // [LATIN CAPITAL LETTER L WITH CARON]
                    case 0x013f: // [LATIN CAPITAL LETTER L WITH MIDDLE DOT]
                    case 0x0141: // [LATIN CAPITAL LETTER L WITH STROKE]
                    case 0x023d: // [LATIN CAPITAL LETTER L WITH BAR]
                    case 0x029f: // [LATIN LETTER SMALL CAPITAL L]
                    case 0x1d0c: // [LATIN LETTER SMALL CAPITAL L WITH STROKE]
                    case 0x1e36: // [LATIN CAPITAL LETTER L WITH DOT BELOW]
                    case 0x1e38: // [LATIN CAPITAL LETTER L WITH DOT BELOW AND MACRON]
                    case 0x1e3a: // [LATIN CAPITAL LETTER L WITH LINE BELOW]
                    case 0x1e3c: // [LATIN CAPITAL LETTER L WITH CIRCUMFLEX BELOW]
                    case 0x24c1: // [CIRCLED LATIN CAPITAL LETTER L]
                    case 0x2c60: // [LATIN CAPITAL LETTER L WITH DOUBLE BAR]
                    case 0x2c62: // [LATIN CAPITAL LETTER L WITH MIDDLE TILDE]
                    case 0xa746: // [LATIN CAPITAL LETTER BROKEN L]
                    case 0xa748: // [LATIN CAPITAL LETTER L WITH HIGH STROKE]
                    case 0xa780: // [LATIN CAPITAL LETTER TURNED L]
                    case 0xff2c: // [FULLWIDTH LATIN CAPITAL LETTER L]
                        output[outputPos++] = L'L';
                        break;
                    case 0x013a: // [LATIN SMALL LETTER L WITH ACUTE]
                    case 0x013c: // [LATIN SMALL LETTER L WITH CEDILLA]
                    case 0x013e: // [LATIN SMALL LETTER L WITH CARON]
                    case 0x0140: // [LATIN SMALL LETTER L WITH MIDDLE DOT]
                    case 0x0142: // [LATIN SMALL LETTER L WITH STROKE]
                    case 0x019a: // [LATIN SMALL LETTER L WITH BAR]
                    case 0x0234: // [LATIN SMALL LETTER L WITH CURL]
                    case 0x026b: // [LATIN SMALL LETTER L WITH MIDDLE TILDE]
                    case 0x026c: // [LATIN SMALL LETTER L WITH BELT]
                    case 0x026d: // [LATIN SMALL LETTER L WITH RETROFLEX HOOK]
                    case 0x1d85: // [LATIN SMALL LETTER L WITH PALATAL HOOK]
                    case 0x1e37: // [LATIN SMALL LETTER L WITH DOT BELOW]
                    case 0x1e39: // [LATIN SMALL LETTER L WITH DOT BELOW AND MACRON]
                    case 0x1e3b: // [LATIN SMALL LETTER L WITH LINE BELOW]
                    case 0x1e3d: // [LATIN SMALL LETTER L WITH CIRCUMFLEX BELOW]
                    case 0x24db: // [CIRCLED LATIN SMALL LETTER L]
                    case 0x2c61: // [LATIN SMALL LETTER L WITH DOUBLE BAR]
                    case 0xa747: // [LATIN SMALL LETTER BROKEN L]
                    case 0xa749: // [LATIN SMALL LETTER L WITH HIGH STROKE]
                    case 0xa781: // [LATIN SMALL LETTER TURNED L]
                    case 0xff4c: // [FULLWIDTH LATIN SMALL LETTER L]
                        output[outputPos++] = L'l';
                        break;
                    case 0x01c7: // [LATIN CAPITAL LETTER LJ]
                        output[outputPos++] = L'L';
                        output[outputPos++] = L'J';
                        break;
                    case 0x1efa: // [LATIN CAPITAL LETTER MIDDLE-WELSH LL]
                        output[outputPos++] = L'L';
                        output[outputPos++] = L'L';
                        break;
                    case 0x01c8: // [LATIN CAPITAL LETTER L WITH SMALL LETTER J]
                        output[outputPos++] = L'L';
                        output[outputPos++] = L'j';
                        break;
                    case 0x24a7: // [PARENTHESIZED LATIN SMALL LETTER L]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'l';
                        output[outputPos++] = L')';
                        break;
                    case 0x01c9: // [LATIN SMALL LETTER LJ]
                        output[outputPos++] = L'l';
                        output[outputPos++] = L'j';
                        break;
                    case 0x1efb: // [LATIN SMALL LETTER MIDDLE-WELSH LL]
                        output[outputPos++] = L'l';
                        output[outputPos++] = L'l';
                        break;
                    case 0x02aa: // [LATIN SMALL LETTER LS DIGRAPH]
                        output[outputPos++] = L'l';
                        output[outputPos++] = L's';
                        break;
                    case 0x02ab: // [LATIN SMALL LETTER LZ DIGRAPH]
                        output[outputPos++] = L'l';
                        output[outputPos++] = L'z';
                        break;
                    case 0x019c: // [LATIN CAPITAL LETTER TURNED M]
                    case 0x1d0d: // [LATIN LETTER SMALL CAPITAL M]
                    case 0x1e3e: // [LATIN CAPITAL LETTER M WITH ACUTE]
                    case 0x1e40: // [LATIN CAPITAL LETTER M WITH DOT ABOVE]
                    case 0x1e42: // [LATIN CAPITAL LETTER M WITH DOT BELOW]
                    case 0x24c2: // [CIRCLED LATIN CAPITAL LETTER M]
                    case 0x2c6e: // [LATIN CAPITAL LETTER M WITH HOOK]
                    case 0xa7fd: // [LATIN EPIGRAPHIC LETTER INVERTED M]
                    case 0xa7ff: // [LATIN EPIGRAPHIC LETTER ARCHAIC M]
                    case 0xff2d: // [FULLWIDTH LATIN CAPITAL LETTER M]
                        output[outputPos++] = L'M';
                        break;
                    case 0x026f: // [LATIN SMALL LETTER TURNED M]
                    case 0x0270: // [LATIN SMALL LETTER TURNED M WITH LONG LEG]
                    case 0x0271: // [LATIN SMALL LETTER M WITH HOOK]
                    case 0x1d6f: // [LATIN SMALL LETTER M WITH MIDDLE TILDE]
                    case 0x1d86: // [LATIN SMALL LETTER M WITH PALATAL HOOK]
                    case 0x1e3f: // [LATIN SMALL LETTER M WITH ACUTE]
                    case 0x1e41: // [LATIN SMALL LETTER M WITH DOT ABOVE]
                    case 0x1e43: // [LATIN SMALL LETTER M WITH DOT BELOW]
                    case 0x24dc: // [CIRCLED LATIN SMALL LETTER M]
                    case 0xff4d: // [FULLWIDTH LATIN SMALL LETTER M]
                        output[outputPos++] = L'm';
                        break;
                    case 0x24a8: // [PARENTHESIZED LATIN SMALL LETTER M]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'm';
                        output[outputPos++] = L')';
                        break;
                    case 0x00d1: // [LATIN CAPITAL LETTER N WITH TILDE]
                    case 0x0143: // [LATIN CAPITAL LETTER N WITH ACUTE]
                    case 0x0145: // [LATIN CAPITAL LETTER N WITH CEDILLA]
                    case 0x0147: // [LATIN CAPITAL LETTER N WITH CARON]
                    case 0x014a: // [LATIN CAPITAL LETTER ENG]
                    case 0x019d: // [LATIN CAPITAL LETTER N WITH LEFT HOOK]
                    case 0x01f8: // [LATIN CAPITAL LETTER N WITH GRAVE]
                    case 0x0220: // [LATIN CAPITAL LETTER N WITH LONG RIGHT LEG]
                    case 0x0274: // [LATIN LETTER SMALL CAPITAL N]
                    case 0x1d0e: // [LATIN LETTER SMALL CAPITAL REVERSED N]
                    case 0x1e44: // [LATIN CAPITAL LETTER N WITH DOT ABOVE]
                    case 0x1e46: // [LATIN CAPITAL LETTER N WITH DOT BELOW]
                    case 0x1e48: // [LATIN CAPITAL LETTER N WITH LINE BELOW]
                    case 0x1e4a: // [LATIN CAPITAL LETTER N WITH CIRCUMFLEX BELOW]
                    case 0x24c3: // [CIRCLED LATIN CAPITAL LETTER N]
                    case 0xff2e: // [FULLWIDTH LATIN CAPITAL LETTER N]
                        output[outputPos++] = L'N';
                        break;
                    case 0x00f1: // [LATIN SMALL LETTER N WITH TILDE]
                    case 0x0144: // [LATIN SMALL LETTER N WITH ACUTE]
                    case 0x0146: // [LATIN SMALL LETTER N WITH CEDILLA]
                    case 0x0148: // [LATIN SMALL LETTER N WITH CARON]
                    case 0x0149: // [LATIN SMALL LETTER N PRECEDED BY APOSTROPHE]
                    case 0x014b: // [LATIN SMALL LETTER ENG]
                    case 0x019e: // [LATIN SMALL LETTER N WITH LONG RIGHT LEG]
                    case 0x01f9: // [LATIN SMALL LETTER N WITH GRAVE]
                    case 0x0235: // [LATIN SMALL LETTER N WITH CURL]
                    case 0x0272: // [LATIN SMALL LETTER N WITH LEFT HOOK]
                    case 0x0273: // [LATIN SMALL LETTER N WITH RETROFLEX HOOK]
                    case 0x1d70: // [LATIN SMALL LETTER N WITH MIDDLE TILDE]
                    case 0x1d87: // [LATIN SMALL LETTER N WITH PALATAL HOOK]
                    case 0x1e45: // [LATIN SMALL LETTER N WITH DOT ABOVE]
                    case 0x1e47: // [LATIN SMALL LETTER N WITH DOT BELOW]
                    case 0x1e49: // [LATIN SMALL LETTER N WITH LINE BELOW]
                    case 0x1e4b: // [LATIN SMALL LETTER N WITH CIRCUMFLEX BELOW]
                    case 0x207f: // [SUPERSCRIPT LATIN SMALL LETTER N]
                    case 0x24dd: // [CIRCLED LATIN SMALL LETTER N]
                    case 0xff4e: // [FULLWIDTH LATIN SMALL LETTER N]
                        output[outputPos++] = L'n';
                        break;
                    case 0x01ca: // [LATIN CAPITAL LETTER NJ]
                        output[outputPos++] = L'N';
                        output[outputPos++] = L'J';
                        break;
                    case 0x01cb: // [LATIN CAPITAL LETTER N WITH SMALL LETTER J]
                        output[outputPos++] = L'N';
                        output[outputPos++] = L'j';
                        break;
                    case 0x24a9: // [PARENTHESIZED LATIN SMALL LETTER N]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'n';
                        output[outputPos++] = L')';
                        break;
                    case 0x01cc: // [LATIN SMALL LETTER NJ]
                        output[outputPos++] = L'n';
                        output[outputPos++] = L'j';
                        break;
                    case 0x00d2: // [LATIN CAPITAL LETTER O WITH GRAVE]
                    case 0x00d3: // [LATIN CAPITAL LETTER O WITH ACUTE]
                    case 0x00d4: // [LATIN CAPITAL LETTER O WITH CIRCUMFLEX]
                    case 0x00d5: // [LATIN CAPITAL LETTER O WITH TILDE]
                    case 0x00d6: // [LATIN CAPITAL LETTER O WITH DIAERESIS]
                    case 0x00d8: // [LATIN CAPITAL LETTER O WITH STROKE]
                    case 0x014c: // [LATIN CAPITAL LETTER O WITH MACRON]
                    case 0x014e: // [LATIN CAPITAL LETTER O WITH BREVE]
                    case 0x0150: // [LATIN CAPITAL LETTER O WITH DOUBLE ACUTE]
                    case 0x0186: // [LATIN CAPITAL LETTER OPEN O]
                    case 0x019f: // [LATIN CAPITAL LETTER O WITH MIDDLE TILDE]
                    case 0x01a0: // [LATIN CAPITAL LETTER O WITH HORN]
                    case 0x01d1: // [LATIN CAPITAL LETTER O WITH CARON]
                    case 0x01ea: // [LATIN CAPITAL LETTER O WITH OGONEK]
                    case 0x01ec: // [LATIN CAPITAL LETTER O WITH OGONEK AND MACRON]
                    case 0x01fe: // [LATIN CAPITAL LETTER O WITH STROKE AND ACUTE]
                    case 0x020c: // [LATIN CAPITAL LETTER O WITH DOUBLE GRAVE]
                    case 0x020e: // [LATIN CAPITAL LETTER O WITH INVERTED BREVE]
                    case 0x022a: // [LATIN CAPITAL LETTER O WITH DIAERESIS AND MACRON]
                    case 0x022c: // [LATIN CAPITAL LETTER O WITH TILDE AND MACRON]
                    case 0x022e: // [LATIN CAPITAL LETTER O WITH DOT ABOVE]
                    case 0x0230: // [LATIN CAPITAL LETTER O WITH DOT ABOVE AND MACRON]
                    case 0x1d0f: // [LATIN LETTER SMALL CAPITAL O]
                    case 0x1d10: // [LATIN LETTER SMALL CAPITAL OPEN O]
                    case 0x1e4c: // [LATIN CAPITAL LETTER O WITH TILDE AND ACUTE]
                    case 0x1e4e: // [LATIN CAPITAL LETTER O WITH TILDE AND DIAERESIS]
                    case 0x1e50: // [LATIN CAPITAL LETTER O WITH MACRON AND GRAVE]
                    case 0x1e52: // [LATIN CAPITAL LETTER O WITH MACRON AND ACUTE]
                    case 0x1ecc: // [LATIN CAPITAL LETTER O WITH DOT BELOW]
                    case 0x1ece: // [LATIN CAPITAL LETTER O WITH HOOK ABOVE]
                    case 0x1ed0: // [LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND ACUTE]
                    case 0x1ed2: // [LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND GRAVE]
                    case 0x1ed4: // [LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND HOOK ABOVE]
                    case 0x1ed6: // [LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND TILDE]
                    case 0x1ed8: // [LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND DOT BELOW]
                    case 0x1eda: // [LATIN CAPITAL LETTER O WITH HORN AND ACUTE]
                    case 0x1edc: // [LATIN CAPITAL LETTER O WITH HORN AND GRAVE]
                    case 0x1ede: // [LATIN CAPITAL LETTER O WITH HORN AND HOOK ABOVE]
                    case 0x1ee0: // [LATIN CAPITAL LETTER O WITH HORN AND TILDE]
                    case 0x1ee2: // [LATIN CAPITAL LETTER O WITH HORN AND DOT BELOW]
                    case 0x24c4: // [CIRCLED LATIN CAPITAL LETTER O]
                    case 0xa74a: // [LATIN CAPITAL LETTER O WITH LONG STROKE OVERLAY]
                    case 0xa74c: // [LATIN CAPITAL LETTER O WITH LOOP]
                    case 0xff2f: // [FULLWIDTH LATIN CAPITAL LETTER O]
                        output[outputPos++] = L'O';
                        break;
                    case 0x00f2: // [LATIN SMALL LETTER O WITH GRAVE]
                    case 0x00f3: // [LATIN SMALL LETTER O WITH ACUTE]
                    case 0x00f4: // [LATIN SMALL LETTER O WITH CIRCUMFLEX]
                    case 0x00f5: // [LATIN SMALL LETTER O WITH TILDE]
                    case 0x00f6: // [LATIN SMALL LETTER O WITH DIAERESIS]
                    case 0x00f8: // [LATIN SMALL LETTER O WITH STROKE]
                    case 0x014d: // [LATIN SMALL LETTER O WITH MACRON]
                    case 0x014f: // [LATIN SMALL LETTER O WITH BREVE]
                    case 0x0151: // [LATIN SMALL LETTER O WITH DOUBLE ACUTE]
                    case 0x01a1: // [LATIN SMALL LETTER O WITH HORN]
                    case 0x01d2: // [LATIN SMALL LETTER O WITH CARON]
                    case 0x01eb: // [LATIN SMALL LETTER O WITH OGONEK]
                    case 0x01ed: // [LATIN SMALL LETTER O WITH OGONEK AND MACRON]
                    case 0x01ff: // [LATIN SMALL LETTER O WITH STROKE AND ACUTE]
                    case 0x020d: // [LATIN SMALL LETTER O WITH DOUBLE GRAVE]
                    case 0x020f: // [LATIN SMALL LETTER O WITH INVERTED BREVE]
                    case 0x022b: // [LATIN SMALL LETTER O WITH DIAERESIS AND MACRON]
                    case 0x022d: // [LATIN SMALL LETTER O WITH TILDE AND MACRON]
                    case 0x022f: // [LATIN SMALL LETTER O WITH DOT ABOVE]
                    case 0x0231: // [LATIN SMALL LETTER O WITH DOT ABOVE AND MACRON]
                    case 0x0254: // [LATIN SMALL LETTER OPEN O]
                    case 0x0275: // [LATIN SMALL LETTER BARRED O]
                    case 0x1d16: // [LATIN SMALL LETTER TOP HALF O]
                    case 0x1d17: // [LATIN SMALL LETTER BOTTOM HALF O]
                    case 0x1d97: // [LATIN SMALL LETTER OPEN O WITH RETROFLEX HOOK]
                    case 0x1e4d: // [LATIN SMALL LETTER O WITH TILDE AND ACUTE]
                    case 0x1e4f: // [LATIN SMALL LETTER O WITH TILDE AND DIAERESIS]
                    case 0x1e51: // [LATIN SMALL LETTER O WITH MACRON AND GRAVE]
                    case 0x1e53: // [LATIN SMALL LETTER O WITH MACRON AND ACUTE]
                    case 0x1ecd: // [LATIN SMALL LETTER O WITH DOT BELOW]
                    case 0x1ecf: // [LATIN SMALL LETTER O WITH HOOK ABOVE]
                    case 0x1ed1: // [LATIN SMALL LETTER O WITH CIRCUMFLEX AND ACUTE]
                    case 0x1ed3: // [LATIN SMALL LETTER O WITH CIRCUMFLEX AND GRAVE]
                    case 0x1ed5: // [LATIN SMALL LETTER O WITH CIRCUMFLEX AND HOOK ABOVE]
                    case 0x1ed7: // [LATIN SMALL LETTER O WITH CIRCUMFLEX AND TILDE]
                    case 0x1ed9: // [LATIN SMALL LETTER O WITH CIRCUMFLEX AND DOT BELOW]
                    case 0x1edb: // [LATIN SMALL LETTER O WITH HORN AND ACUTE]
                    case 0x1edd: // [LATIN SMALL LETTER O WITH HORN AND GRAVE]
                    case 0x1edf: // [LATIN SMALL LETTER O WITH HORN AND HOOK ABOVE]
                    case 0x1ee1: // [LATIN SMALL LETTER O WITH HORN AND TILDE]
                    case 0x1ee3: // [LATIN SMALL LETTER O WITH HORN AND DOT BELOW]
                    case 0x2092: // [LATIN SUBSCRIPT SMALL LETTER O]
                    case 0x24de: // [CIRCLED LATIN SMALL LETTER O]
                    case 0x2c7a: // [LATIN SMALL LETTER O WITH LOW RING INSIDE]
                    case 0xa74b: // [LATIN SMALL LETTER O WITH LONG STROKE OVERLAY]
                    case 0xa74d: // [LATIN SMALL LETTER O WITH LOOP]
                    case 0xff4f: // [FULLWIDTH LATIN SMALL LETTER O]
                        output[outputPos++] = L'o';
                        break;
                    case 0x0152: // [LATIN CAPITAL LIGATURE OE]
                    case 0x0276: // [LATIN LETTER SMALL CAPITAL OE]
                        output[outputPos++] = L'O';
                        output[outputPos++] = L'E';
                        break;
                    case 0xa74e: // [LATIN CAPITAL LETTER OO]
                        output[outputPos++] = L'O';
                        output[outputPos++] = L'O';
                        break;
                    case 0x0222: // [LATIN CAPITAL LETTER OU]
                    case 0x1d15: // [LATIN LETTER SMALL CAPITAL OU]
                        output[outputPos++] = L'O';
                        output[outputPos++] = L'U';
                        break;
                    case 0x24aa: // [PARENTHESIZED LATIN SMALL LETTER O]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'o';
                        output[outputPos++] = L')';
                        break;
                    case 0x0153: // [LATIN SMALL LIGATURE OE]
                    case 0x1d14: // [LATIN SMALL LETTER TURNED OE]
                        output[outputPos++] = L'o';
                        output[outputPos++] = L'e';
                        break;
                    case 0xa74f: // [LATIN SMALL LETTER OO]
                        output[outputPos++] = L'o';
                        output[outputPos++] = L'o';
                        break;
                    case 0x0223: // [LATIN SMALL LETTER OU]
                        output[outputPos++] = L'o';
                        output[outputPos++] = L'u';
                        break;
                    case 0x01a4: // [LATIN CAPITAL LETTER P WITH HOOK]
                    case 0x1d18: // [LATIN LETTER SMALL CAPITAL P]
                    case 0x1e54: // [LATIN CAPITAL LETTER P WITH ACUTE]
                    case 0x1e56: // [LATIN CAPITAL LETTER P WITH DOT ABOVE]
                    case 0x24c5: // [CIRCLED LATIN CAPITAL LETTER P]
                    case 0x2c63: // [LATIN CAPITAL LETTER P WITH STROKE]
                    case 0xa750: // [LATIN CAPITAL LETTER P WITH STROKE THROUGH DESCENDER]
                    case 0xa752: // [LATIN CAPITAL LETTER P WITH FLOURISH]
                    case 0xa754: // [LATIN CAPITAL LETTER P WITH SQUIRREL TAIL]
                    case 0xff30: // [FULLWIDTH LATIN CAPITAL LETTER P]
                        output[outputPos++] = L'P';
                        break;
                    case 0x01a5: // [LATIN SMALL LETTER P WITH HOOK]
                    case 0x1d71: // [LATIN SMALL LETTER P WITH MIDDLE TILDE]
                    case 0x1d7d: // [LATIN SMALL LETTER P WITH STROKE]
                    case 0x1d88: // [LATIN SMALL LETTER P WITH PALATAL HOOK]
                    case 0x1e55: // [LATIN SMALL LETTER P WITH ACUTE]
                    case 0x1e57: // [LATIN SMALL LETTER P WITH DOT ABOVE]
                    case 0x24df: // [CIRCLED LATIN SMALL LETTER P]
                    case 0xa751: // [LATIN SMALL LETTER P WITH STROKE THROUGH DESCENDER]
                    case 0xa753: // [LATIN SMALL LETTER P WITH FLOURISH]
                    case 0xa755: // [LATIN SMALL LETTER P WITH SQUIRREL TAIL]
                    case 0xa7fc: // [LATIN EPIGRAPHIC LETTER REVERSED P]
                    case 0xff50: // [FULLWIDTH LATIN SMALL LETTER P]
                        output[outputPos++] = L'p';
                        break;
                    case 0x24ab: // [PARENTHESIZED LATIN SMALL LETTER P]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'p';
                        output[outputPos++] = L')';
                        break;
                    case 0x024a: // [LATIN CAPITAL LETTER SMALL Q WITH HOOK TAIL]
                    case 0x24c6: // [CIRCLED LATIN CAPITAL LETTER Q]
                    case 0xa756: // [LATIN CAPITAL LETTER Q WITH STROKE THROUGH DESCENDER]
                    case 0xa758: // [LATIN CAPITAL LETTER Q WITH DIAGONAL STROKE]
                    case 0xff31: // [FULLWIDTH LATIN CAPITAL LETTER Q]
                        output[outputPos++] = L'Q';
                        break;
                    case 0x0138: // [LATIN SMALL LETTER KRA]
                    case 0x024b: // [LATIN SMALL LETTER Q WITH HOOK TAIL]
                    case 0x02a0: // [LATIN SMALL LETTER Q WITH HOOK]
                    case 0x24e0: // [CIRCLED LATIN SMALL LETTER Q]
                    case 0xa757: // [LATIN SMALL LETTER Q WITH STROKE THROUGH DESCENDER]
                    case 0xa759: // [LATIN SMALL LETTER Q WITH DIAGONAL STROKE]
                    case 0xff51: // [FULLWIDTH LATIN SMALL LETTER Q]
                        output[outputPos++] = L'q';
                        break;
                    case 0x24ac: // [PARENTHESIZED LATIN SMALL LETTER Q]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'q';
                        output[outputPos++] = L')';
                        break;
                    case 0x0239: // [LATIN SMALL LETTER QP DIGRAPH]
                        output[outputPos++] = L'q';
                        output[outputPos++] = L'p';
                        break;
                    case 0x0154: // [LATIN CAPITAL LETTER R WITH ACUTE]
                    case 0x0156: // [LATIN CAPITAL LETTER R WITH CEDILLA]
                    case 0x0158: // [LATIN CAPITAL LETTER R WITH CARON]
                    case 0x0210: // [LATIN CAPITAL LETTER R WITH DOUBLE GRAVE]
                    case 0x0212: // [LATIN CAPITAL LETTER R WITH INVERTED BREVE]
                    case 0x024c: // [LATIN CAPITAL LETTER R WITH STROKE]
                    case 0x0280: // [LATIN LETTER SMALL CAPITAL R]
                    case 0x0281: // [LATIN LETTER SMALL CAPITAL INVERTED R]
                    case 0x1d19: // [LATIN LETTER SMALL CAPITAL REVERSED R]
                    case 0x1d1a: // [LATIN LETTER SMALL CAPITAL TURNED R]
                    case 0x1e58: // [LATIN CAPITAL LETTER R WITH DOT ABOVE]
                    case 0x1e5a: // [LATIN CAPITAL LETTER R WITH DOT BELOW]
                    case 0x1e5c: // [LATIN CAPITAL LETTER R WITH DOT BELOW AND MACRON]
                    case 0x1e5e: // [LATIN CAPITAL LETTER R WITH LINE BELOW]
                    case 0x24c7: // [CIRCLED LATIN CAPITAL LETTER R]
                    case 0x2c64: // [LATIN CAPITAL LETTER R WITH TAIL]
                    case 0xa75a: // [LATIN CAPITAL LETTER R ROTUNDA]
                    case 0xa782: // [LATIN CAPITAL LETTER INSULAR R]
                    case 0xff32: // [FULLWIDTH LATIN CAPITAL LETTER R]
                        output[outputPos++] = L'R';
                        break;
                    case 0x0155: // [LATIN SMALL LETTER R WITH ACUTE]
                    case 0x0157: // [LATIN SMALL LETTER R WITH CEDILLA]
                    case 0x0159: // [LATIN SMALL LETTER R WITH CARON]
                    case 0x0211: // [LATIN SMALL LETTER R WITH DOUBLE GRAVE]
                    case 0x0213: // [LATIN SMALL LETTER R WITH INVERTED BREVE]
                    case 0x024d: // [LATIN SMALL LETTER R WITH STROKE]
                    case 0x027c: // [LATIN SMALL LETTER R WITH LONG LEG]
                    case 0x027d: // [LATIN SMALL LETTER R WITH TAIL]
                    case 0x027e: // [LATIN SMALL LETTER R WITH FISHHOOK]
                    case 0x027f: // [LATIN SMALL LETTER REVERSED R WITH FISHHOOK]
                    case 0x1d63: // [LATIN SUBSCRIPT SMALL LETTER R]
                    case 0x1d72: // [LATIN SMALL LETTER R WITH MIDDLE TILDE]
                    case 0x1d73: // [LATIN SMALL LETTER R WITH FISHHOOK AND MIDDLE TILDE]
                    case 0x1d89: // [LATIN SMALL LETTER R WITH PALATAL HOOK]
                    case 0x1e59: // [LATIN SMALL LETTER R WITH DOT ABOVE]
                    case 0x1e5b: // [LATIN SMALL LETTER R WITH DOT BELOW]
                    case 0x1e5d: // [LATIN SMALL LETTER R WITH DOT BELOW AND MACRON]
                    case 0x1e5f: // [LATIN SMALL LETTER R WITH LINE BELOW]
                    case 0x24e1: // [CIRCLED LATIN SMALL LETTER R]
                    case 0xa75b: // [LATIN SMALL LETTER R ROTUNDA]
                    case 0xa783: // [LATIN SMALL LETTER INSULAR R]
                    case 0xff52: // [FULLWIDTH LATIN SMALL LETTER R]
                        output[outputPos++] = L'r';
                        break;
                    case 0x24ad: // [PARENTHESIZED LATIN SMALL LETTER R]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'r';
                        output[outputPos++] = L')';
                        break;
                    case 0x015a: // [LATIN CAPITAL LETTER S WITH ACUTE]
                    case 0x015c: // [LATIN CAPITAL LETTER S WITH CIRCUMFLEX]
                    case 0x015e: // [LATIN CAPITAL LETTER S WITH CEDILLA]
                    case 0x0160: // [LATIN CAPITAL LETTER S WITH CARON]
                    case 0x0218: // [LATIN CAPITAL LETTER S WITH COMMA BELOW]
                    case 0x1e60: // [LATIN CAPITAL LETTER S WITH DOT ABOVE]
                    case 0x1e62: // [LATIN CAPITAL LETTER S WITH DOT BELOW]
                    case 0x1e64: // [LATIN CAPITAL LETTER S WITH ACUTE AND DOT ABOVE]
                    case 0x1e66: // [LATIN CAPITAL LETTER S WITH CARON AND DOT ABOVE]
                    case 0x1e68: // [LATIN CAPITAL LETTER S WITH DOT BELOW AND DOT ABOVE]
                    case 0x24c8: // [CIRCLED LATIN CAPITAL LETTER S]
                    case 0xa731: // [LATIN LETTER SMALL CAPITAL S]
                    case 0xa785: // [LATIN SMALL LETTER INSULAR S]
                    case 0xff33: // [FULLWIDTH LATIN CAPITAL LETTER S]
                        output[outputPos++] = L'S';
                        break;
                    case 0x015b: // [LATIN SMALL LETTER S WITH ACUTE]
                    case 0x015d: // [LATIN SMALL LETTER S WITH CIRCUMFLEX]
                    case 0x015f: // [LATIN SMALL LETTER S WITH CEDILLA]
                    case 0x0161: // [LATIN SMALL LETTER S WITH CARON]
                    case 0x017f: // [LATIN SMALL LETTER LONG S]
                    case 0x0219: // [LATIN SMALL LETTER S WITH COMMA BELOW]
                    case 0x023f: // [LATIN SMALL LETTER S WITH SWASH TAIL]
                    case 0x0282: // [LATIN SMALL LETTER S WITH HOOK]
                    case 0x1d74: // [LATIN SMALL LETTER S WITH MIDDLE TILDE]
                    case 0x1d8a: // [LATIN SMALL LETTER S WITH PALATAL HOOK]
                    case 0x1e61: // [LATIN SMALL LETTER S WITH DOT ABOVE]
                    case 0x1e63: // [LATIN SMALL LETTER S WITH DOT BELOW]
                    case 0x1e65: // [LATIN SMALL LETTER S WITH ACUTE AND DOT ABOVE]
                    case 0x1e67: // [LATIN SMALL LETTER S WITH CARON AND DOT ABOVE]
                    case 0x1e69: // [LATIN SMALL LETTER S WITH DOT BELOW AND DOT ABOVE]
                    case 0x1e9c: // [LATIN SMALL LETTER LONG S WITH DIAGONAL STROKE]
                    case 0x1e9d: // [LATIN SMALL LETTER LONG S WITH HIGH STROKE]
                    case 0x24e2: // [CIRCLED LATIN SMALL LETTER S]
                    case 0xa784: // [LATIN CAPITAL LETTER INSULAR S]
                    case 0xff53: // [FULLWIDTH LATIN SMALL LETTER S]
                        output[outputPos++] = L's';
                        break;
                    case 0x1e9e: // [LATIN CAPITAL LETTER SHARP S]
                        output[outputPos++] = L'S';
                        output[outputPos++] = L'S';
                        break;
                    case 0x24ae: // [PARENTHESIZED LATIN SMALL LETTER S]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L's';
                        output[outputPos++] = L')';
                        break;
                    case 0x00df: // [LATIN SMALL LETTER SHARP S]
                        output[outputPos++] = L's';
                        output[outputPos++] = L's';
                        break;
                    case 0xfb06: // [LATIN SMALL LIGATURE ST]
                        output[outputPos++] = L's';
                        output[outputPos++] = L't';
                        break;
                    case 0x0162: // [LATIN CAPITAL LETTER T WITH CEDILLA]
                    case 0x0164: // [LATIN CAPITAL LETTER T WITH CARON]
                    case 0x0166: // [LATIN CAPITAL LETTER T WITH STROKE]
                    case 0x01ac: // [LATIN CAPITAL LETTER T WITH HOOK]
                    case 0x01ae: // [LATIN CAPITAL LETTER T WITH RETROFLEX HOOK]
                    case 0x021a: // [LATIN CAPITAL LETTER T WITH COMMA BELOW]
                    case 0x023e: // [LATIN CAPITAL LETTER T WITH DIAGONAL STROKE]
                    case 0x1d1b: // [LATIN LETTER SMALL CAPITAL T]
                    case 0x1e6a: // [LATIN CAPITAL LETTER T WITH DOT ABOVE]
                    case 0x1e6c: // [LATIN CAPITAL LETTER T WITH DOT BELOW]
                    case 0x1e6e: // [LATIN CAPITAL LETTER T WITH LINE BELOW]
                    case 0x1e70: // [LATIN CAPITAL LETTER T WITH CIRCUMFLEX BELOW]
                    case 0x24c9: // [CIRCLED LATIN CAPITAL LETTER T]
                    case 0xa786: // [LATIN CAPITAL LETTER INSULAR T]
                    case 0xff34: // [FULLWIDTH LATIN CAPITAL LETTER T]
                        output[outputPos++] = L'T';
                        break;
                    case 0x0163: // [LATIN SMALL LETTER T WITH CEDILLA]
                    case 0x0165: // [LATIN SMALL LETTER T WITH CARON]
                    case 0x0167: // [LATIN SMALL LETTER T WITH STROKE]
                    case 0x01ab: // [LATIN SMALL LETTER T WITH PALATAL HOOK]
                    case 0x01ad: // [LATIN SMALL LETTER T WITH HOOK]
                    case 0x021b: // [LATIN SMALL LETTER T WITH COMMA BELOW]
                    case 0x0236: // [LATIN SMALL LETTER T WITH CURL]
                    case 0x0287: // [LATIN SMALL LETTER TURNED T]
                    case 0x0288: // [LATIN SMALL LETTER T WITH RETROFLEX HOOK]
                    case 0x1d75: // [LATIN SMALL LETTER T WITH MIDDLE TILDE]
                    case 0x1e6b: // [LATIN SMALL LETTER T WITH DOT ABOVE]
                    case 0x1e6d: // [LATIN SMALL LETTER T WITH DOT BELOW]
                    case 0x1e6f: // [LATIN SMALL LETTER T WITH LINE BELOW]
                    case 0x1e71: // [LATIN SMALL LETTER T WITH CIRCUMFLEX BELOW]
                    case 0x1e97: // [LATIN SMALL LETTER T WITH DIAERESIS]
                    case 0x24e3: // [CIRCLED LATIN SMALL LETTER T]
                    case 0x2c66: // [LATIN SMALL LETTER T WITH DIAGONAL STROKE]
                    case 0xff54: // [FULLWIDTH LATIN SMALL LETTER T]
                        output[outputPos++] = L't';
                        break;
                    case 0x00de: // [LATIN CAPITAL LETTER THORN]
                    case 0xa766: // [LATIN CAPITAL LETTER THORN WITH STROKE THROUGH DESCENDER]
                        output[outputPos++] = L'T';
                        output[outputPos++] = L'H';
                        break;
                    case 0xa728: // [LATIN CAPITAL LETTER TZ]
                        output[outputPos++] = L'T';
                        output[outputPos++] = L'Z';
                        break;
                    case 0x24af: // [PARENTHESIZED LATIN SMALL LETTER T]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L't';
                        output[outputPos++] = L')';
                        break;
                    case 0x02a8: // [LATIN SMALL LETTER TC DIGRAPH WITH CURL]
                        output[outputPos++] = L't';
                        output[outputPos++] = L'c';
                        break;
                    case 0x00fe: // [LATIN SMALL LETTER THORN]
                    case 0x1d7a: // [LATIN SMALL LETTER TH WITH STRIKETHROUGH]
                    case 0xa767: // [LATIN SMALL LETTER THORN WITH STROKE THROUGH DESCENDER]
                        output[outputPos++] = L't';
                        output[outputPos++] = L'h';
                        break;
                    case 0x02a6: // [LATIN SMALL LETTER TS DIGRAPH]
                        output[outputPos++] = L't';
                        output[outputPos++] = L's';
                        break;
                    case 0xa729: // [LATIN SMALL LETTER TZ]
                        output[outputPos++] = L't';
                        output[outputPos++] = L'z';
                        break;
                    case 0x00d9: // [LATIN CAPITAL LETTER U WITH GRAVE]
                    case 0x00da: // [LATIN CAPITAL LETTER U WITH ACUTE]
                    case 0x00db: // [LATIN CAPITAL LETTER U WITH CIRCUMFLEX]
                    case 0x00dc: // [LATIN CAPITAL LETTER U WITH DIAERESIS]
                    case 0x0168: // [LATIN CAPITAL LETTER U WITH TILDE]
                    case 0x016a: // [LATIN CAPITAL LETTER U WITH MACRON]
                    case 0x016c: // [LATIN CAPITAL LETTER U WITH BREVE]
                    case 0x016e: // [LATIN CAPITAL LETTER U WITH RING ABOVE]
                    case 0x0170: // [LATIN CAPITAL LETTER U WITH DOUBLE ACUTE]
                    case 0x0172: // [LATIN CAPITAL LETTER U WITH OGONEK]
                    case 0x01af: // [LATIN CAPITAL LETTER U WITH HORN]
                    case 0x01d3: // [LATIN CAPITAL LETTER U WITH CARON]
                    case 0x01d5: // [LATIN CAPITAL LETTER U WITH DIAERESIS AND MACRON]
                    case 0x01d7: // [LATIN CAPITAL LETTER U WITH DIAERESIS AND ACUTE]
                    case 0x01d9: // [LATIN CAPITAL LETTER U WITH DIAERESIS AND CARON]
                    case 0x01db: // [LATIN CAPITAL LETTER U WITH DIAERESIS AND GRAVE]
                    case 0x0214: // [LATIN CAPITAL LETTER U WITH DOUBLE GRAVE]
                    case 0x0216: // [LATIN CAPITAL LETTER U WITH INVERTED BREVE]
                    case 0x0244: // [LATIN CAPITAL LETTER U BAR]
                    case 0x1d1c: // [LATIN LETTER SMALL CAPITAL U]
                    case 0x1d7e: // [LATIN SMALL CAPITAL LETTER U WITH STROKE]
                    case 0x1e72: // [LATIN CAPITAL LETTER U WITH DIAERESIS BELOW]
                    case 0x1e74: // [LATIN CAPITAL LETTER U WITH TILDE BELOW]
                    case 0x1e76: // [LATIN CAPITAL LETTER U WITH CIRCUMFLEX BELOW]
                    case 0x1e78: // [LATIN CAPITAL LETTER U WITH TILDE AND ACUTE]
                    case 0x1e7a: // [LATIN CAPITAL LETTER U WITH MACRON AND DIAERESIS]
                    case 0x1ee4: // [LATIN CAPITAL LETTER U WITH DOT BELOW]
                    case 0x1ee6: // [LATIN CAPITAL LETTER U WITH HOOK ABOVE]
                    case 0x1ee8: // [LATIN CAPITAL LETTER U WITH HORN AND ACUTE]
                    case 0x1eea: // [LATIN CAPITAL LETTER U WITH HORN AND GRAVE]
                    case 0x1eec: // [LATIN CAPITAL LETTER U WITH HORN AND HOOK ABOVE]
                    case 0x1eee: // [LATIN CAPITAL LETTER U WITH HORN AND TILDE]
                    case 0x1ef0: // [LATIN CAPITAL LETTER U WITH HORN AND DOT BELOW]
                    case 0x24ca: // [CIRCLED LATIN CAPITAL LETTER U]
                    case 0xff35: // [FULLWIDTH LATIN CAPITAL LETTER U]
                        output[outputPos++] = L'U';
                        break;
                    case 0x00f9: // [LATIN SMALL LETTER U WITH GRAVE]
                    case 0x00fa: // [LATIN SMALL LETTER U WITH ACUTE]
                    case 0x00fb: // [LATIN SMALL LETTER U WITH CIRCUMFLEX]
                    case 0x00fc: // [LATIN SMALL LETTER U WITH DIAERESIS]
                    case 0x0169: // [LATIN SMALL LETTER U WITH TILDE]
                    case 0x016b: // [LATIN SMALL LETTER U WITH MACRON]
                    case 0x016d: // [LATIN SMALL LETTER U WITH BREVE]
                    case 0x016f: // [LATIN SMALL LETTER U WITH RING ABOVE]
                    case 0x0171: // [LATIN SMALL LETTER U WITH DOUBLE ACUTE]
                    case 0x0173: // [LATIN SMALL LETTER U WITH OGONEK]
                    case 0x01b0: // [LATIN SMALL LETTER U WITH HORN]
                    case 0x01d4: // [LATIN SMALL LETTER U WITH CARON]
                    case 0x01d6: // [LATIN SMALL LETTER U WITH DIAERESIS AND MACRON]
                    case 0x01d8: // [LATIN SMALL LETTER U WITH DIAERESIS AND ACUTE]
                    case 0x01da: // [LATIN SMALL LETTER U WITH DIAERESIS AND CARON]
                    case 0x01dc: // [LATIN SMALL LETTER U WITH DIAERESIS AND GRAVE]
                    case 0x0215: // [LATIN SMALL LETTER U WITH DOUBLE GRAVE]
                    case 0x0217: // [LATIN SMALL LETTER U WITH INVERTED BREVE]
                    case 0x0289: // [LATIN SMALL LETTER U BAR]
                    case 0x1d64: // [LATIN SUBSCRIPT SMALL LETTER U]
                    case 0x1d99: // [LATIN SMALL LETTER U WITH RETROFLEX HOOK]
                    case 0x1e73: // [LATIN SMALL LETTER U WITH DIAERESIS BELOW]
                    case 0x1e75: // [LATIN SMALL LETTER U WITH TILDE BELOW]
                    case 0x1e77: // [LATIN SMALL LETTER U WITH CIRCUMFLEX BELOW]
                    case 0x1e79: // [LATIN SMALL LETTER U WITH TILDE AND ACUTE]
                    case 0x1e7b: // [LATIN SMALL LETTER U WITH MACRON AND DIAERESIS]
                    case 0x1ee5: // [LATIN SMALL LETTER U WITH DOT BELOW]
                    case 0x1ee7: // [LATIN SMALL LETTER U WITH HOOK ABOVE]
                    case 0x1ee9: // [LATIN SMALL LETTER U WITH HORN AND ACUTE]
                    case 0x1eeb: // [LATIN SMALL LETTER U WITH HORN AND GRAVE]
                    case 0x1eed: // [LATIN SMALL LETTER U WITH HORN AND HOOK ABOVE]
                    case 0x1eef: // [LATIN SMALL LETTER U WITH HORN AND TILDE]
                    case 0x1ef1: // [LATIN SMALL LETTER U WITH HORN AND DOT BELOW]
                    case 0x24e4: // [CIRCLED LATIN SMALL LETTER U]
                    case 0xff55: // [FULLWIDTH LATIN SMALL LETTER U]
                        output[outputPos++] = L'u';
                        break;
                    case 0x24b0: // [PARENTHESIZED LATIN SMALL LETTER U]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'u';
                        output[outputPos++] = L')';
                        break;
                    case 0x1d6b: // [LATIN SMALL LETTER UE]
                        output[outputPos++] = L'u';
                        output[outputPos++] = L'e';
                        break;
                    case 0x01b2: // [LATIN CAPITAL LETTER V WITH HOOK]
                    case 0x0245: // [LATIN CAPITAL LETTER TURNED V]
                    case 0x1d20: // [LATIN LETTER SMALL CAPITAL V]
                    case 0x1e7c: // [LATIN CAPITAL LETTER V WITH TILDE]
                    case 0x1e7e: // [LATIN CAPITAL LETTER V WITH DOT BELOW]
                    case 0x1efc: // [LATIN CAPITAL LETTER MIDDLE-WELSH V]
                    case 0x24cb: // [CIRCLED LATIN CAPITAL LETTER V]
                    case 0xa75e: // [LATIN CAPITAL LETTER V WITH DIAGONAL STROKE]
                    case 0xa768: // [LATIN CAPITAL LETTER VEND]
                    case 0xff36: // [FULLWIDTH LATIN CAPITAL LETTER V]
                        output[outputPos++] = L'V';
                        break;
                    case 0x028b: // [LATIN SMALL LETTER V WITH HOOK]
                    case 0x028c: // [LATIN SMALL LETTER TURNED V]
                    case 0x1d65: // [LATIN SUBSCRIPT SMALL LETTER V]
                    case 0x1d8c: // [LATIN SMALL LETTER V WITH PALATAL HOOK]
                    case 0x1e7d: // [LATIN SMALL LETTER V WITH TILDE]
                    case 0x1e7f: // [LATIN SMALL LETTER V WITH DOT BELOW]
                    case 0x24e5: // [CIRCLED LATIN SMALL LETTER V]
                    case 0x2c71: // [LATIN SMALL LETTER V WITH RIGHT HOOK]
                    case 0x2c74: // [LATIN SMALL LETTER V WITH CURL]
                    case 0xa75f: // [LATIN SMALL LETTER V WITH DIAGONAL STROKE]
                    case 0xff56: // [FULLWIDTH LATIN SMALL LETTER V]
                        output[outputPos++] = L'v';
                        break;
                    case 0xa760: // [LATIN CAPITAL LETTER VY]
                        output[outputPos++] = L'V';
                        output[outputPos++] = L'Y';
                        break;
                    case 0x24b1: // [PARENTHESIZED LATIN SMALL LETTER V]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'v';
                        output[outputPos++] = L')';
                        break;
                    case 0xa761: // [LATIN SMALL LETTER VY]
                        output[outputPos++] = L'v';
                        output[outputPos++] = L'y';
                        break;
                    case 0x0174: // [LATIN CAPITAL LETTER W WITH CIRCUMFLEX]
                    case 0x01f7: // [LATIN CAPITAL LETTER WYNN]
                    case 0x1d21: // [LATIN LETTER SMALL CAPITAL W]
                    case 0x1e80: // [LATIN CAPITAL LETTER W WITH GRAVE]
                    case 0x1e82: // [LATIN CAPITAL LETTER W WITH ACUTE]
                    case 0x1e84: // [LATIN CAPITAL LETTER W WITH DIAERESIS]
                    case 0x1e86: // [LATIN CAPITAL LETTER W WITH DOT ABOVE]
                    case 0x1e88: // [LATIN CAPITAL LETTER W WITH DOT BELOW]
                    case 0x24cc: // [CIRCLED LATIN CAPITAL LETTER W]
                    case 0x2c72: // [LATIN CAPITAL LETTER W WITH HOOK]
                    case 0xff37: // [FULLWIDTH LATIN CAPITAL LETTER W]
                        output[outputPos++] = L'W';
                        break;
                    case 0x0175: // [LATIN SMALL LETTER W WITH CIRCUMFLEX]
                    case 0x01bf: // [LATIN LETTER WYNN]
                    case 0x028d: // [LATIN SMALL LETTER TURNED W]
                    case 0x1e81: // [LATIN SMALL LETTER W WITH GRAVE]
                    case 0x1e83: // [LATIN SMALL LETTER W WITH ACUTE]
                    case 0x1e85: // [LATIN SMALL LETTER W WITH DIAERESIS]
                    case 0x1e87: // [LATIN SMALL LETTER W WITH DOT ABOVE]
                    case 0x1e89: // [LATIN SMALL LETTER W WITH DOT BELOW]
                    case 0x1e98: // [LATIN SMALL LETTER W WITH RING ABOVE]
                    case 0x24e6: // [CIRCLED LATIN SMALL LETTER W]
                    case 0x2c73: // [LATIN SMALL LETTER W WITH HOOK]
                    case 0xff57: // [FULLWIDTH LATIN SMALL LETTER W]
                        output[outputPos++] = L'w';
                        break;
                    case 0x24b2: // [PARENTHESIZED LATIN SMALL LETTER W]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'w';
                        output[outputPos++] = L')';
                        break;
                    case 0x1e8a: // [LATIN CAPITAL LETTER X WITH DOT ABOVE]
                    case 0x1e8c: // [LATIN CAPITAL LETTER X WITH DIAERESIS]
                    case 0x24cd: // [CIRCLED LATIN CAPITAL LETTER X]
                    case 0xff38: // [FULLWIDTH LATIN CAPITAL LETTER X]
                        output[outputPos++] = L'X';
                        break;
                    case 0x1d8d: // [LATIN SMALL LETTER X WITH PALATAL HOOK]
                    case 0x1e8b: // [LATIN SMALL LETTER X WITH DOT ABOVE]
                    case 0x1e8d: // [LATIN SMALL LETTER X WITH DIAERESIS]
                    case 0x2093: // [LATIN SUBSCRIPT SMALL LETTER X]
                    case 0x24e7: // [CIRCLED LATIN SMALL LETTER X]
                    case 0xff58: // [FULLWIDTH LATIN SMALL LETTER X]
                        output[outputPos++] = L'x';
                        break;
                    case 0x24b3: // [PARENTHESIZED LATIN SMALL LETTER X]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'x';
                        output[outputPos++] = L')';
                        break;
                    case 0x00dd: // [LATIN CAPITAL LETTER Y WITH ACUTE]
                    case 0x0176: // [LATIN CAPITAL LETTER Y WITH CIRCUMFLEX]
                    case 0x0178: // [LATIN CAPITAL LETTER Y WITH DIAERESIS]
                    case 0x01b3: // [LATIN CAPITAL LETTER Y WITH HOOK]
                    case 0x0232: // [LATIN CAPITAL LETTER Y WITH MACRON]
                    case 0x024e: // [LATIN CAPITAL LETTER Y WITH STROKE]
                    case 0x028f: // [LATIN LETTER SMALL CAPITAL Y]
                    case 0x1e8e: // [LATIN CAPITAL LETTER Y WITH DOT ABOVE]
                    case 0x1ef2: // [LATIN CAPITAL LETTER Y WITH GRAVE]
                    case 0x1ef4: // [LATIN CAPITAL LETTER Y WITH DOT BELOW]
                    case 0x1ef6: // [LATIN CAPITAL LETTER Y WITH HOOK ABOVE]
                    case 0x1ef8: // [LATIN CAPITAL LETTER Y WITH TILDE]
                    case 0x1efe: // [LATIN CAPITAL LETTER Y WITH LOOP]
                    case 0x24ce: // [CIRCLED LATIN CAPITAL LETTER Y]
                    case 0xff39: // [FULLWIDTH LATIN CAPITAL LETTER Y]
                        output[outputPos++] = L'Y';
                        break;
                    case 0x00fd: // [LATIN SMALL LETTER Y WITH ACUTE]
                    case 0x00ff: // [LATIN SMALL LETTER Y WITH DIAERESIS]
                    case 0x0177: // [LATIN SMALL LETTER Y WITH CIRCUMFLEX]
                    case 0x01b4: // [LATIN SMALL LETTER Y WITH HOOK]
                    case 0x0233: // [LATIN SMALL LETTER Y WITH MACRON]
                    case 0x024f: // [LATIN SMALL LETTER Y WITH STROKE]
                    case 0x028e: // [LATIN SMALL LETTER TURNED Y]
                    case 0x1e8f: // [LATIN SMALL LETTER Y WITH DOT ABOVE]
                    case 0x1e99: // [LATIN SMALL LETTER Y WITH RING ABOVE]
                    case 0x1ef3: // [LATIN SMALL LETTER Y WITH GRAVE]
                    case 0x1ef5: // [LATIN SMALL LETTER Y WITH DOT BELOW]
                    case 0x1ef7: // [LATIN SMALL LETTER Y WITH HOOK ABOVE]
                    case 0x1ef9: // [LATIN SMALL LETTER Y WITH TILDE]
                    case 0x1eff: // [LATIN SMALL LETTER Y WITH LOOP]
                    case 0x24e8: // [CIRCLED LATIN SMALL LETTER Y]
                    case 0xff59: // [FULLWIDTH LATIN SMALL LETTER Y]
                        output[outputPos++] = L'y';
                        break;
                    case 0x24b4: // [PARENTHESIZED LATIN SMALL LETTER Y]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'y';
                        output[outputPos++] = L')';
                        break;
                    case 0x0179: // [LATIN CAPITAL LETTER Z WITH ACUTE]
                    case 0x017b: // [LATIN CAPITAL LETTER Z WITH DOT ABOVE]
                    case 0x017d: // [LATIN CAPITAL LETTER Z WITH CARON]
                    case 0x01b5: // [LATIN CAPITAL LETTER Z WITH STROKE]
                    case 0x021c: // [LATIN CAPITAL LETTER YOGH]
                    case 0x0224: // [LATIN CAPITAL LETTER Z WITH HOOK]
                    case 0x1d22: // [LATIN LETTER SMALL CAPITAL Z]
                    case 0x1e90: // [LATIN CAPITAL LETTER Z WITH CIRCUMFLEX]
                    case 0x1e92: // [LATIN CAPITAL LETTER Z WITH DOT BELOW]
                    case 0x1e94: // [LATIN CAPITAL LETTER Z WITH LINE BELOW]
                    case 0x24cf: // [CIRCLED LATIN CAPITAL LETTER Z]
                    case 0x2c6b: // [LATIN CAPITAL LETTER Z WITH DESCENDER]
                    case 0xa762: // [LATIN CAPITAL LETTER VISIGOTHIC Z]
                    case 0xff3a: // [FULLWIDTH LATIN CAPITAL LETTER Z]
                        output[outputPos++] = L'Z';
                        break;
                    case 0x017a: // [LATIN SMALL LETTER Z WITH ACUTE]
                    case 0x017c: // [LATIN SMALL LETTER Z WITH DOT ABOVE]
                    case 0x017e: // [LATIN SMALL LETTER Z WITH CARON]
                    case 0x01b6: // [LATIN SMALL LETTER Z WITH STROKE]
                    case 0x021d: // [LATIN SMALL LETTER YOGH]
                    case 0x0225: // [LATIN SMALL LETTER Z WITH HOOK]
                    case 0x0240: // [LATIN SMALL LETTER Z WITH SWASH TAIL]
                    case 0x0290: // [LATIN SMALL LETTER Z WITH RETROFLEX HOOK]
                    case 0x0291: // [LATIN SMALL LETTER Z WITH CURL]
                    case 0x1d76: // [LATIN SMALL LETTER Z WITH MIDDLE TILDE]
                    case 0x1d8e: // [LATIN SMALL LETTER Z WITH PALATAL HOOK]
                    case 0x1e91: // [LATIN SMALL LETTER Z WITH CIRCUMFLEX]
                    case 0x1e93: // [LATIN SMALL LETTER Z WITH DOT BELOW]
                    case 0x1e95: // [LATIN SMALL LETTER Z WITH LINE BELOW]
                    case 0x24e9: // [CIRCLED LATIN SMALL LETTER Z]
                    case 0x2c6c: // [LATIN SMALL LETTER Z WITH DESCENDER]
                    case 0xa763: // [LATIN SMALL LETTER VISIGOTHIC Z]
                    case 0xff5a: // [FULLWIDTH LATIN SMALL LETTER Z]
                        output[outputPos++] = L'z';
                        break;
                    case 0x24b5: // [PARENTHESIZED LATIN SMALL LETTER Z]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'z';
                        output[outputPos++] = L')';
                        break;
                    case 0x2070: // [SUPERSCRIPT ZERO]
                    case 0x2080: // [SUBSCRIPT ZERO]
                    case 0x24ea: // [CIRCLED DIGIT ZERO]
                    case 0x24ff: // [NEGATIVE CIRCLED DIGIT ZERO]
                    case 0xff10: // [FULLWIDTH DIGIT ZERO]
                        output[outputPos++] = L'0';
                        break;
                    case 0x00b9: // [SUPERSCRIPT ONE]
                    case 0x2081: // [SUBSCRIPT ONE]
                    case 0x2460: // [CIRCLED DIGIT ONE]
                    case 0x24f5: // [DOUBLE CIRCLED DIGIT ONE]
                    case 0x2776: // [DINGBAT NEGATIVE CIRCLED DIGIT ONE]
                    case 0x2780: // [DINGBAT CIRCLED SANS-SERIF DIGIT ONE]
                    case 0x278a: // [DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT ONE]
                    case 0xff11: // [FULLWIDTH DIGIT ONE]
                        output[outputPos++] = L'1';
                        break;
                    case 0x2488: // [DIGIT ONE FULL STOP]
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'.';
                        break;
                    case 0x2474: // [PARENTHESIZED DIGIT ONE]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'1';
                        output[outputPos++] = L')';
                        break;
                    case 0x00b2: // [SUPERSCRIPT TWO]
                    case 0x2082: // [SUBSCRIPT TWO]
                    case 0x2461: // [CIRCLED DIGIT TWO]
                    case 0x24f6: // [DOUBLE CIRCLED DIGIT TWO]
                    case 0x2777: // [DINGBAT NEGATIVE CIRCLED DIGIT TWO]
                    case 0x2781: // [DINGBAT CIRCLED SANS-SERIF DIGIT TWO]
                    case 0x278b: // [DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT TWO]
                    case 0xff12: // [FULLWIDTH DIGIT TWO]
                        output[outputPos++] = L'2';
                        break;
                    case 0x2489: // [DIGIT TWO FULL STOP]
                        output[outputPos++] = L'2';
                        output[outputPos++] = L'.';
                        break;
                    case 0x2475: // [PARENTHESIZED DIGIT TWO]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'2';
                        output[outputPos++] = L')';
                        break;
                    case 0x00b3: // [SUPERSCRIPT THREE]
                    case 0x2083: // [SUBSCRIPT THREE]
                    case 0x2462: // [CIRCLED DIGIT THREE]
                    case 0x24f7: // [DOUBLE CIRCLED DIGIT THREE]
                    case 0x2778: // [DINGBAT NEGATIVE CIRCLED DIGIT THREE]
                    case 0x2782: // [DINGBAT CIRCLED SANS-SERIF DIGIT THREE]
                    case 0x278c: // [DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT THREE]
                    case 0xff13: // [FULLWIDTH DIGIT THREE]
                        output[outputPos++] = L'3';
                        break;
                    case 0x248a: // [DIGIT THREE FULL STOP]
                        output[outputPos++] = L'3';
                        output[outputPos++] = L'.';
                        break;
                    case 0x2476: // [PARENTHESIZED DIGIT THREE]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'3';
                        output[outputPos++] = L')';
                        break;
                    case 0x2074: // [SUPERSCRIPT FOUR]
                    case 0x2084: // [SUBSCRIPT FOUR]
                    case 0x2463: // [CIRCLED DIGIT FOUR]
                    case 0x24f8: // [DOUBLE CIRCLED DIGIT FOUR]
                    case 0x2779: // [DINGBAT NEGATIVE CIRCLED DIGIT FOUR]
                    case 0x2783: // [DINGBAT CIRCLED SANS-SERIF DIGIT FOUR]
                    case 0x278d: // [DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT FOUR]
                    case 0xff14: // [FULLWIDTH DIGIT FOUR]
                        output[outputPos++] = L'4';
                        break;
                    case 0x248b: // [DIGIT FOUR FULL STOP]
                        output[outputPos++] = L'4';
                        output[outputPos++] = L'.';
                        break;
                    case 0x2477: // [PARENTHESIZED DIGIT FOUR]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'4';
                        output[outputPos++] = L')';
                        break;
                    case 0x2075: // [SUPERSCRIPT FIVE]
                    case 0x2085: // [SUBSCRIPT FIVE]
                    case 0x2464: // [CIRCLED DIGIT FIVE]
                    case 0x24f9: // [DOUBLE CIRCLED DIGIT FIVE]
                    case 0x277a: // [DINGBAT NEGATIVE CIRCLED DIGIT FIVE]
                    case 0x2784: // [DINGBAT CIRCLED SANS-SERIF DIGIT FIVE]
                    case 0x278e: // [DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT FIVE]
                    case 0xff15: // [FULLWIDTH DIGIT FIVE]
                        output[outputPos++] = L'5';
                        break;
                    case 0x248c: // [DIGIT FIVE FULL STOP]
                        output[outputPos++] = L'5';
                        output[outputPos++] = L'.';
                        break;
                    case 0x2478: // [PARENTHESIZED DIGIT FIVE]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'5';
                        output[outputPos++] = L')';
                        break;
                    case 0x2076: // [SUPERSCRIPT SIX]
                    case 0x2086: // [SUBSCRIPT SIX]
                    case 0x2465: // [CIRCLED DIGIT SIX]
                    case 0x24fa: // [DOUBLE CIRCLED DIGIT SIX]
                    case 0x277b: // [DINGBAT NEGATIVE CIRCLED DIGIT SIX]
                    case 0x2785: // [DINGBAT CIRCLED SANS-SERIF DIGIT SIX]
                    case 0x278f: // [DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT SIX]
                    case 0xff16: // [FULLWIDTH DIGIT SIX]
                        output[outputPos++] = L'6';
                        break;
                    case 0x248d: // [DIGIT SIX FULL STOP]
                        output[outputPos++] = L'6';
                        output[outputPos++] = L'.';
                        break;
                    case 0x2479: // [PARENTHESIZED DIGIT SIX]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'6';
                        output[outputPos++] = L')';
                        break;
                    case 0x2077: // [SUPERSCRIPT SEVEN]
                    case 0x2087: // [SUBSCRIPT SEVEN]
                    case 0x2466: // [CIRCLED DIGIT SEVEN]
                    case 0x24fb: // [DOUBLE CIRCLED DIGIT SEVEN]
                    case 0x277c: // [DINGBAT NEGATIVE CIRCLED DIGIT SEVEN]
                    case 0x2786: // [DINGBAT CIRCLED SANS-SERIF DIGIT SEVEN]
                    case 0x2790: // [DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT SEVEN]
                    case 0xff17: // [FULLWIDTH DIGIT SEVEN]
                        output[outputPos++] = L'7';
                        break;
                    case 0x248e: // [DIGIT SEVEN FULL STOP]
                        output[outputPos++] = L'7';
                        output[outputPos++] = L'.';
                        break;
                    case 0x247a: // [PARENTHESIZED DIGIT SEVEN]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'7';
                        output[outputPos++] = L')';
                        break;
                    case 0x2078: // [SUPERSCRIPT EIGHT]
                    case 0x2088: // [SUBSCRIPT EIGHT]
                    case 0x2467: // [CIRCLED DIGIT EIGHT]
                    case 0x24fc: // [DOUBLE CIRCLED DIGIT EIGHT]
                    case 0x277d: // [DINGBAT NEGATIVE CIRCLED DIGIT EIGHT]
                    case 0x2787: // [DINGBAT CIRCLED SANS-SERIF DIGIT EIGHT]
                    case 0x2791: // [DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT EIGHT]
                    case 0xff18: // [FULLWIDTH DIGIT EIGHT]
                        output[outputPos++] = L'8';
                        break;
                    case 0x248f: // [DIGIT EIGHT FULL STOP]
                        output[outputPos++] = L'8';
                        output[outputPos++] = L'.';
                        break;
                    case 0x247b: // [PARENTHESIZED DIGIT EIGHT]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'8';
                        output[outputPos++] = L')';
                        break;
                    case 0x2079: // [SUPERSCRIPT NINE]
                    case 0x2089: // [SUBSCRIPT NINE]
                    case 0x2468: // [CIRCLED DIGIT NINE]
                    case 0x24fd: // [DOUBLE CIRCLED DIGIT NINE]
                    case 0x277e: // [DINGBAT NEGATIVE CIRCLED DIGIT NINE]
                    case 0x2788: // [DINGBAT CIRCLED SANS-SERIF DIGIT NINE]
                    case 0x2792: // [DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT NINE]
                    case 0xff19: // [FULLWIDTH DIGIT NINE]
                        output[outputPos++] = L'9';
                        break;
                    case 0x2490: // [DIGIT NINE FULL STOP]
                        output[outputPos++] = L'9';
                        output[outputPos++] = L'.';
                        break;
                    case 0x247c: // [PARENTHESIZED DIGIT NINE]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'9';
                        output[outputPos++] = L')';
                        break;
                    case 0x2469: // [CIRCLED NUMBER TEN]
                    case 0x24fe: // [DOUBLE CIRCLED NUMBER TEN]
                    case 0x277f: // [DINGBAT NEGATIVE CIRCLED NUMBER TEN]
                    case 0x2789: // [DINGBAT CIRCLED SANS-SERIF NUMBER TEN]
                    case 0x2793: // [DINGBAT NEGATIVE CIRCLED SANS-SERIF NUMBER TEN]
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'0';
                        break;
                    case 0x2491: // [NUMBER TEN FULL STOP]
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'0';
                        output[outputPos++] = L'.';
                        break;
                    case 0x247d: // [PARENTHESIZED NUMBER TEN]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'0';
                        output[outputPos++] = L')';
                        break;
                    case 0x246a: // [CIRCLED NUMBER ELEVEN]
                    case 0x24eb: // [NEGATIVE CIRCLED NUMBER ELEVEN]
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'1';
                        break;
                    case 0x2492: // [NUMBER ELEVEN FULL STOP]
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'.';
                        break;
                    case 0x247e: // [PARENTHESIZED NUMBER ELEVEN]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'1';
                        output[outputPos++] = L')';
                        break;
                    case 0x246b: // [CIRCLED NUMBER TWELVE]
                    case 0x24ec: // [NEGATIVE CIRCLED NUMBER TWELVE]
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'2';
                        break;
                    case 0x2493: // [NUMBER TWELVE FULL STOP]
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'2';
                        output[outputPos++] = L'.';
                        break;
                    case 0x247f: // [PARENTHESIZED NUMBER TWELVE]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'2';
                        output[outputPos++] = L')';
                        break;
                    case 0x246c: // [CIRCLED NUMBER THIRTEEN]
                    case 0x24ed: // [NEGATIVE CIRCLED NUMBER THIRTEEN]
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'3';
                        break;
                    case 0x2494: // [NUMBER THIRTEEN FULL STOP]
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'3';
                        output[outputPos++] = L'.';
                        break;
                    case 0x2480: // [PARENTHESIZED NUMBER THIRTEEN]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'3';
                        output[outputPos++] = L')';
                        break;
                    case 0x246d: // [CIRCLED NUMBER FOURTEEN]
                    case 0x24ee: // [NEGATIVE CIRCLED NUMBER FOURTEEN]
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'4';
                        break;
                    case 0x2495: // [NUMBER FOURTEEN FULL STOP]
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'4';
                        output[outputPos++] = L'.';
                        break;
                    case 0x2481: // [PARENTHESIZED NUMBER FOURTEEN]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'4';
                        output[outputPos++] = L')';
                        break;
                    case 0x246e: // [CIRCLED NUMBER FIFTEEN]
                    case 0x24ef: // [NEGATIVE CIRCLED NUMBER FIFTEEN]
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'5';
                        break;
                    case 0x2496: // [NUMBER FIFTEEN FULL STOP]
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'5';
                        output[outputPos++] = L'.';
                        break;
                    case 0x2482: // [PARENTHESIZED NUMBER FIFTEEN]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'5';
                        output[outputPos++] = L')';
                        break;
                    case 0x246f: // [CIRCLED NUMBER SIXTEEN]
                    case 0x24f0: // [NEGATIVE CIRCLED NUMBER SIXTEEN]
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'6';
                        break;
                    case 0x2497: // [NUMBER SIXTEEN FULL STOP]
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'6';
                        output[outputPos++] = L'.';
                        break;
                    case 0x2483: // [PARENTHESIZED NUMBER SIXTEEN]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'6';
                        output[outputPos++] = L')';
                        break;
                    case 0x2470: // [CIRCLED NUMBER SEVENTEEN]
                    case 0x24f1: // [NEGATIVE CIRCLED NUMBER SEVENTEEN]
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'7';
                        break;
                    case 0x2498: // [NUMBER SEVENTEEN FULL STOP]
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'7';
                        output[outputPos++] = L'.';
                        break;
                    case 0x2484: // [PARENTHESIZED NUMBER SEVENTEEN]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'7';
                        output[outputPos++] = L')';
                        break;
                    case 0x2471: // [CIRCLED NUMBER EIGHTEEN]
                    case 0x24f2: // [NEGATIVE CIRCLED NUMBER EIGHTEEN]
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'8';
                        break;
                    case 0x2499: // [NUMBER EIGHTEEN FULL STOP]
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'8';
                        output[outputPos++] = L'.';
                        break;
                    case 0x2485: // [PARENTHESIZED NUMBER EIGHTEEN]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'8';
                        output[outputPos++] = L')';
                        break;
                    case 0x2472: // [CIRCLED NUMBER NINETEEN]
                    case 0x24f3: // [NEGATIVE CIRCLED NUMBER NINETEEN]
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'9';
                        break;
                    case 0x249a: // [NUMBER NINETEEN FULL STOP]
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'9';
                        output[outputPos++] = L'.';
                        break;
                    case 0x2486: // [PARENTHESIZED NUMBER NINETEEN]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'1';
                        output[outputPos++] = L'9';
                        output[outputPos++] = L')';
                        break;
                    case 0x2473: // [CIRCLED NUMBER TWENTY]
                    case 0x24f4: // [NEGATIVE CIRCLED NUMBER TWENTY]
                        output[outputPos++] = L'2';
                        output[outputPos++] = L'0';
                        break;
                    case 0x249b: // [NUMBER TWENTY FULL STOP]
                        output[outputPos++] = L'2';
                        output[outputPos++] = L'0';
                        output[outputPos++] = L'.';
                        break;
                    case 0x2487: // [PARENTHESIZED NUMBER TWENTY]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'2';
                        output[outputPos++] = L'0';
                        output[outputPos++] = L')';
                        break;
                    case 0x00ab: // [LEFT-POINTING DOUBLE ANGLE QUOTATION MARK]
                    case 0x00bb: // [RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK]
                    case 0x201c: // [LEFT DOUBLE QUOTATION MARK]
                    case 0x201d: // [RIGHT DOUBLE QUOTATION MARK]
                    case 0x201e: // [DOUBLE LOW-9 QUOTATION MARK]
                    case 0x2033: // [DOUBLE PRIME]
                    case 0x2036: // [REVERSED DOUBLE PRIME]
                    case 0x275d: // [HEAVY DOUBLE TURNED COMMA QUOTATION MARK ORNAMENT]
                    case 0x275e: // [HEAVY DOUBLE COMMA QUOTATION MARK ORNAMENT]
                    case 0x276e: // [HEAVY LEFT-POINTING ANGLE QUOTATION MARK ORNAMENT]
                    case 0x276f: // [HEAVY RIGHT-POINTING ANGLE QUOTATION MARK ORNAMENT]
                    case 0xff02: // [FULLWIDTH QUOTATION MARK]
                        output[outputPos++] = L'"';
                        break;
                    case 0x2018: // [LEFT SINGLE QUOTATION MARK]
                    case 0x2019: // [RIGHT SINGLE QUOTATION MARK]
                    case 0x201a: // [SINGLE LOW-9 QUOTATION MARK]
                    case 0x201b: // [SINGLE HIGH-REVERSED-9 QUOTATION MARK]
                    case 0x2032: // [PRIME]
                    case 0x2035: // [REVERSED PRIME]
                    case 0x2039: // [SINGLE LEFT-POINTING ANGLE QUOTATION MARK]
                    case 0x203a: // [SINGLE RIGHT-POINTING ANGLE QUOTATION MARK]
                    case 0x275b: // [HEAVY SINGLE TURNED COMMA QUOTATION MARK ORNAMENT]
                    case 0x275c: // [HEAVY SINGLE COMMA QUOTATION MARK ORNAMENT]
                    case 0xff07: // [FULLWIDTH APOSTROPHE]
                        output[outputPos++] = L'\'';
                        break;
                    case 0x2010: // [HYPHEN]
                    case 0x2011: // [NON-BREAKING HYPHEN]
                    case 0x2012: // [FIGURE DASH]
                    case 0x2013: // [EN DASH]
                    case 0x2014: // [EM DASH]
                    case 0x207b: // [SUPERSCRIPT MINUS]
                    case 0x208b: // [SUBSCRIPT MINUS]
                    case 0xff0d: // [FULLWIDTH HYPHEN-MINUS]
                        output[outputPos++] = L'-';
                        break;
                    case 0x2045: // [LEFT SQUARE BRACKET WITH QUILL]
                    case 0x2772: // [LIGHT LEFT TORTOISE SHELL BRACKET ORNAMENT]
                    case 0xff3b: // [FULLWIDTH LEFT SQUARE BRACKET]
                        output[outputPos++] = L'[';
                        break;
                    case 0x2046: // [RIGHT SQUARE BRACKET WITH QUILL]
                    case 0x2773: // [LIGHT RIGHT TORTOISE SHELL BRACKET ORNAMENT]
                    case 0xff3d: // [FULLWIDTH RIGHT SQUARE BRACKET]
                        output[outputPos++] = L']';
                        break;
                    case 0x207d: // [SUPERSCRIPT LEFT PARENTHESIS]
                    case 0x208d: // [SUBSCRIPT LEFT PARENTHESIS]
                    case 0x2768: // [MEDIUM LEFT PARENTHESIS ORNAMENT]
                    case 0x276a: // [MEDIUM FLATTENED LEFT PARENTHESIS ORNAMENT]
                    case 0xff08: // [FULLWIDTH LEFT PARENTHESIS]
                        output[outputPos++] = L'(';
                        break;
                    case 0x2e28: // [LEFT DOUBLE PARENTHESIS]
                        output[outputPos++] = L'(';
                        output[outputPos++] = L'(';
                        break;
                    case 0x207e: // [SUPERSCRIPT RIGHT PARENTHESIS]
                    case 0x208e: // [SUBSCRIPT RIGHT PARENTHESIS]
                    case 0x2769: // [MEDIUM RIGHT PARENTHESIS ORNAMENT]
                    case 0x276b: // [MEDIUM FLATTENED RIGHT PARENTHESIS ORNAMENT]
                    case 0xff09: // [FULLWIDTH RIGHT PARENTHESIS]
                        output[outputPos++] = L')';
                        break;
                    case 0x2e29: // [RIGHT DOUBLE PARENTHESIS]
                        output[outputPos++] = L')';
                        output[outputPos++] = L')';
                        break;
                    case 0x276c: // [MEDIUM LEFT-POINTING ANGLE BRACKET ORNAMENT]
                    case 0x2770: // [HEAVY LEFT-POINTING ANGLE BRACKET ORNAMENT]
                    case 0xff1c: // [FULLWIDTH LESS-THAN SIGN]
                        output[outputPos++] = L'<';
                        break;
                    case 0x276d: // [MEDIUM RIGHT-POINTING ANGLE BRACKET ORNAMENT]
                    case 0x2771: // [HEAVY RIGHT-POINTING ANGLE BRACKET ORNAMENT]
                    case 0xff1e: // [FULLWIDTH GREATER-THAN SIGN]
                        output[outputPos++] = L'>';
                        break;
                    case 0x2774: // [MEDIUM LEFT CURLY BRACKET ORNAMENT]
                    case 0xff5b: // [FULLWIDTH LEFT CURLY BRACKET]
                        output[outputPos++] = L'{';
                        break;
                    case 0x2775: // [MEDIUM RIGHT CURLY BRACKET ORNAMENT]
                    case 0xff5d: // [FULLWIDTH RIGHT CURLY BRACKET]
                        output[outputPos++] = L'}';
                        break;
                    case 0x207a: // [SUPERSCRIPT PLUS SIGN]
                    case 0x208a: // [SUBSCRIPT PLUS SIGN]
                    case 0xff0b: // [FULLWIDTH PLUS SIGN]
                        output[outputPos++] = L'+';
                        break;
                    case 0x207c: // [SUPERSCRIPT EQUALS SIGN]
                    case 0x208c: // [SUBSCRIPT EQUALS SIGN]
                    case 0xff1d: // [FULLWIDTH EQUALS SIGN]
                        output[outputPos++] = L'=';
                        break;
                    case 0xff01: // [FULLWIDTH EXCLAMATION MARK]
                        output[outputPos++] = L'!';
                        break;
                    case 0x203c: // [DOUBLE EXCLAMATION MARK]
                        output[outputPos++] = L'!';
                        output[outputPos++] = L'!';
                        break;
                    case 0x2049: // [EXCLAMATION QUESTION MARK]
                        output[outputPos++] = L'!';
                        output[outputPos++] = L'?';
                        break;
                    case 0xff03: // [FULLWIDTH NUMBER SIGN]
                        output[outputPos++] = L'#';
                        break;
                    case 0xff04: // [FULLWIDTH DOLLAR SIGN]
                        output[outputPos++] = L'$';
                        break;
                    case 0x2052: // [COMMERCIAL MINUS SIGN]
                    case 0xff05: // [FULLWIDTH PERCENT SIGN]
                        output[outputPos++] = L'%';
                        break;
                    case 0xff06: // [FULLWIDTH AMPERSAND]
                        output[outputPos++] = L'&';
                        break;
                    case 0x204e: // [LOW ASTERISK]
                    case 0xff0a: // [FULLWIDTH ASTERISK]
                        output[outputPos++] = L'*';
                        break;
                    case 0xff0c: // [FULLWIDTH COMMA]
                        output[outputPos++] = L',';
                        break;
                    case 0xff0e: // [FULLWIDTH FULL STOP]
                        output[outputPos++] = L'.';
                        break;
                    case 0x2044: // [FRACTION SLASH]
                    case 0xff0f: // [FULLWIDTH SOLIDUS]
                        output[outputPos++] = L'/';
                        break;
                    case 0xff1a: // [FULLWIDTH COLON]
                        output[outputPos++] = L':';
                        break;
                    case 0x204f: // [REVERSED SEMICOLON]
                    case 0xff1b: // [FULLWIDTH SEMICOLON]
                        output[outputPos++] = L';';
                        break;
                    case 0xff1f: // [FULLWIDTH QUESTION MARK]
                        output[outputPos++] = L'?';
                        break;
                    case 0x2047: // [DOUBLE QUESTION MARK]
                        output[outputPos++] = L'?';
                        output[outputPos++] = L'?';
                        break;
                    case 0x2048: // [QUESTION EXCLAMATION MARK]
                        output[outputPos++] = L'?';
                        output[outputPos++] = L'!';
                        break;
                    case 0xff20: // [FULLWIDTH COMMERCIAL AT]
                        output[outputPos++] = L'@';
                        break;
                    case 0xff3c: // [FULLWIDTH REVERSE SOLIDUS]
                        output[outputPos++] = L'\\';
                        break;
                    case 0x2038: // [CARET]
                    case 0xff3e: // [FULLWIDTH CIRCUMFLEX ACCENT]
                        output[outputPos++] = L'^';
                        break;
                    case 0xff3f: // [FULLWIDTH LOW LINE]
                        output[outputPos++] = L'_';
                        break;
                    case 0x2053: // [SWUNG DASH]
                    case 0xff5e: // [FULLWIDTH TILDE]
                        output[outputPos++] = L'~';
                        break;
                    default:
                        output[outputPos++] = c;
                        break;
                }
            }
        }
        return outputPos;
    }
}
