/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "Token.h"
#include "Payload.h"
#include "TermAttribute.h"
#include "OffsetAttribute.h"
#include "PositionIncrementAttribute.h"
#include "PayloadAttribute.h"
#include "FlagsAttribute.h"
#include "TypeAttribute.h"
#include "MiscUtils.h"
#include "StringUtils.h"

namespace Lucene
{
    /// the default type
    const String Token::DEFAULT_TYPE = L"word";
    
    Token::Token()
    {
        ConstructToken(0, 0, DEFAULT_TYPE, 0);
    }
    
    Token::Token(int32_t start, int32_t end)
    {
        ConstructToken(start, end, DEFAULT_TYPE, 0);
    }
    
    Token::Token(int32_t start, int32_t end, const String& type)
    {
        ConstructToken(start, end, type, 0);
    }
    
    Token::Token(int32_t start, int32_t end, int32_t flags)
    {
        ConstructToken(start, end, DEFAULT_TYPE, flags);
    }
    
    Token::Token(const String& text, int32_t start, int32_t end)
    {
        ConstructToken(start, end, DEFAULT_TYPE, 0);
        append(text);
    }
    
    Token::Token(const String& text, int32_t start, int32_t end, const String& type)
    {
        ConstructToken(start, end, type, 0);
        append(text);
    }
    
    Token::Token(const String& text, int32_t start, int32_t end, int32_t flags)
    {
        ConstructToken(start, end, DEFAULT_TYPE, flags);
        append(text);
    }
    
    Token::Token(CharArray startTermBuffer, int32_t termBufferOffset, int32_t termBufferLength, int32_t start, int32_t end)
    {
        ConstructToken(start, end, DEFAULT_TYPE, 0);
        copyBuffer(startTermBuffer.get(), termBufferOffset, termBufferLength);
    }
    
    Token::~Token()
    {
    }
    
    void Token::ConstructToken(int32_t start, int32_t end, const String& type, int32_t flags)
    {
        this->_startOffset = start;
        this->_endOffset = end;
        this->_type = type;
        this->flags = flags;
        this->positionIncrement = 1;
    }
    
    void Token::setPositionIncrement(int32_t positionIncrement)
    {
        if (positionIncrement < 0)
            boost::throw_exception(IllegalArgumentException(L"Increment must be zero or greater: " + StringUtils::toString(positionIncrement)));
        this->positionIncrement = positionIncrement;
    }
    
    int32_t Token::getPositionIncrement()
    {
        return positionIncrement;
    }
    
    int32_t Token::startOffset()
    {
        return _startOffset;
    }
    
    void Token::setStartOffset(int32_t offset)
    {
        this->_startOffset = offset;
    }
    
    int32_t Token::endOffset()
    {
        return _endOffset;
    }
    
    void Token::setEndOffset(int32_t offset)
    {
        this->_endOffset = offset;
    }
    
    void Token::setOffset(int32_t startOffset, int32_t endOffset)
    {
        this->_startOffset = startOffset;
        this->_endOffset = endOffset;
    }
    
    String Token::type()
    {
        return _type;
    }
    
    void Token::setType(const String& type)
    {
        this->_type = type;
    }
    
    int32_t Token::getFlags()
    {
        return flags;
    }
    
    void Token::setFlags(int32_t flags)
    {
        this->flags = flags;
    }
    
    PayloadPtr Token::getPayload()
    {
        return this->payload;
    }
    
    void Token::setPayload(PayloadPtr payload)
    {
        this->payload = payload;
    }
    
    void Token::clear()
    {
        TermAttribute::clear();
        payload.reset();
        positionIncrement = 1;
        flags = 0;
        _startOffset = 0;
        _endOffset = 0;
        _type = DEFAULT_TYPE;
    }
    
    LuceneObjectPtr Token::clone(LuceneObjectPtr other)
    {
        LuceneObjectPtr clone = TermAttribute::clone(other ? other : newLucene<Token>());
        TokenPtr cloneToken(boost::static_pointer_cast<Token>(clone));
        cloneToken->_termLength = _termLength;
        cloneToken->_startOffset = _startOffset;
        cloneToken->_endOffset = _endOffset;
        cloneToken->_type = _type;
        cloneToken->flags = flags;
        cloneToken->positionIncrement = positionIncrement;
        
        // Do a deep clone
        if (payload)
            cloneToken->payload = boost::static_pointer_cast<Payload>(payload->clone());
            
        return cloneToken;
    }
    
