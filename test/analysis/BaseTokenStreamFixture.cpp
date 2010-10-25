/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "TokenStream.h"
#include "TermAttribute.h"
#include "OffsetAttribute.h"
#include "TypeAttribute.h"
#include "PositionIncrementAttribute.h"
#include "Analyzer.h"
#include "StringReader.h"

namespace Lucene
{
    CheckClearAttributesAttribute::CheckClearAttributesAttribute()
    {
        clearCalled = false;
    }
    
    CheckClearAttributesAttribute::~CheckClearAttributesAttribute()
    {
    }
    
    bool CheckClearAttributesAttribute::getAndResetClearCalled()
    {
        bool _clearCalled = clearCalled;
        clearCalled = false;
        return _clearCalled;
    }
    
    void CheckClearAttributesAttribute::clear()
    {
        clearCalled = true;
    }
    
    bool CheckClearAttributesAttribute::equals(LuceneObjectPtr other)
    {
        if (Attribute::equals(other))
            return true;
        
        CheckClearAttributesAttributePtr otherAttribute(boost::dynamic_pointer_cast<CheckClearAttributesAttribute>(other));
        if (otherAttribute)
            return (otherAttribute->clearCalled == clearCalled);
        
        return false;
    }
    
    int32_t CheckClearAttributesAttribute::hashCode()
    {
        return 76137213 ^ (clearCalled ? 1231 : 1237);
    }
    
    void CheckClearAttributesAttribute::copyTo(AttributePtr target)
    {
        CheckClearAttributesAttributePtr clearAttribute(boost::dynamic_pointer_cast<CheckClearAttributesAttribute>(target));
        clearAttribute->clear();
    }
    
    LuceneObjectPtr CheckClearAttributesAttribute::clone(LuceneObjectPtr other)
    {
        LuceneObjectPtr clone = other ? other : newLucene<CheckClearAttributesAttribute>();
        CheckClearAttributesAttributePtr cloneAttribute(boost::dynamic_pointer_cast<CheckClearAttributesAttribute>(Attribute::clone(clone)));
        cloneAttribute->clearCalled = clearCalled;
        return cloneAttribute;
    }
    
    BaseTokenStreamFixture::~BaseTokenStreamFixture()
    {
    }
    
    void BaseTokenStreamFixture::checkTokenStreamContents(TokenStreamPtr ts, Collection<String> output, Collection<int32_t> startOffsets, Collection<int32_t> endOffsets,
                                                          Collection<String> types, Collection<int32_t> posIncrements, int32_t finalOffset)
    {
        BOOST_CHECK(output);
        CheckClearAttributesAttributePtr checkClearAtt = ts->addAttribute<CheckClearAttributesAttribute>();
        
        BOOST_CHECK(ts->hasAttribute<TermAttribute>());
        TermAttributePtr termAtt = ts->getAttribute<TermAttribute>();
        
        OffsetAttributePtr offsetAtt;
        if (startOffsets || endOffsets || finalOffset != -1)
        {
            BOOST_CHECK(ts->hasAttribute<OffsetAttribute>());
            offsetAtt = ts->getAttribute<OffsetAttribute>();
        }
        
        TypeAttributePtr typeAtt;
        if (types)
        {
            BOOST_CHECK(ts->hasAttribute<TypeAttribute>());
            typeAtt = ts->getAttribute<TypeAttribute>();
        }
        
        PositionIncrementAttributePtr posIncrAtt;
        if (posIncrements)
        {
            BOOST_CHECK(ts->hasAttribute<PositionIncrementAttribute>());
            posIncrAtt = ts->getAttribute<PositionIncrementAttribute>();
        }
        
        ts->reset();
        for (int32_t i = 0; i < output.size(); ++i)
        {
            // extra safety to enforce, that the state is not preserved and also assign bogus values
            ts->clearAttributes();
            termAtt->setTermBuffer(L"bogusTerm");
            if (offsetAtt)
                offsetAtt->setOffset(14584724, 24683243);
            if (typeAtt)
                typeAtt->setType(L"bogusType");
            if (posIncrAtt)
                posIncrAtt->setPositionIncrement(45987657);
            
            checkClearAtt->getAndResetClearCalled(); // reset it, because we called clearAttribute() before
            BOOST_CHECK(ts->incrementToken());
            BOOST_CHECK(checkClearAtt->getAndResetClearCalled());
            
            BOOST_CHECK_EQUAL(output[i], termAtt->term());
            if (startOffsets)
                BOOST_CHECK_EQUAL(startOffsets[i], offsetAtt->startOffset());
            if (endOffsets)
                BOOST_CHECK_EQUAL(endOffsets[i], offsetAtt->endOffset());
            if (types)
                BOOST_CHECK_EQUAL(types[i], typeAtt->type());
            if (posIncrements)
                BOOST_CHECK_EQUAL(posIncrements[i], posIncrAtt->getPositionIncrement());
        }
        BOOST_CHECK(!ts->incrementToken());
        ts->end();
        if (finalOffset != -1)
            BOOST_CHECK_EQUAL(finalOffset, offsetAtt->endOffset());
        ts->close();
    }
    
