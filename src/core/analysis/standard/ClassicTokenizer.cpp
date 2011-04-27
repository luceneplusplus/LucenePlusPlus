/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "ClassicTokenizer.h"
#include "ClassicTokenizerImpl.h"
#include "StandardAnalyzer.h"
#include "CharTermAttribute.h"
#include "OffsetAttribute.h"
#include "PositionIncrementAttribute.h"
#include "TypeAttribute.h"

namespace Lucene
{
    const int32_t ClassicTokenizer::ALPHANUM = 0;
    const int32_t ClassicTokenizer::APOSTROPHE = 1;
    const int32_t ClassicTokenizer::ACRONYM = 2;
    const int32_t ClassicTokenizer::COMPANY = 3;
    const int32_t ClassicTokenizer::EMAIL = 4;
    const int32_t ClassicTokenizer::HOST = 5;
    const int32_t ClassicTokenizer::NUM = 6;
    const int32_t ClassicTokenizer::CJ = 7;

    /// @deprecated this solves a bug where HOSTs that end with '.' are identified as ACRONYMs.
    const int32_t ClassicTokenizer::ACRONYM_DEP = 8;
    
    ClassicTokenizer::ClassicTokenizer(LuceneVersion::Version matchVersion, ReaderPtr input)
    {
        init(input, matchVersion);
    }
    
    ClassicTokenizer::ClassicTokenizer(LuceneVersion::Version matchVersion, AttributeSourcePtr source, ReaderPtr input) : Tokenizer(source)
    {
        init(input, matchVersion);
    }
    
    ClassicTokenizer::ClassicTokenizer(LuceneVersion::Version matchVersion, AttributeFactoryPtr factory, ReaderPtr input) : Tokenizer(factory)
    {
        init(input, matchVersion);
    }
    
    ClassicTokenizer::~ClassicTokenizer()
    {
    }
    
    const Collection<String> ClassicTokenizer::TOKEN_TYPES()
    {
        static Collection<String> _TOKEN_TYPES;
        if (!_TOKEN_TYPES)
        {
            _TOKEN_TYPES = newCollection<String>(
                L"<ALPHANUM>",
                L"<APOSTROPHE>",
                L"<ACRONYM>",
                L"<COMPANY>",
                L"<EMAIL>",
                L"<HOST>",
                L"<NUM>",
                L"<CJ>",
                L"<ACRONYM_DEP>"
            );
        }
        return _TOKEN_TYPES;
    }
    
    void ClassicTokenizer::init(ReaderPtr input, LuceneVersion::Version matchVersion)
    {
        this->scanner = newLucene<ClassicTokenizerImpl>(input);
        replaceInvalidAcronym = LuceneVersion::onOrAfter(matchVersion, LuceneVersion::LUCENE_24);
        maxTokenLength = StandardAnalyzer::DEFAULT_MAX_TOKEN_LENGTH;
        this->input = input;
        termAtt = addAttribute<CharTermAttribute>();
        offsetAtt = addAttribute<OffsetAttribute>();
        posIncrAtt = addAttribute<PositionIncrementAttribute>();
        typeAtt = addAttribute<TypeAttribute>();
    }
    
    void ClassicTokenizer::setMaxTokenLength(int32_t length)
    {
        this->maxTokenLength = length;
    }
    
    int32_t ClassicTokenizer::getMaxTokenLength()
    {
        return maxTokenLength;
    }
    
    bool ClassicTokenizer::incrementToken()
    {
        clearAttributes();
        int32_t posIncr = 1;
        
        while (true)
        {
            int32_t tokenType = scanner->getNextToken();
            
            if (tokenType == ClassicTokenizerImpl::YYEOF)
                return false;
            
            if (scanner->yylength() <= maxTokenLength)
            {
                posIncrAtt->setPositionIncrement(posIncr);
                scanner->getText(termAtt);
                int32_t start = scanner->yychar();
                offsetAtt->setOffset(correctOffset(start), correctOffset(start + termAtt->length()));
                
                // This 'if' should be removed in the next release. For now, it converts invalid acronyms to HOST. 
                /// When removed, only the 'else' part should remain.
                if (tokenType == ACRONYM_DEP)
                {
                    if (replaceInvalidAcronym)
                    {
                        typeAtt->setType(TOKEN_TYPES()[HOST]);
                        termAtt->setTermLength(termAtt->length() - 1); // remove extra '.'
                    }
                    else
                        typeAtt->setType(TOKEN_TYPES()[ACRONYM]);
                }
                else
                    typeAtt->setType(TOKEN_TYPES()[tokenType]);
                return true;
            }
            else
            {
                // When we skip a too-long term, we still increment the position increment
                ++posIncr;
            }
        }
    }
    
    void ClassicTokenizer::end()
    {
        // set final offset
        int32_t finalOffset = correctOffset(scanner->yychar() + scanner->yylength());
        offsetAtt->setOffset(finalOffset, finalOffset);
    }
    
    void ClassicTokenizer::reset(ReaderPtr input)
    {
        Tokenizer::reset(input);
        scanner->yyreset(input);
    }
    
    bool ClassicTokenizer::isReplaceInvalidAcronym()
    {
        return replaceInvalidAcronym;
    }
    
    void ClassicTokenizer::setReplaceInvalidAcronym(bool replaceInvalidAcronym)
    {
        this->replaceInvalidAcronym = replaceInvalidAcronym;
    }
}
