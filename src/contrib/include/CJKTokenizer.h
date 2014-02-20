/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CJKTOKENIZER_H
#define CJKTOKENIZER_H

#include "Tokenizer.h"

namespace Lucene {

/// CJKTokenizer is designed for Chinese, Japanese, and Korean languages.
///
/// The tokens returned are every two adjacent characters with overlap match.
///
/// Example: "lucene C1C2C3C4" will be segmented to: "lucene" "C1C2" "C2C3" "C3C4".
///
/// Additionally, the following is applied to Latin text (such as English):
/// <ul>
/// <li>Text is converted to lowercase.
/// <li>Numeric digits, '+', '#', and '_' are tokenized as letters.
/// <li>Full-width forms are converted to half-width forms.
/// </ul>
/// For more info on Asian language (Chinese, Japanese, and Korean) text segmentation:
/// please search <a href="http://www.google.com/search?q=word+chinese+segment">google</a>
class LPPCONTRIBAPI CJKTokenizer : public Tokenizer {
public:
    CJKTokenizer(const ReaderPtr& input);
    CJKTokenizer(const AttributeSourcePtr& source, const ReaderPtr& input);
    CJKTokenizer(const AttributeFactoryPtr& factory, const ReaderPtr& input);

    virtual ~CJKTokenizer();

    LUCENE_CLASS(CJKTokenizer);

public:
    /// Word token type
    static const int32_t WORD_TYPE;

    /// Single byte token type
    static const int32_t SINGLE_TOKEN_TYPE;

    /// Double byte token type
    static const int32_t DOUBLE_TOKEN_TYPE;

    /// Names for token types
    static const wchar_t* TOKEN_TYPE_NAMES[];

protected:
    /// Max word length
    static const int32_t MAX_WORD_LEN;

    static const int32_t IO_BUFFER_SIZE;

    enum UnicodeBlock { NONE, BASIC_LATIN, HALFWIDTH_AND_FULLWIDTH_FORMS };

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

    /// word type: single=>ASCII  double=>non-ASCII word=>default
    int32_t tokenType;

    /// tag: previous character is a cached double-byte character  "C1C2C3C4"
    /// ----(set the C1 isTokened) C1C2 "C2C3C4" ----(set the C2 isTokened)
    /// C1C2 C2C3 "C3C4" ----(set the C3 isTokened) "C1C2 C2C3 C3C4"
    bool preIsTokened;

    TermAttributePtr termAtt;
    OffsetAttributePtr offsetAtt;
    TypeAttributePtr typeAtt;

protected:
    /// return unicode block for given character (see http://unicode.org/Public/UNIDATA/Blocks.txt)
    UnicodeBlock unicodeBlock(wchar_t c);

public:
    virtual void initialize();
    virtual bool incrementToken();
    virtual void end();
    virtual void reset();
    virtual void reset(const ReaderPtr& input);
};

}

#endif