    TokenPtr Token::clone(CharArray newTermBuffer, int32_t newTermOffset, int32_t newTermLength, int32_t newStartOffset, int32_t newEndOffset)
    {
        TokenPtr clone(newLucene<Token>(newTermBuffer, newTermOffset, newTermLength, newStartOffset, newEndOffset));
        clone->positionIncrement = positionIncrement;
        clone->flags = flags;
        clone->_type = _type;
        if (payload)
            clone->payload = boost::static_pointer_cast<Payload>(payload->clone());
        return clone;
    }
    
    bool Token::equals(LuceneObjectPtr other)
    {
        if (LuceneObject::equals(other))
            return true;
        
        TokenPtr otherToken(boost::dynamic_pointer_cast<Token>(other));
        if (otherToken)
        {
            return (_startOffset == otherToken->_startOffset && _endOffset == otherToken->_endOffset && 
                    flags == otherToken->flags && positionIncrement == otherToken->positionIncrement &&
                    (_type.empty() ? otherToken->_type.empty() : _type == otherToken->_type) &&
                    (!payload ? !otherToken->payload : payload->equals(otherToken->payload)) &&
                    TermAttribute::equals(otherToken));
        }
        else
            return false;
    }
    
    int32_t Token::hashCode()
    {
        int32_t code = TermAttribute::hashCode();
        code = code * 31 + _startOffset;
        code = code * 31 + _endOffset;
        code = code * 31 + flags;
        code = code * 31 + positionIncrement;
        if (!_type.empty())
            code = code * 31 + StringUtils::hashCode(_type);
        if (payload)
            code = code * 31 + payload->hashCode();
        return code;
    }
    
    void Token::clearNoTermBuffer()
    {
        payload.reset();
        positionIncrement = 1;
        flags = 0;
        _startOffset = 0;
        _endOffset = 0;
        _type = DEFAULT_TYPE;
    }
    
    TokenPtr Token::reinit(CharArray newTermBuffer, int32_t newTermOffset, int32_t newTermLength, int32_t newStartOffset, int32_t newEndOffset, const String& newType)
    {
        clearNoTermBuffer();
        copyBuffer(newTermBuffer.get(), newTermOffset, newTermLength);
        payload.reset();
        positionIncrement = 1;
        _startOffset = newStartOffset;
        _endOffset = newEndOffset;
        _type = newType;
        return shared_from_this();
    }
    
    TokenPtr Token::reinit(CharArray newTermBuffer, int32_t newTermOffset, int32_t newTermLength, int32_t newStartOffset, int32_t newEndOffset)
    {
        clearNoTermBuffer();
        copyBuffer(newTermBuffer.get(), newTermOffset, newTermLength);
        _startOffset = newStartOffset;
        _endOffset = newEndOffset;
        _type = DEFAULT_TYPE;
        return shared_from_this();
    }
    
    TokenPtr Token::reinit(const String& newTerm, int32_t newStartOffset, int32_t newEndOffset, const String& newType)
    {
        clear();
        append(newTerm);
        _startOffset = newStartOffset;
        _endOffset = newEndOffset;
        _type = newType;
        return shared_from_this();
    }
    
    TokenPtr Token::reinit(const String& newTerm, int32_t newTermOffset, int32_t newTermLength, int32_t newStartOffset, int32_t newEndOffset, const String& newType)
    {
        clear();
        append(newTerm, newTermOffset, newTermOffset + newTermLength);
        _startOffset = newStartOffset;
        _endOffset = newEndOffset;
        _type = newType;
        return shared_from_this();
    }
    
    TokenPtr Token::reinit(const String& newTerm, int32_t newStartOffset, int32_t newEndOffset)
    {
        clear();
        append(newTerm);
        _startOffset = newStartOffset;
        _endOffset = newEndOffset;
        _type = DEFAULT_TYPE;
        return shared_from_this();
    }
    
    TokenPtr Token::reinit(const String& newTerm, int32_t newTermOffset, int32_t newTermLength, int32_t newStartOffset, int32_t newEndOffset)
    {
        clear();
        append(newTerm, newTermOffset, newTermOffset + newTermLength);
        _startOffset = newStartOffset;
        _endOffset = newEndOffset;
        _type = DEFAULT_TYPE;
        return shared_from_this();
    }
    
