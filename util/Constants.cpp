/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Constants.h"

namespace Lucene
{
    #if defined(linux) || defined(__linux) || defined(__linux__)
    String Constants::OS_NAME = L"Linux";
    #elif defined(sun) || defined(__sun)
    String Constants::OS_NAME = L"Sun";
    #elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    String Constants::OS_NAME = L"Windows";
    #elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
    String Constants::OS_NAME = L"Mac";
    #elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    String Constants::OS_NAME = L"BSD";
    #endif
    
    String Constants::LUCENE_MAIN_VERSION = L"3.0.2";
    String Constants::LUCENE_VERSION = L"3.0.2";
    
    Constants::Constants()
    {
        // private
    }
    
    Constants::~Constants()
    {
    }
    
    LuceneVersion::LuceneVersion()
    {
        // private
    }
    
    LuceneVersion::~LuceneVersion()
    {
    }
    
    bool LuceneVersion::onOrAfter(LuceneVersion::Version first, LuceneVersion::Version second)
    {
        return (first >= second);
    }
}
