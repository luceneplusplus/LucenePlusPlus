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
    /// A simple class that stores Strings as char[]'s in a hash table.  Note that this 
    /// is not a general purpose class. For example, it cannot remove items from the set, 
    /// nor does it resize its hash table to be smaller, etc. It is  designed to be quick
    /// to test if a char[] is in the set without the necessity of converting it to a 
    /// String first.
    ///
    /// You must specify the required {@link Version} compatibility when creating {@link 
    /// CharArraySet}:
    /// <ul>
    ///    <li> As of 3.1, supplementary characters are properly lowercased.
    /// </ul>
    /// Before 3.1 supplementary characters could not be lowercased correctly. To use 
    /// instances of {@link CharArraySet} with the behaviour before Lucene 3.1 pass a 
    /// {@link Version} < 3.1 to the constructors.
    class LPPAPI CharArraySet : public LuceneObject
    {
    public:
        /// Create empty set.
        /// @param matchVersion compatibility match version.
        /// @param ignoreCase false if and only if the set should be case sensitive 
        /// otherwise true.
        CharArraySet(LuceneVersion::Version matchVersion, bool ignoreCase);
        
        /// Create set from a set of strings.
        /// @param matchVersion compatibility match version.
        /// @param entries a collection whose elements to be placed into the set
        /// @param ignoreCase false if and only if the set should be case sensitive 
        /// otherwise true.
        CharArraySet(LuceneVersion::Version matchVersion, HashSet<String> entries, bool ignoreCase);
        
        /// Creates a set from a Collection of objects. 
        /// @param matchVersion compatibility match version.
        /// @param entries a collection whose elements to be placed into the set
        /// @param ignoreCase false if and only if the set should be case sensitive 
        /// otherwise true.
        CharArraySet(LuceneVersion::Version matchVersion, Collection<String> entries, bool ignoreCase);
        
        /// Create empty set.
        /// @param ignoreCase false if and only if the set should be case sensitive 
        /// otherwise true.
        CharArraySet(bool ignoreCase);
        
        /// Create set from a set of strings.
        /// @param entries a collection whose elements to be placed into the set
        /// @param ignoreCase false if and only if the set should be case sensitive 
        /// otherwise true.
        /// @deprecated use {@link #CharArraySet(Version, HashSet, bool)} instead
        CharArraySet(HashSet<String> entries, bool ignoreCase);
        
        /// Creates a set from a Collection of objects. 
        /// @param entries a collection whose elements to be placed into the set
        /// @param ignoreCase false if and only if the set should be case sensitive 
        /// otherwise true.
        /// @deprecated use {@link #CharArraySet(Version, Collection, bool)} instead
        CharArraySet(Collection<String> entries, bool ignoreCase);
        
        virtual ~CharArraySet();
        
        LUCENE_CLASS(CharArraySet);
    
    protected:
        HashSet<String> entries;
        bool ignoreCase;
        
    public:
        virtual bool contains(const String& text);
        
        /// True if the length chars of text starting at offset are in the set
        virtual bool contains(const wchar_t* text, int32_t offset, int32_t length);
        
        /// Add this String into the set
        virtual bool add(const String& text);
        
        /// Add this char[] into the set.
        virtual bool add(CharArray text);
        
        virtual int32_t size();
        
        HashSet<String>::iterator begin();
        HashSet<String>::iterator end();
        
        virtual String toString();
    
    protected:
        void ConstructCharArraySet(bool ignoreCase);
    };
}

#endif
