/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "CJKTokenizer.h"
#include "TermAttribute.h"
#include "OffsetAttribute.h"
#include "TypeAttribute.h"
#include "Reader.h"
#include "CharFolder.h"
#include "MiscUtils.h"
#include "UnicodeUtils.h"

namespace Lucene {

/// Word token type
const int32_t CJKTokenizer::WORD_TYPE = 0;

/// Single byte token type
const int32_t CJKTokenizer::SINGLE_TOKEN_TYPE = 1;

/// Double byte token type
const int32_t CJKTokenizer::DOUBLE_TOKEN_TYPE = 2;

/// Names for token types
const wchar_t* CJKTokenizer::TOKEN_TYPE_NAMES[] = {L"word", L"single", L"double"};

const int32_t CJKTokenizer::MAX_WORD_LEN = 255;

const int32_t CJKTokenizer::IO_BUFFER_SIZE = 256;

CJKTokenizer::CJKTokenizer(const ReaderPtr& input) : Tokenizer(input) {
}

CJKTokenizer::CJKTokenizer(const AttributeSourcePtr& source, const ReaderPtr& input) : Tokenizer(source, input) {
}

CJKTokenizer::CJKTokenizer(const AttributeFactoryPtr& factory, const ReaderPtr& input) : Tokenizer(factory, input) {
}

CJKTokenizer::~CJKTokenizer() {
}

void CJKTokenizer::initialize() {
    offset = 0;
    bufferIndex = 0;
    dataLen = 0;
    buffer = CharArray::newInstance(MAX_WORD_LEN);
    ioBuffer = CharArray::newInstance(IO_BUFFER_SIZE);
    tokenType = WORD_TYPE;
    preIsTokened = false;

    termAtt = addAttribute<TermAttribute>();
    offsetAtt = addAttribute<OffsetAttribute>();
    typeAtt = addAttribute<TypeAttribute>();
}

CJKTokenizer::UnicodeBlock CJKTokenizer::unicodeBlock(wchar_t c) {
    if (c >= 0x0000 && c <= 0x007f) {
        return BASIC_LATIN;
    } else if (c >= 0xff00 && c <= 0xffef) {
        return HALFWIDTH_AND_FULLWIDTH_FORMS;
    }
    return NONE;
}

bool CJKTokenizer::incrementToken() {
    clearAttributes();

    while (true) { // loop until we find a non-empty token
        int32_t length = 0;

        // the position used to create Token
        int32_t start = offset;

        while (true) { // loop until we've found a full token
            wchar_t c = 0;
            UnicodeBlock ub = NONE;

            ++offset;

            if (bufferIndex >= dataLen) {
                dataLen = input->read(ioBuffer.get(), 0, ioBuffer.size());
                bufferIndex = 0;
            }

            if (dataLen == -1) {
                if (length > 0) {
                    if (preIsTokened == true) {
                        length = 0;
                        preIsTokened = false;
                    } else {
                        --offset;
                    }
                    break;
                } else {
                    --offset;
                    return false;
                }
            } else {
                // get current character
                c = ioBuffer[bufferIndex++];

                // get the UnicodeBlock of the current character
                ub = unicodeBlock(c);
            }

            // if the current character is ASCII or Extend ASCII
            if (ub == BASIC_LATIN || ub == HALFWIDTH_AND_FULLWIDTH_FORMS) {
                if (ub == HALFWIDTH_AND_FULLWIDTH_FORMS) {
                    int32_t i = (int32_t)c;
                    if (i >= 65281 && i <= 65374) {
                        // convert certain HALFWIDTH_AND_FULLWIDTH_FORMS to BASIC_LATIN
                        i = i - 65248;
                        c = (wchar_t)i;
                    }
                }

                // if the current character is a letter or "_" "+" "#"
                if (UnicodeUtil::isAlnum(c) || c == L'_' || c == L'+' || c == L'#') {
                    if (length == 0) {
                        // "javaC1C2C3C4linux" <br>
                        //      ^--: the current character begin to token the ASCII
                        // letter
                        start = offset - 1;
                    } else if (tokenType == DOUBLE_TOKEN_TYPE) {
                        // "javaC1C2C3C4linux" <br>
                        //              ^--: the previous non-ASCII
                        // : the current character
                        --offset;
                        --bufferIndex;

                        if (preIsTokened) {
                            // there is only one non-ASCII has been stored
                            length = 0;
                            preIsTokened = false;
                            break;
                        } else {
                            break;
                        }
                    }

                    // store the LowerCase(c) in the buffer
                    buffer[length++] = CharFolder::toLower(c);
                    tokenType = SINGLE_TOKEN_TYPE;

                    // break the procedure if buffer overflowed!
                    if (length == MAX_WORD_LEN) {
                        break;
                    }
                } else if (length > 0) {
                    if (preIsTokened) {
                        length = 0;
                        preIsTokened = false;
                    } else {
                        break;
                    }
                }
            } else {
                // non-ASCII letter, e.g."C1C2C3C4"
                if (UnicodeUtil::isAlpha(c)) {
                    if (length == 0) {
                        start = offset - 1;
                        buffer[length++] = c;
                        tokenType = DOUBLE_TOKEN_TYPE;
                    } else {
                        if (tokenType == SINGLE_TOKEN_TYPE) {
                            --offset;
                            --bufferIndex;

                            // return the previous ASCII characters
                            break;
                        } else {
                            buffer[length++] = c;
                            tokenType = DOUBLE_TOKEN_TYPE;

                            if (length == 2) {
                                --offset;
                                --bufferIndex;
                                preIsTokened = true;
                                break;
                            }
                        }
                    }
                } else if (length > 0) {
                    if (preIsTokened) {
                        // empty the buffer
                        length = 0;
                        preIsTokened = false;
                    } else {
                        break;
                    }
                }
            }
        }

        if (length > 0) {
            termAtt->setTermBuffer(buffer.get(), 0, length);
            offsetAtt->setOffset(correctOffset(start), correctOffset(start + length));
            typeAtt->setType(TOKEN_TYPE_NAMES[tokenType]);
            return true;
        } else if (dataLen == -1) {
            --offset;
            return false;
        }

        // Cycle back and try for the next token (don't return an empty string)
    }
}

void CJKTokenizer::end() {
    // set final offset
    int32_t finalOffset = correctOffset(offset);
    offsetAtt->setOffset(finalOffset, finalOffset);
}

void CJKTokenizer::reset() {
    Tokenizer::reset();
    offset = 0;
    bufferIndex = 0;
    dataLen = 0;
    preIsTokened = false;
    tokenType = WORD_TYPE;
}

void CJKTokenizer::reset(const ReaderPtr& input) {
    Tokenizer::reset(input);
    reset();
}

}