    void BaseTokenStreamFixture::checkTokenStreamContents(TokenStreamPtr ts, Collection<String> output)
    {
        checkTokenStreamContents(ts, output, Collection<int32_t>(), Collection<int32_t>(), Collection<String>(), Collection<int32_t>(), -1);
    }
    
    void BaseTokenStreamFixture::checkTokenStreamContents(TokenStreamPtr ts, Collection<String> output, Collection<String> types)
    {
        checkTokenStreamContents(ts, output, Collection<int32_t>(), Collection<int32_t>(), types, Collection<int32_t>(), -1);
    }
    
    void BaseTokenStreamFixture::checkTokenStreamContents(TokenStreamPtr ts, Collection<String> output, Collection<int32_t> posIncrements)
    {
        checkTokenStreamContents(ts, output, Collection<int32_t>(), Collection<int32_t>(), Collection<String>(), posIncrements, -1);
    }
    
    void BaseTokenStreamFixture::checkTokenStreamContents(TokenStreamPtr ts, Collection<String> output, Collection<int32_t> startOffsets, Collection<int32_t> endOffsets)
    {
        checkTokenStreamContents(ts, output, startOffsets, endOffsets, Collection<String>(), Collection<int32_t>(), -1);
    }
    
    void BaseTokenStreamFixture::checkTokenStreamContents(TokenStreamPtr ts, Collection<String> output, Collection<int32_t> startOffsets, Collection<int32_t> endOffsets, int32_t finalOffset)
    {
        checkTokenStreamContents(ts, output, startOffsets, endOffsets, Collection<String>(), Collection<int32_t>(), finalOffset);
    }
    
    void BaseTokenStreamFixture::checkTokenStreamContents(TokenStreamPtr ts, Collection<String> output, Collection<int32_t> startOffsets, Collection<int32_t> endOffsets, Collection<int32_t> posIncrements)
    {
        checkTokenStreamContents(ts, output, startOffsets, endOffsets, Collection<String>(), posIncrements, -1);
    }
    
    void BaseTokenStreamFixture::checkTokenStreamContents(TokenStreamPtr ts, Collection<String> output, Collection<int32_t> startOffsets, Collection<int32_t> endOffsets, Collection<int32_t> posIncrements, int32_t finalOffset)
    {
        checkTokenStreamContents(ts, output, startOffsets, endOffsets, Collection<String>(), posIncrements, finalOffset);
    }
    
    void BaseTokenStreamFixture::checkAnalyzesTo(AnalyzerPtr analyzer, const String& input, Collection<String> output, Collection<int32_t> startOffsets,
                                                 Collection<int32_t> endOffsets, Collection<String> types, Collection<int32_t> posIncrements)
    {
        checkTokenStreamContents(analyzer->tokenStream(L"dummy", newLucene<StringReader>(input)), output, startOffsets, endOffsets, types, posIncrements, (int32_t)input.length());
    }
    
    void BaseTokenStreamFixture::checkAnalyzesTo(AnalyzerPtr analyzer, const String& input, Collection<String> output)
    {
        checkAnalyzesTo(analyzer, input, output, Collection<int32_t>(), Collection<int32_t>(), Collection<String>(), Collection<int32_t>());
    }
    
