/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "ISOLatin1AccentFilter.h"
#include "CharTermAttribute.h"

namespace Lucene
{
    ISOLatin1AccentFilter::ISOLatin1AccentFilter(TokenStreamPtr input) : TokenFilter(input)
    {
        output = CharArray::newInstance(256);
        outputPos = 0;
        termAtt = addAttribute<CharTermAttribute>();
    }
    
    ISOLatin1AccentFilter::~ISOLatin1AccentFilter()
    {
    }
    
    bool ISOLatin1AccentFilter::incrementToken()
    {
        if (input->incrementToken())
        {
            wchar_t* buffer = termAtt->bufferArray();
            int32_t length = termAtt->length();
            
            // If no characters actually require rewriting then we just return token as-is
            for (int32_t i = 0; i < length; ++i)
            {
                wchar_t c = buffer[i];
                if (c >= 0x00c0 && c <= 0xfb06)
                {
                    removeAccents(buffer, length);
                    termAtt->copyBuffer(output.get(), 0, outputPos);
                    break;
                }
            }
            return true;
        }
        else
            return false;
    }
    
    void ISOLatin1AccentFilter::removeAccents(const wchar_t* input, int32_t length)
    {
        // Worst-case length required
        int32_t maxSizeNeeded = 2 * length;
        
        int32_t size = output.size();
        while (size < maxSizeNeeded)
            size *= 2;
        
        if (size != output.size())
            output.resize(size);
        
        outputPos = 0;
        int32_t pos = 0;
        
        wchar_t* output = this->output.get();
        
        for (int32_t i = 0; i < length; ++i, ++pos)
        {
            wchar_t c = input[pos];
            
            // Quick test: if it's not in range then just keep current character
            if (c < 0x00c0 || c > 0xfb06)
                output[outputPos++] = c;
            else
            {
                switch (c)
                {
                    case 0x00c0:
                    case 0x00c1:
                    case 0x00c2:
                    case 0x00c3:
                    case 0x00c4:
                    case 0x00c5:
                        output[outputPos++] = L'A';
                        break;
                    case 0x00c6:
                        output[outputPos++] = L'A';
                        output[outputPos++] = L'E';
                        break;
                    case 0x00c7:
                        output[outputPos++] = L'C';
                        break;
                    case 0x00c8:
                    case 0x00c9:
                    case 0x00ca:
                    case 0x00cb:
                        output[outputPos++] = L'E';
                        break;
                    case 0x00cc:
                    case 0x00cd:
                    case 0x00ce:
                    case 0x00cf:
                        output[outputPos++] = L'I';
                        break;
                    case 0x0132:
                        output[outputPos++] = L'I';
                        output[outputPos++] = L'J';
                        break;
                    case 0x00d0:
                        output[outputPos++] = L'D';
                        break;
                    case 0x00d1:
                        output[outputPos++] = L'N';
                        break;
                    case 0x00d2:
                    case 0x00d3:
                    case 0x00d4:
                    case 0x00d5:
                    case 0x00d6:
                    case 0x00d8:
                        output[outputPos++] = L'O';
                        break;
                    case 0x0152:
                        output[outputPos++] = L'O';
                        output[outputPos++] = L'E';
                        break;
                    case 0x00de:
                        output[outputPos++] = L'T';
                        output[outputPos++] = L'H';
                        break;
                    case 0x00d9:
                    case 0x00da:
                    case 0x00db:
                    case 0x00dc:
                        output[outputPos++] = L'U';
                        break;
                    case 0x00dd:
                    case 0x0178:
                        output[outputPos++] = L'Y';
                        break;
                    case 0x00e0:
                    case 0x00e1:
                    case 0x00e2:
                    case 0x00e3:
                    case 0x00e4:
                    case 0x00e5:
                        output[outputPos++] = L'a';
                        break;
                    case 0x00e6:
                        output[outputPos++] = L'a';
                        output[outputPos++] = L'e';
                        break;
                    case 0x00e7:
                        output[outputPos++] = L'c';
                        break;
                    case 0x00e8:
                    case 0x00e9:
                    case 0x00ea:
                    case 0x00eb:
                        output[outputPos++] = L'e';
                        break;
                    case 0x00ec:
                    case 0x00ed:
                    case 0x00ee:
                    case 0x00ef:
                        output[outputPos++] = L'i';
                        break;
                    case 0x0133:
                        output[outputPos++] = L'i';
                        output[outputPos++] = L'j';
                        break;
                    case 0x00f0:
                        output[outputPos++] = L'd';
                        break;
                    case 0x00f1:
                        output[outputPos++] = L'n';
                        break;
                    case 0x00f2:
                    case 0x00f3:
                    case 0x00f4:
                    case 0x00f5:
                    case 0x00f6:
                    case 0x00f8:
                        output[outputPos++] = L'o';
                        break;
                    case 0x0153:
                        output[outputPos++] = L'o';
                        output[outputPos++] = L'e';
                        break;
                    case 0x00df:
                        output[outputPos++] = L's';
                        output[outputPos++] = L's';
                        break;
                    case 0x00fe:
                        output[outputPos++] = L't';
                        output[outputPos++] = L'h';
                        break;
                    case 0x00f9:
                    case 0x00fa:
                    case 0x00fb:
                    case 0x00fc:
                        output[outputPos++] = L'u';
                        break;
                    case 0x00fd:
                    case 0x00ff:
                        output[outputPos++] = L'y';
                        break;
                    case 0xfb00:
                        output[outputPos++] = L'f';
                        output[outputPos++] = L'f';
                        break;
                    case 0xfb01:
                        output[outputPos++] = L'f';
                        output[outputPos++] = L'i';
                        break;
                    case 0xfb02:
                        output[outputPos++] = L'f';
                        output[outputPos++] = L'l';
                        break;
                    case 0xfb05:
                        output[outputPos++] = L'f';
                        output[outputPos++] = L't';
                        break;
                    case 0xfb06:
                        output[outputPos++] = L's';
                        output[outputPos++] = L't';
                        break;
                    default :
                        output[outputPos++] = c;
                        break;
                }
            }
        }
    }
}
