/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _FIELDSREADER_H
#define _FIELDSREADER_H

#include "AbstractField.h"

namespace Lucene
{
    class LazyField : public AbstractField
    {
    public:
        LazyField(FieldsReaderPtr reader, const String& name, Store store, int32_t toRead, int64_t pointer, uint8_t fieldOptions);
        LazyField(FieldsReaderPtr reader, const String& name, Store store, Index index, TermVector termVector, int32_t toRead, int64_t pointer, uint8_t fieldOptions);
        virtual ~LazyField();
        
        LUCENE_CLASS(LazyField);
    
    public:
        static const uint8_t IS_BINARY; // work around for max parameters
        static const uint8_t IS_COMPRESSED; // work around for max parameters
        static const uint8_t CACHE_RESULT; // work around for max parameters
                
    protected:
        FieldsReaderWeakPtr _reader; 
        int32_t toRead;
        int64_t pointer;
        
        /// @deprecated Only kept for backward-compatibility with <3.0 indexes.
        bool isCompressed;
        bool cacheResult;
    
    public:
        /// The value of the field as a Reader, or null.  If null, the String value, binary value, or TokenStream value is used.  
        /// Exactly one of stringValue(), readerValue(), getBinaryValue(), and tokenStreamValue() must be set.
        ReaderPtr readerValue();
        
        /// The value of the field as a TokenStream, or null.  If null, the Reader value, String value, or binary value is used. 
        /// Exactly one of stringValue(), readerValue(), getBinaryValue(), and tokenStreamValue() must be set.
        TokenStreamPtr tokenStreamValue();
        
        /// The value of the field as a String, or null.  If null, the Reader value, binary value, or TokenStream value is used.  
        /// Exactly one of stringValue(), readerValue(), getBinaryValue(), and tokenStreamValue() must be set.
        String stringValue();
        
        int64_t getPointer();
        void setPointer(int64_t pointer);
        int32_t getToRead();
        void setToRead(int32_t toRead);
        
        /// Return the raw byte[] for the binary field.
        virtual ByteArray getBinaryValue(ByteArray result);
        
    protected:
        IndexInputPtr getFieldStream();
    };
}

#endif