    void BaseTokenStreamFixture::checkAnalyzesTo(AnalyzerPtr analyzer, const String& input, Collection<String> output, Collection<String> types)
    {
        checkAnalyzesTo(analyzer, input, output, Collection<int32_t>(), Collection<int32_t>(), types, Collection<int32_t>());
    }
    
    void BaseTokenStreamFixture::checkAnalyzesTo(AnalyzerPtr analyzer, const String& input, Collection<String> output, Collection<int32_t> posIncrements)
    {
        checkAnalyzesTo(analyzer, input, output, Collection<int32_t>(), Collection<int32_t>(), Collection<String>(), posIncrements);
    }
    
    void BaseTokenStreamFixture::checkAnalyzesTo(AnalyzerPtr analyzer, const String& input, Collection<String> output, Collection<int32_t> startOffsets, Collection<int32_t> endOffsets)
    {
        checkAnalyzesTo(analyzer, input, output, startOffsets, endOffsets, Collection<String>(), Collection<int32_t>());
    }
    
    void BaseTokenStreamFixture::checkAnalyzesTo(AnalyzerPtr analyzer, const String& input, Collection<String> output, Collection<int32_t> startOffsets, Collection<int32_t> endOffsets, Collection<int32_t> posIncrements)
    {
        checkAnalyzesTo(analyzer, input, output, startOffsets, endOffsets, Collection<String>(), posIncrements);
    }
    
    void BaseTokenStreamFixture::checkAnalyzesToReuse(AnalyzerPtr analyzer, const String& input, Collection<String> output, Collection<int32_t> startOffsets,
                                                      Collection<int32_t> endOffsets, Collection<String> types, Collection<int32_t> posIncrements)
    {
        checkTokenStreamContents(analyzer->reusableTokenStream(L"dummy", newLucene<StringReader>(input)), output, startOffsets, endOffsets, types, posIncrements, (int32_t)input.length());
    }
    
    void BaseTokenStreamFixture::checkAnalyzesToReuse(AnalyzerPtr analyzer, const String& input, Collection<String> output)
    {
        checkAnalyzesToReuse(analyzer, input, output, Collection<int32_t>(), Collection<int32_t>(), Collection<String>(), Collection<int32_t>());
    }
    
    void BaseTokenStreamFixture::checkAnalyzesToReuse(AnalyzerPtr analyzer, const String& input, Collection<String> output, Collection<String> types)
    {
        checkAnalyzesToReuse(analyzer, input, output, Collection<int32_t>(), Collection<int32_t>(), types, Collection<int32_t>());
    }
    
    void BaseTokenStreamFixture::checkAnalyzesToReuse(AnalyzerPtr analyzer, const String& input, Collection<String> output, Collection<int32_t> posIncrements)
    {
        checkAnalyzesToReuse(analyzer, input, output, Collection<int32_t>(), Collection<int32_t>(), Collection<String>(), posIncrements);
    }
    
    void BaseTokenStreamFixture::checkAnalyzesToReuse(AnalyzerPtr analyzer, const String& input, Collection<String> output, Collection<int32_t> startOffsets, Collection<int32_t> endOffsets)
    {
        checkAnalyzesToReuse(analyzer, input, output, startOffsets, endOffsets, Collection<String>(), Collection<int32_t>());
    }
    
    void BaseTokenStreamFixture::checkAnalyzesToReuse(AnalyzerPtr analyzer, const String& input, Collection<String> output, Collection<int32_t> startOffsets, Collection<int32_t> endOffsets, Collection<int32_t> posIncrements)
    {
        checkAnalyzesToReuse(analyzer, input, output, startOffsets, endOffsets, Collection<String>(), posIncrements);
    }
    
    void BaseTokenStreamFixture::checkOneTerm(AnalyzerPtr analyzer, const String& input, const String& expected)
    {
        checkAnalyzesTo(analyzer, input, newCollection<String>(expected));
    }
    
    void BaseTokenStreamFixture::checkOneTermReuse(AnalyzerPtr analyzer, const String& input, const String& expected)
    {
        checkAnalyzesToReuse(analyzer, input, newCollection<String>(expected));
    }
}
