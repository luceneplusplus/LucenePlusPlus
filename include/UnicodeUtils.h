/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneObject.h"

namespace Lucene
{
	class LPPAPI UnicodeUtil
	{
	public:
		virtual ~UnicodeUtil();
	
	public:
        #ifdef LPP_UNICODE_CHAR_SIZE_2
        static const int32_t UNICODE_SUR_HIGH_START;
        static const int32_t UNICODE_SUR_HIGH_END;
        static const int32_t UNICODE_SUR_LOW_START;
        static const int32_t UNICODE_SUR_LOW_END;
        #endif

        static const wchar_t UNICODE_REPLACEMENT_CHAR;
        static const wchar_t UNICODE_TERMINATOR;
		
    public:
        /// Convert uft8 buffer into unicode.
        static int32_t toUnicode(const uint8_t* utf8, int32_t length, wchar_t* unicode);

        /// Convert uft16 buffer into unicode.
        static int32_t utf16ToUnicode(const uint8_t* utf16, int32_t length, wchar_t* unicode);

        /// Convert unicode buffer into uft8.
        static int32_t toUTF8(const uint8_t* unicode, int32_t length, uint8_t* utf8);

        /// Return true if supplied character is non-spacing (as defined by IEEE Std 1002.1-2001)
        static bool isNonSpacing(wchar_t c);
	};
	
	/// Utility class that contains utf8 and unicode translations.
	template <typename TYPE> 
	class TranslationResult : public LuceneObject
	{
	public:
		TranslationResult()
		{
			result = Array<TYPE>::newInstance(10);
			length = 0;
		}
		
	public:
		Array<TYPE> result;
		int32_t length;
	
	public:
		void setLength(int32_t length)
		{
			if (!result)
				result = Array<TYPE>::newInstance((int32_t)(1.5 * (double)length));
			if (result.length() < length)
				result.resize((int32_t)(1.5 * (double)length));
			this->length = length;
		}
		
		void copyText(const TranslationResult& other)
		{
			setLength(other.length);
			MiscUtils::arrayCopy(other.result.get(), 0, result.get(), 0, other.length);
		}
		
		void copyText(boost::shared_ptr< TranslationResult<TYPE> > other)
		{
			copyText(*other);
		}
	};

	class LPPAPI UTF8Result : public TranslationResult<uint8_t>
	{
	public:
		virtual ~UTF8Result();
	};
	
	class LPPAPI UnicodeResult : public TranslationResult<wchar_t>
	{
	public:
		virtual ~UnicodeResult();
	};	
}
