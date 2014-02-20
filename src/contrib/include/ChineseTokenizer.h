/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CHINESETOKENIZER_H
#define CHINESETOKENIZER_H

#include "Tokenizer.h"

namespace Lucene {

/// Tokenize Chinese text as individual Chinese characters.
///
/// The difference between ChineseTokenizer and ChineseTokenizer is that they have different
/// token parsing logic.
///
/// For example, if the Chinese text "C1C2C3C4" is to be indexed:
/// <ul>
/// <li> The tokens returned from ChineseTokenizer are C1, C2, C3, C4.
/// <li>The tokens returned from the ChineseTokenizer are C1C2, C2C3, C3C4.
/// </ul>
///
/// Therefore the index created by ChineseTokenizer is much larger.
///
/// The problem is that when searching for C1, C1C2, C1C3, C4C2, C1C2C3 ... the
/// ChineseTokenizer works, but the ChineseTokenizer will not work.
class LPPCONTRIBAPI ChineseTokenizer : public Tokenizer {
public:
    ChineseTokenizer(const ReaderPtr& input);
    ChineseTokenizer(const AttributeSourcePtr& source, const ReaderPtr& input);
    ChineseTokenizer(const AttributeFactoryPtr& factory, const ReaderPtr& input);

    virtual ~ChineseTokenizer();

    LUCENE_CLASS(ChineseTokenizer);

protected:
    /// Max word length
    static const int32_t MAX_WORD_LEN;

    static const int32_t IO_BUFFER_SIZE;

protected:
    /// word offset, used to imply which character(in) is parsed
    int32_t offset;

    /// the index used only for ioBuffer
    int32_t bufferIndex;

    /// data length
    int32_t dataLen;

    /// character buffer, store the characters which are used to compose the returned Token
    CharArray buffer;

    /// I/O buffer, used to store the content of the input (one of the members of Tokenizer)
    CharArray ioBuffer;

    TermAttributePtr termAtt;
    OffsetAttributePtr offsetAtt;

    int32_t length;
    int32_t start;

public:
    virtual void initialize();
    virtual bool incrementToken();
    virtual void end();
    virtual void reset();
    virtual void reset(const ReaderPtr& input);

protected:
    void push(wchar_t c);
    bool flush();
};

}

#endif
