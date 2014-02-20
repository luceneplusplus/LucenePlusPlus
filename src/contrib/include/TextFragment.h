/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef TEXTFRAGMENT_H
#define TEXTFRAGMENT_H

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene {

/// Low-level class used to record information about a section of a document with a score.
class LPPCONTRIBAPI TextFragment : public LuceneObject {
public:
    TextFragment(const StringBufferPtr& markedUpText, int32_t textStartPos, int32_t fragNum);
    virtual ~TextFragment();

    LUCENE_CLASS(TextFragment);

public:
    StringBufferPtr markedUpText;
    int32_t fragNum;
    int32_t textStartPos;
    int32_t textEndPos;
    double score;

public:
    void setScore(double score);
    double getScore();

    /// @param frag2 Fragment to be merged into this one
    void merge(const TextFragmentPtr& frag2);

    /// @return true if this fragment follows the one passed
    bool follows(const TextFragmentPtr& fragment);

    /// @return the fragment sequence number
    int32_t getFragNum();

    /// Returns the marked-up text for this text fragment
    virtual String toString();
};

/// Utility class to store a string buffer that contains text fragment
class LPPCONTRIBAPI StringBuffer : public LuceneObject {
public:
    virtual ~StringBuffer();
    LUCENE_CLASS(StringBuffer);

protected:
    StringStream buffer;

public:
    virtual String toString();
    virtual int32_t length();
    virtual void append(const String& str);
    virtual void clear();
};

}

#endif
