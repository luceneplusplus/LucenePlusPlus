/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UnicodeUtils.h"

extern "C"
{
#include "wcwidth.h"
}

namespace Lucene
{
    const int32_t UnicodeUtil::UNI_SUR_HIGH_START = 0xd800;
	const int32_t UnicodeUtil::UNI_SUR_HIGH_END = 0xdbff;
	const int32_t UnicodeUtil::UNI_SUR_LOW_START = 0xdc00;
	const int32_t UnicodeUtil::UNI_SUR_LOW_END = 0xdfff;
	const wchar_t UnicodeUtil::UNI_REPLACEMENT_CHAR = (wchar_t)0xfffd;
	const wchar_t UnicodeUtil::UNICODE_TERMINATOR = (wchar_t)0xffff;
	
    UnicodeUtil::~UnicodeUtil()
    {
    }
    
    bool UnicodeUtil::isNonSpacing(wchar_t c)
    {
        return (wcwidth_ucs(c) == 0);
    }
    
    UTF8Result::~UTF8Result()
    {
    }
    
    UnicodeResult::~UnicodeResult()
    {
    }
}
