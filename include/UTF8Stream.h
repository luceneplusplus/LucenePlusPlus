/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneObject.h"

namespace Lucene
{
	class LPPAPI UTF8Stream : public LuceneObject
	{
	public:
	    UTF8Stream(ReaderPtr reader);
		virtual ~UTF8Stream();
		
		LUCENE_CLASS(UTF8Stream);
    
    protected:
        ReaderPtr reader;
    
    public:
        static const uint16_t LEAD_SURROGATE_MIN;
        static const uint16_t LEAD_SURROGATE_MAX;
        static const uint16_t TRAIL_SURROGATE_MIN;
        static const uint16_t TRAIL_SURROGATE_MAX;
        static const uint16_t LEAD_OFFSET;
        static const uint32_t SURROGATE_OFFSET;
        static const uint32_t CODE_POINT_MAX;
        
        static const wchar_t UNICODE_REPLACEMENT_CHAR;
        static const wchar_t UNICODE_TERMINATOR;
    
    protected:
        inline uint32_t readNext();
        inline uint8_t mask8(uint32_t b);
        inline uint16_t mask16(uint32_t c);
        inline bool isTrail(uint32_t b);
        inline bool isSurrogate(uint32_t cp);
        inline bool isLeadSurrogate(uint32_t cp);
        inline bool isTrailSurrogate(uint32_t cp);
        inline bool isValidCodePoint(uint32_t cp);
        inline bool isOverlongSequence(uint32_t cp, int32_t length);
	};
	
	class LPPAPI UTF8Encoder : public UTF8Stream
	{
	public:
	    UTF8Encoder(ReaderPtr reader);
		virtual ~UTF8Encoder();
		
		LUCENE_CLASS(UTF8Encoder);
    
    public:
        int32_t encode(uint8_t* utf8, int32_t length);
        
        int32_t utf16to8(uint8_t* utf8, int32_t length);
        int32_t utf32to8(uint8_t* utf8, int32_t length);
        
    protected:
        uint8_t* appendChar(uint8_t* utf8, uint32_t cp);
	};
	
	class LPPAPI UTF8Decoder : public UTF8Stream
	{
	public:
	    UTF8Decoder(ReaderPtr reader);
		virtual ~UTF8Decoder();
		
		LUCENE_CLASS(UTF8Decoder);
    
    public:
        int32_t decode(wchar_t* unicode, int32_t length);
        
        int32_t utf8to16(wchar_t* unicode, int32_t length);
        int32_t utf8to32(wchar_t* unicode, int32_t length);
    
    protected:
        int32_t sequenceLength(uint32_t cp);
        bool getSequence(uint32_t& cp, int32_t length);
        bool isValidNext(uint32_t& cp);
	};
	
	class LPPAPI UTF16Decoder : public UTF8Stream
	{
	public:
	    UTF16Decoder(ReaderPtr reader);
		virtual ~UTF16Decoder();
		
		LUCENE_CLASS(UTF16Decoder);
    
    public:
        int32_t decode(wchar_t* unicode, int32_t length);
        
        int32_t utf16to16(wchar_t* unicode, int32_t length);
        int32_t utf16to32(wchar_t* unicode, int32_t length);
	};
}
