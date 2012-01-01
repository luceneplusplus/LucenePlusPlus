/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "Collection<CharArray>Set.h"
#include "StringUtils.h"

namespace Lucene
{
    Collection<CharArray>Set::Collection<CharArray>Set(bool ignoreCase)
    {
        this->ignoreCase = ignoreCase;
        this->entries = SetString::newInstance();
    }

    Collection<CharArray>Set::Collection<CharArray>Set(SetString entries, bool ignoreCase)
    {
        this->ignoreCase = ignoreCase;
        this->entries = SetString::newInstance();
        if (entries)
        {
            for (SetString::iterator entry = entries.begin(); entry != entries.end(); ++entry)
                add(*entry);
        }
    }

    Collection<CharArray>Set::Collection<CharArray>Set(Collection<String> entries, bool ignoreCase)
    {
        this->ignoreCase = ignoreCase;
        this->entries = SetString::newInstance();
        if (entries)
        {
            for (Collection<String>::iterator entry = entries.begin(); entry != entries.end(); ++entry)
                add(*entry);
        }
    }

    Collection<CharArray>Set::~Collection<CharArray>Set()
    {
    }

    bool Collection<CharArray>Set::contains(const String& text)
    {
        return entries.contains(ignoreCase ? StringUtils::toLower(text) : text);
    }

    bool Collection<CharArray>Set::contains(const wchar_t* text, int32_t offset, int32_t length)
    {
        return contains(String(text + offset, length));
    }

    bool Collection<CharArray>Set::add(const String& text)
    {
        return entries.add(ignoreCase ? StringUtils::toLower(text) : text);
    }

    bool Collection<CharArray>Set::add(Collection<CharArray> text)
    {
        return add(String(text.get(), text.size()));
    }

    int32_t Collection<CharArray>Set::size()
    {
        return entries.size();
    }

    bool Collection<CharArray>Set::isEmpty()
    {
        return entries.empty();
    }

    SetString::iterator Collection<CharArray>Set::begin()
    {
        return entries.begin();
    }

    SetString::iterator Collection<CharArray>Set::end()
    {
        return entries.end();
    }
}
