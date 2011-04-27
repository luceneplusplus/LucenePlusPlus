/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CHARTERMATTRIBUTE_H
#define CHARTERMATTRIBUTE_H

#include "Attribute.h"

namespace Lucene
{
    /// The term text of a Token.
    class LPPAPI CharTermAttribute : public Attribute
    {
    public:
        CharTermAttribute();
        virtual ~CharTermAttribute();
        
        LUCENE_CLASS(CharTermAttribute);
    
    protected:
        static const int32_t MIN_BUFFER_SIZE;
        
        CharArray _termBuffer;
        int32_t _termLength;
    
    public:
        /// Returns the Token's term text.
        ///
        /// This method has a performance penalty because the text is stored internally in a char[].  If possible, 
        /// use {@link #termBuffer()} and {@link #termLength()} directly instead.  If you really need a String, use 
        /// this method, which is nothing more than a convenience call to new String(token.termBuffer(), 0, 
        /// token.termLength())
        virtual String term();
        
        /// Copies the contents of buffer, starting at offset for length characters, into the termBuffer array.
        /// @param buffer the buffer to copy
        /// @param offset the index in the buffer of the first character to copy
        /// @param length the number of characters to copy
        virtual void copyBuffer(const wchar_t* buffer, int32_t offset, int32_t length);
        
        /// Copies the contents of buffer, starting at offset for length characters, into the termBuffer array.
        /// @Deprecated
        virtual void setTermBuffer(const wchar_t* buffer, int32_t offset, int32_t length);
        
        /// Copies the contents of buffer into the termBuffer array.
        /// @Deprecated
        virtual void setTermBuffer(const String& buffer);
        
        /// Copies the contents of buffer, starting at offset and continuing for length characters, into the 
        /// termBuffer array.
        /// @Deprecated
        virtual void setTermBuffer(const String& buffer, int32_t offset, int32_t length);
        
        /// Returns the internal termBuffer character array which you can then directly alter.  If the array is 
        /// too small for your token, use {@link #resizeBuffer(int32_t)} to increase it.  After altering the buffer 
        /// be sure to call {@link #setLength} to record the number of valid characters that were placed into 
        /// the termBuffer.
        virtual CharArray buffer();

        /// Returns the internal termBuffer character array which you can then directly alter.  If the array is 
        /// too small for your token, use {@link #resizeTermBuffer(int)} to increase it.  After altering the buffer 
        /// be sure to call {@link #setTermLength} to record the number of valid characters that were placed into 
        /// the termBuffer.
        /// @Deprecated
        virtual CharArray termBuffer();
        
        /// Optimized implementation of termBuffer.
        /// @Deprecated
        virtual wchar_t* termBufferArray();
        
        /// Optimized implementation of termBuffer.
        virtual wchar_t* bufferArray();
        
        /// Grows the termBuffer to at least size newSize, preserving the existing content.
        /// @param newSize minimum size of the new termBuffer
        /// @return newly created termBuffer with length >= newSize
        virtual CharArray resizeBuffer(int32_t newSize);

        /// Grows the termBuffer to at least size newSize, preserving the existing content. 
        /// @Deprecated
        virtual CharArray resizeTermBuffer(int32_t newSize);
        
        /// Return number of valid characters (length of the term) in the termBuffer array.
        /// @Deprecated
        virtual int32_t termLength();

        /// Set number of valid characters (length of the term) in the termBuffer array. Use this to truncate the 
        /// termBuffer or to synchronize with external manipulation of the termBuffer.  Note: to grow the size of 
        /// the array, use {@link #resizeBuffer(int32_t)} first.
        /// @param length the truncated length
        virtual CharTermAttributePtr setLength(int32_t length);
        
        /// Sets the length of the termBuffer to zero.  Use this method before appending contents.
        virtual CharTermAttributePtr setEmpty();
        
        /// Set number of valid characters (length of the term) in the termBuffer array. 
        /// @Deprecated
        virtual void setTermLength(int32_t length);
        
        /// Return number of valid characters (length of the term) in the termBuffer array.
        virtual int32_t length();
        virtual wchar_t charAt(int32_t index);
        virtual String subSequence(int32_t start, int32_t end);
        
        /// Appends the specified {@code String} to this character sequence. 
        ///
        /// The characters of the {@code String} argument are appended, in order, increasing the length of this 
        /// sequence by the length of the argument.
        virtual CharTermAttributePtr append(const String& s);
        virtual CharTermAttributePtr append(const String& s, int32_t start, int32_t end);
        
        virtual CharTermAttributePtr append(wchar_t c);
        
        /// Appends the contents of the other {@code CharTermAttribute} to this character sequence. 
        ///
        /// The characters of the {@code CharTermAttribute} argument are appended, in order, increasing the length 
        /// of this sequence by the length of the argument.
        virtual CharTermAttributePtr append(CharTermAttributePtr termAtt);

        virtual int32_t hashCode();
        virtual void clear();
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        virtual bool equals(LuceneObjectPtr other);
        
        /// Returns solely the term text as specified by the {@link CharSequence} interface.
        ///
        /// This method changed the behavior with Lucene 3.1, before it returned a String representation of 
        /// the whole term with all attributes.  This affects especially the {@link Token} subclass.
        virtual String toString();
        
        virtual void copyTo(AttributePtr target);
        
    private:
        /// Allocates a buffer char[] of at least newSize, without preserving the existing content.  Its always 
        /// used in places that set the content.
        /// @param newSize minimum size of the buffer
        void growTermBuffer(int32_t newSize);    
    };
}

#endif