    void Token::reinit(TokenPtr prototype)
    {
        copyBuffer(prototype->buffer().get(), 0, prototype->length());
        positionIncrement = prototype->positionIncrement;
        flags = prototype->flags;
        _startOffset = prototype->_startOffset;
        _endOffset = prototype->_endOffset;
        _type = prototype->_type;
        payload = prototype->payload;
    }
    
    void Token::reinit(TokenPtr prototype, const String& newTerm)
    {
        setEmpty()->append(newTerm);
        positionIncrement = prototype->positionIncrement;
        flags = prototype->flags;
        _startOffset = prototype->_startOffset;
        _endOffset = prototype->_endOffset;
        _type = prototype->_type;
        payload = prototype->payload;
    }
    
    void Token::reinit(TokenPtr prototype, CharArray newTermBuffer, int32_t offset, int32_t length)
    {
        copyBuffer(newTermBuffer.get(), offset, length);
        positionIncrement = prototype->positionIncrement;
        flags = prototype->flags;
        _startOffset = prototype->_startOffset;
        _endOffset = prototype->_endOffset;
        _type = prototype->_type;
        payload = prototype->payload;
    }
    
    void Token::copyTo(AttributePtr target)
    {
        TokenPtr targetToken(boost::dynamic_pointer_cast<Token>(target));
        if (targetToken)
        {
            targetToken->reinit(shared_from_this());
            // reinit shares the payload, so clone it
            if (payload)
                targetToken->payload = boost::static_pointer_cast<Payload>(payload->clone());
        }
        else
        {
            TermAttribute::copyTo(target);
            OffsetAttributePtr targetOffsetAttribute(boost::dynamic_pointer_cast<OffsetAttribute>(target));
            if (targetOffsetAttribute)
                targetOffsetAttribute->setOffset(_startOffset, _endOffset);
            PositionIncrementAttributePtr targetPositionIncrementAttribute(boost::dynamic_pointer_cast<PositionIncrementAttribute>(target));
            if (targetPositionIncrementAttribute)
                targetPositionIncrementAttribute->setPositionIncrement(positionIncrement);
            PayloadAttributePtr targetPayloadAttribute(boost::dynamic_pointer_cast<PayloadAttribute>(target));
            if (targetPayloadAttribute)
                targetPayloadAttribute->setPayload(payload ? boost::dynamic_pointer_cast<Payload>(payload->clone()) : PayloadPtr());
            FlagsAttributePtr targetFlagsAttribute(boost::dynamic_pointer_cast<FlagsAttribute>(target));
            if (targetFlagsAttribute)
                targetFlagsAttribute->setFlags(flags);
            TypeAttributePtr targetTypeAttribute(boost::dynamic_pointer_cast<TypeAttribute>(target));
            if (targetTypeAttribute)
                targetTypeAttribute->setType(_type);
        }
    }
    
    AttributeFactoryPtr Token::TOKEN_ATTRIBUTE_FACTORY()
    {
        static AttributeFactoryPtr _TOKEN_ATTRIBUTE_FACTORY;
        if (!_TOKEN_ATTRIBUTE_FACTORY)
        {
            _TOKEN_ATTRIBUTE_FACTORY = newLucene<TokenAttributeFactory>(AttributeFactory::DEFAULT_ATTRIBUTE_FACTORY());
            CycleCheck::addStatic(_TOKEN_ATTRIBUTE_FACTORY);
        }
        return _TOKEN_ATTRIBUTE_FACTORY;
    }
    
    TokenAttributeFactory::TokenAttributeFactory(AttributeFactoryPtr delegate)
    {
        this->delegate = delegate;
    }
    
    TokenAttributeFactory::~TokenAttributeFactory()
    {
    }
    
    AttributePtr TokenAttributeFactory::createAttributeInstance(const String& className)
    {
        return newLucene<Token>();
    }
    
    bool TokenAttributeFactory::equals(LuceneObjectPtr other)
    {
        if (AttributeFactory::equals(other))
            return true;
        
        TokenAttributeFactoryPtr otherTokenAttributeFactory(boost::dynamic_pointer_cast<TokenAttributeFactory>(other));
        if (otherTokenAttributeFactory)
            return this->delegate->equals(otherTokenAttributeFactory->delegate);
        return false;
    }
    
    int32_t TokenAttributeFactory::hashCode()
    {
        return (delegate->hashCode() ^ 0x0a45aa31);
    }
}
