/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CHARARRAYSET_H
#define CHARARRAYSET_H

#include "LuceneObject.h"

namespace Lucene
{
    /// A simple class that stores Strings as char[]'s in a hash table.  Note that this is not a general purpose class.
    /// For example, it cannot remove items from the set, nor does it resize its hash table to be smaller, etc.  It is
    /// designed to be quick to test if a char[] is in the set without the necessity of converting it to a String first.
    class LPPAPI Collection<CharArray>Set : public LuceneObject
    {
    public:
        Collection<CharArray>Set(bool ignoreCase);

        /// Create set from a set of strings.
        Collection<CharArray>Set(SetString entries, bool ignoreCase);

        /// Create set from a collection of strings.
        Collection<CharArray>Set(Collection<String> entries, bool ignoreCase);

        virtual ~Collection<CharArray>Set();

        LUCENE_CLASS(Collection<CharArray>Set);

    protected:
        SetString entries;
        bool ignoreCase;

    public:
        virtual bool contains(const String& text);

        /// True if the length chars of text starting at offset are in the set
        virtual bool contains(const wchar_t* text, int32_t offset, int32_t length);

        /// Add this String into the set
        virtual bool add(const String& text);

        /// Add this char[] into the set.
        virtual bool add(Collection<CharArray> text);

        virtual int32_t size();
        virtual bool isEmpty();

        SetString::iterator begin();
        SetString::iterator end();
    };
}

#endif
