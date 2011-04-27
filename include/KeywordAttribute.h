/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef KEYWORDATTRIBUTE_H
#define KEYWORDATTRIBUTE_H

#include "Attribute.h"

namespace Lucene
{
    /// This attribute can be used to mark a token as a keyword. Keyword aware {@link TokenStream}s can decide to 
    /// modify a token based on the return value of {@link #isKeyword()} if the token is modified. Stemming filters 
    /// for instance can use this attribute to conditionally skip a term if {@link #isKeyword()} returns true.
    class LPPAPI KeywordAttribute : public Attribute
    {
    public:
        KeywordAttribute();
        virtual ~KeywordAttribute();
        
        LUCENE_CLASS(KeywordAttribute);
    
    protected:
        bool keyword;
    
    public:
        virtual String toString();
        
        virtual void clear();
        virtual void copyTo(AttributePtr target);
        virtual int32_t hashCode();
        virtual bool equals(LuceneObjectPtr other);
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        
        /// @return true if the current token is a keyword, otherwise
        virtual bool isKeyword();
        
        /// Marks the current token as keyword if set to true.
        /// @param isKeyword true if the current token is a keyword, otherwise false.
        virtual void setKeyword(bool isKeyword);
    };
}

#endif
