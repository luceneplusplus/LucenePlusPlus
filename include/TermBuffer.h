/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef TERMBUFFER_H
#define TERMBUFFER_H

#include "LuceneObject.h"

namespace Lucene
{
    class LPPAPI TermBuffer : public LuceneObject
    {
    public:
        TermBuffer();
        virtual ~TermBuffer();
        
        LUCENE_CLASS(TermBuffer);
            
    protected:
        String field;
        TermPtr term; // cached
        bool preUTF8Strings; // true if strings are stored in modified UTF8 encoding
        
        UnicodeResultPtr text;
        UTF8ResultPtr bytes;
    
    public:
        virtual int32_t compareTo(LuceneObjectPtr other);
        
        /// Call this if the IndexInput passed to {@link #read} stores terms in the "modified UTF8" format.
        void setPreUTF8Strings();
        
        void read(IndexInputPtr input, FieldInfosPtr fieldInfos);
        
        void set(TermPtr term);
        void set(TermBufferPtr other);
        void reset();
        
        TermPtr toTerm();
        
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
    
    protected:
        int32_t compareChars(wchar_t* chars1, int32_t len1, wchar_t* chars2, int32_t len2);
    };
}

#endif
