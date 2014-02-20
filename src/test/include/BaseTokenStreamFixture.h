/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef BASETOKENSTREAMFIXTURE_H
#define BASETOKENSTREAMFIXTURE_H

#include "LuceneTestFixture.h"
#include "Attribute.h"

namespace Lucene {

DECLARE_SHARED_PTR(CheckClearAttributesAttribute)

class CheckClearAttributesAttribute : public Attribute {
public:
    CheckClearAttributesAttribute();
    virtual ~CheckClearAttributesAttribute();

    LUCENE_CLASS(CheckClearAttributesAttribute);

protected:
    bool clearCalled;

public:
    bool getAndResetClearCalled();

    virtual void clear();
    virtual bool equals(const LuceneObjectPtr& other);
    virtual int32_t hashCode();
    virtual void copyTo(const AttributePtr& target);
    virtual LuceneObjectPtr clone(const LuceneObjectPtr& other = LuceneObjectPtr());
};

class BaseTokenStreamFixture : public LuceneTestFixture {
public:
    virtual ~BaseTokenStreamFixture();

public:
    // some helpers to test Analyzers and TokenStreams

    static void checkTokenStreamContents(const TokenStreamPtr& ts, Collection<String> output, Collection<int32_t> startOffsets, Collection<int32_t> endOffsets,
                                         Collection<String> types, Collection<int32_t> posIncrements, int32_t finalOffset = -1);
    static void checkTokenStreamContents(const TokenStreamPtr& ts, Collection<String> output);
    static void checkTokenStreamContents(const TokenStreamPtr& ts, Collection<String> output, Collection<String> types);
    static void checkTokenStreamContents(const TokenStreamPtr& ts, Collection<String> output, Collection<int32_t> posIncrements);
    static void checkTokenStreamContents(const TokenStreamPtr& ts, Collection<String> output, Collection<int32_t> startOffsets, Collection<int32_t> endOffsets);
    static void checkTokenStreamContents(const TokenStreamPtr& ts, Collection<String> output, Collection<int32_t> startOffsets, Collection<int32_t> endOffsets, int32_t finalOffset);
    static void checkTokenStreamContents(const TokenStreamPtr& ts, Collection<String> output, Collection<int32_t> startOffsets, Collection<int32_t> endOffsets, Collection<int32_t> posIncrements);
    static void checkTokenStreamContents(const TokenStreamPtr& ts, Collection<String> output, Collection<int32_t> startOffsets, Collection<int32_t> endOffsets, Collection<int32_t> posIncrements, int32_t finalOffset);

    static void checkAnalyzesTo(const AnalyzerPtr& analyzer, const String& input, Collection<String> output, Collection<int32_t> startOffsets,
                                Collection<int32_t> endOffsets, Collection<String> types, Collection<int32_t> posIncrements);
    static void checkAnalyzesTo(const AnalyzerPtr& analyzer, const String& input, Collection<String> output);
    static void checkAnalyzesTo(const AnalyzerPtr& analyzer, const String& input, Collection<String> output, Collection<String> types);
    static void checkAnalyzesTo(const AnalyzerPtr& analyzer, const String& input, Collection<String> output, Collection<int32_t> posIncrements);
    static void checkAnalyzesTo(const AnalyzerPtr& analyzer, const String& input, Collection<String> output, Collection<int32_t> startOffsets, Collection<int32_t> endOffsets);
    static void checkAnalyzesTo(const AnalyzerPtr& analyzer, const String& input, Collection<String> output, Collection<int32_t> startOffsets, Collection<int32_t> endOffsets, Collection<int32_t> posIncrements);

    static void checkAnalyzesToReuse(const AnalyzerPtr& analyzer, const String& input, Collection<String> output, Collection<int32_t> startOffsets,
                                     Collection<int32_t> endOffsets, Collection<String> types, Collection<int32_t> posIncrements);
    static void checkAnalyzesToReuse(const AnalyzerPtr& analyzer, const String& input, Collection<String> output);
    static void checkAnalyzesToReuse(const AnalyzerPtr& analyzer, const String& input, Collection<String> output, Collection<String> types);
    static void checkAnalyzesToReuse(const AnalyzerPtr& analyzer, const String& input, Collection<String> output, Collection<int32_t> posIncrements);
    static void checkAnalyzesToReuse(const AnalyzerPtr& analyzer, const String& input, Collection<String> output, Collection<int32_t> startOffsets, Collection<int32_t> endOffsets);
    static void checkAnalyzesToReuse(const AnalyzerPtr& analyzer, const String& input, Collection<String> output, Collection<int32_t> startOffsets, Collection<int32_t> endOffsets, Collection<int32_t> posIncrements);

    static void checkOneTerm(const AnalyzerPtr& analyzer, const String& input, const String& expected);
    static void checkOneTermReuse(const AnalyzerPtr& analyzer, const String& input, const String& expected);
};

}

#endif
