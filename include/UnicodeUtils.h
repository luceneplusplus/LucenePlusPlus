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
		static const int32_t UNI_SUR_HIGH_START;
		static const int32_t UNI_SUR_HIGH_END;
		static const int32_t UNI_SUR_LOW_START;
		static const int32_t UNI_SUR_LOW_END;
		static const wchar_t UNI_REPLACEMENT_CHAR;
		
		static const int64_t UNI_MAX_BMP;
		
		static const int32_t HALF_BASE;
		static const int64_t HALF_SHIFT;
		static const int64_t HALF_MASK;
		
		static const wchar_t UNICODE_TERMINATOR;

    public:
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
