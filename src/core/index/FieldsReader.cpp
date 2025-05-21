/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "FieldsReader.h"
#include "BufferedIndexInput.h"
#include "IndexFileNames.h"
#include "FieldsWriter.h"
#include "FieldInfos.h"
#include "FieldInfo.h"
#include "FieldSelector.h"
#include "Directory.h"
#include "Document.h"
#include "Field.h"
#include "CompressionTools.h"
#include "MiscUtils.h"
#include "StringUtils.h"
#include "VariantUtils.h"

namespace Lucene {

FieldsReader::FieldsReader(const FieldInfosPtr& fieldInfos, int32_t numTotalDocs, int32_t size, int32_t format,
                           int32_t formatSize, int32_t docStoreOffset, const IndexInputPtr& cloneableFieldsStream,
                           const IndexInputPtr& cloneableIndexStream) {
    closed = false;
    isOriginal = false;
    this->fieldInfos = fieldInfos;
    this->numTotalDocs = numTotalDocs;
    this->_size = size;
    this->format = format;
    this->formatSize = formatSize;
    this->docStoreOffset = docStoreOffset;
    this->cloneableFieldsStream = cloneableFieldsStream;
    this->cloneableIndexStream = cloneableIndexStream;
    fieldsStream = boost::dynamic_pointer_cast<IndexInput>(cloneableFieldsStream->clone());
    indexStream = boost::dynamic_pointer_cast<IndexInput>(cloneableIndexStream->clone());
}

FieldsReader::FieldsReader(const DirectoryPtr& d, const String& segment, const FieldInfosPtr& fn) {
    ConstructReader(d, segment, fn, BufferedIndexInput::BUFFER_SIZE, -1, 0);
}

FieldsReader::FieldsReader(const DirectoryPtr& d, const String& segment, const FieldInfosPtr& fn, int32_t readBufferSize, int32_t docStoreOffset, int32_t size) {
    ConstructReader(d, segment, fn, readBufferSize, docStoreOffset, size);
}

FieldsReader::~FieldsReader() {
}

void FieldsReader::ConstructReader(const DirectoryPtr& d, const String& segment, const FieldInfosPtr& fn, int32_t readBufferSize, int32_t docStoreOffset, int32_t size) {
    bool success = false;
    isOriginal = true;
    numTotalDocs = 0;
    _size = 0;
    closed = false;
    format = 0;
    formatSize = 0;
    docStoreOffset = docStoreOffset;
    LuceneException finally;
    try {
        fieldInfos = fn;

        cloneableFieldsStream = d->openInput(segment + L"." + IndexFileNames::FIELDS_EXTENSION(), readBufferSize);
        cloneableIndexStream = d->openInput(segment + L"." + IndexFileNames::FIELDS_INDEX_EXTENSION(), readBufferSize);

        // First version of fdx did not include a format header, but, the first int will always be 0 in that case
        format = cloneableIndexStream->readInt();

        if (format > FieldsWriter::FORMAT_CURRENT) {
            boost::throw_exception(CorruptIndexException(L"Incompatible format version: " + StringUtils::toString(format) +
                                   L" expected " + StringUtils::toString(FieldsWriter::FORMAT_CURRENT) +
                                   L" or lower"));
        }

        formatSize = format > FieldsWriter::FORMAT ? 4 : 0;

        if (format < FieldsWriter::FORMAT_VERSION_UTF8_LENGTH_IN_BYTES) {
            cloneableFieldsStream->setModifiedUTF8StringsMode();
        }

        fieldsStream = boost::dynamic_pointer_cast<IndexInput>(cloneableFieldsStream->clone());

        int64_t indexSize = cloneableIndexStream->length() - formatSize;

        if (docStoreOffset != -1) {
            // We read only a slice out of this shared fields file
            this->docStoreOffset = docStoreOffset;
            this->_size = size;

            // Verify the file is long enough to hold all of our docs
            BOOST_ASSERT(((int32_t)((double)indexSize / 8.0)) >= _size + this->docStoreOffset);
        } else {
            this->docStoreOffset = 0;
            this->_size = (int32_t)(indexSize >> 3);
        }

        indexStream = boost::dynamic_pointer_cast<IndexInput>(cloneableIndexStream->clone());
        numTotalDocs = (int32_t)(indexSize >> 3);
        success = true;
    } catch (LuceneException& e) {
        finally = e;
    }
    // With lock-less commits, it's entirely possible (and fine) to hit a FileNotFound exception above.
    // In this case, we want to explicitly close any subset of things that were opened
    if (!success) {
        close();
    }
    finally.throwException();
}

LuceneObjectPtr FieldsReader::clone(const LuceneObjectPtr& other) {
    ensureOpen();
    return newLucene<FieldsReader>(fieldInfos, numTotalDocs, _size, format, formatSize, docStoreOffset, cloneableFieldsStream, cloneableIndexStream);
}

void FieldsReader::ensureOpen() {
    if (closed) {
        boost::throw_exception(AlreadyClosedException(L"this FieldsReader is closed"));
    }
}

void FieldsReader::close() {
    if (!closed) {
        if (fieldsStream) {
            fieldsStream->close();
        }
        if (isOriginal) {
            if (cloneableFieldsStream) {
                cloneableFieldsStream->close();
            }
            if (cloneableIndexStream) {
                cloneableIndexStream->close();
            }
        }
        if (indexStream) {
            indexStream->close();
        }
        fieldsStreamTL.close();
        closed = true;
    }
}

int32_t FieldsReader::size() {
    return _size;
}

void FieldsReader::seekIndex(int32_t docID) {
    indexStream->seek(formatSize + (docID + docStoreOffset) * 8);
}

bool FieldsReader::canReadRawDocs() {
    // Disable reading raw docs in 2.x format, because of the removal of compressed fields in 3.0.
    // We don't want rawDocs() to decode field bits to figure out if a field was compressed, hence
    // we enforce ordinary (non-raw) stored field merges for <3.0 indexes.
    return (format >= FieldsWriter::FORMAT_LUCENE_3_0_NO_COMPRESSED_FIELDS);
}

DocumentPtr FieldsReader::doc(int32_t n, const FieldSelectorPtr& fieldSelector) {
    seekIndex(n);
    int64_t position = indexStream->readLong();
    fieldsStream->seek(position);

    DocumentPtr doc(newLucene<Document>());
    int32_t numFields = fieldsStream->readVInt();
    for (int32_t i = 0; i < numFields; ++i) {
        int32_t fieldNumber = fieldsStream->readVInt();
        FieldInfoPtr fi = fieldInfos->fieldInfo(fieldNumber);
        FieldSelector::FieldSelectorResult acceptField = fieldSelector ? fieldSelector->accept(fi->name) : FieldSelector::SELECTOR_LOAD;

        uint8_t bits = fieldsStream->readByte();
        if (bits > FieldsWriter::FIELD_IS_COMPRESSED + FieldsWriter::FIELD_IS_TOKENIZED + FieldsWriter::FIELD_IS_BINARY) {
            bits = bits & (FieldsWriter::FIELD_IS_COMPRESSED + FieldsWriter::FIELD_IS_TOKENIZED + FieldsWriter::FIELD_IS_BINARY);
        }

        bool compressed = ((bits & FieldsWriter::FIELD_IS_COMPRESSED) != 0);

        if (compressed && format >= FieldsWriter::FORMAT_LUCENE_3_0_NO_COMPRESSED_FIELDS) {
            compressed = false;
        }

        bool tokenize = ((bits & FieldsWriter::FIELD_IS_TOKENIZED) != 0);
        bool binary = ((bits & FieldsWriter::FIELD_IS_BINARY) != 0);

        if (acceptField == FieldSelector::SELECTOR_LOAD) {
            addField(doc, fi, binary, compressed, tokenize);
        } else if (acceptField == FieldSelector::SELECTOR_LOAD_AND_BREAK) {
            addField(doc, fi, binary, compressed, tokenize);
            break; // Get out of this loop
        } else if (acceptField == FieldSelector::SELECTOR_LAZY_LOAD) {
            addFieldLazy(doc, fi, binary, compressed, tokenize);
        } else if (acceptField == FieldSelector::SELECTOR_SIZE) {
            skipField(binary, compressed, addFieldSize(doc, fi, binary, compressed));
        } else if (acceptField == FieldSelector::SELECTOR_SIZE_AND_BREAK) {
            addFieldSize(doc, fi, binary, compressed);
            break;
        } else {
            skipField(binary, compressed);
        }
    }

    return doc;
}

IndexInputPtr FieldsReader::rawDocs(Collection<int32_t> lengths, int32_t startDocID, int32_t numDocs) {
    seekIndex(startDocID);
    int64_t startOffset = indexStream->readLong();
    int64_t lastOffset = startOffset;
    int32_t count = 0;
    while (count < numDocs) {
        int32_t docID = docStoreOffset + startDocID + count + 1;
        BOOST_ASSERT(docID <= numTotalDocs);
        int64_t offset = docID < numTotalDocs ? indexStream->readLong() : fieldsStream->length();
        lengths[count++] = (int32_t)(offset - lastOffset);
        lastOffset = offset;
    }

    fieldsStream->seek(startOffset);

    return fieldsStream;
}

void FieldsReader::skipField(bool binary, bool compressed) {
    skipField(binary, compressed, fieldsStream->readVInt());
}

void FieldsReader::skipField(bool binary, bool compressed, int32_t toRead) {
    if (format >= FieldsWriter::FORMAT_VERSION_UTF8_LENGTH_IN_BYTES || binary || compressed) {
        fieldsStream->seek(fieldsStream->getFilePointer() + toRead);
    } else {
        // We need to skip chars.  This will slow us down, but still better
        fieldsStream->skipChars(toRead);
    }
}

void FieldsReader::addFieldLazy(const DocumentPtr& doc, const FieldInfoPtr& fi, bool binary, bool compressed, bool tokenize) {
    if (binary) {
        int32_t toRead = fieldsStream->readVInt();
        int64_t pointer = fieldsStream->getFilePointer();
        doc->add(newLucene<LazyField>(shared_from_this(), fi->name, Field::STORE_YES, toRead, pointer, binary, compressed));
        fieldsStream->seek(pointer + toRead);
    } else {
        Field::Store store = Field::STORE_YES;
        Field::Index index = Field::toIndex(fi->isIndexed, tokenize);
        Field::TermVector termVector = Field::toTermVector(fi->storeTermVector, fi->storeOffsetWithTermVector, fi->storePositionWithTermVector);

        AbstractFieldPtr f;
        if (compressed) {
            int32_t toRead = fieldsStream->readVInt();
            int64_t pointer = fieldsStream->getFilePointer();
            f = newLucene<LazyField>(shared_from_this(), fi->name, store, toRead, pointer, binary, compressed);
            // skip over the part that we aren't loading
            fieldsStream->seek(pointer + toRead);
            f->setOmitNorms(fi->omitNorms);
            f->setOmitTermFreqAndPositions(fi->omitTermFreqAndPositions);
        } else {
            int32_t length = fieldsStream->readVInt();
            int64_t pointer = fieldsStream->getFilePointer();
            // skip ahead of where we are by the length of what is stored
            if (format >= FieldsWriter::FORMAT_VERSION_UTF8_LENGTH_IN_BYTES) {
                fieldsStream->seek(pointer + length);
            } else {
                fieldsStream->skipChars(length);
            }
            f = newLucene<LazyField>(shared_from_this(), fi->name, store, index, termVector, length, pointer, binary, compressed);
            f->setOmitNorms(fi->omitNorms);
            f->setOmitTermFreqAndPositions(fi->omitTermFreqAndPositions);
        }

        doc->add(f);
    }
}

void FieldsReader::addField(const DocumentPtr& doc, const FieldInfoPtr& fi, bool binary, bool compressed, bool tokenize) {
    if (!doc || !fi) {
        return;
    }

    // we have a binary stored field, and it may be compressed
    if (binary) {
        try {
            int32_t toRead = fieldsStream->readVInt();
            if (toRead < 0) {
                return;
            }
            
            ByteArray b;
            try {
                b = ByteArray::newInstance(toRead);
                if (!b) {
                    return;
                }
                
                fieldsStream->readBytes(b.get(), 0, b.size());
                
                // Create field object outside the doc->add call to allow for null checking
                FieldPtr field;
                if (compressed) {
                    try {
                        ByteArray uncompressedData = uncompress(b);
                        if (uncompressedData && uncompressedData.size() > 0) {
                            field = newLucene<Field>(fi->name, uncompressedData, Field::STORE_YES);
                        }
                    } catch (...) {
                        // Fall through to use original data
                    }
                    
                    // If decompression failed or returned null, use the original binary data
                    if (!field) {
                        field = newLucene<Field>(fi->name, b, Field::STORE_YES);
                    }
                } else {
                    field = newLucene<Field>(fi->name, b, Field::STORE_YES);
                }
                
                // Add the field to document only if field creation was successful
                if (field && doc) {
                    doc->add(field);
                }
            } catch (...) {
                // Skip field on error
            }
        } catch (...) {
            // Skip field on error
        }
    } else {
        Field::Store store = Field::STORE_YES;
        Field::Index index = Field::toIndex(fi->isIndexed, tokenize);
        Field::TermVector termVector = Field::toTermVector(fi->storeTermVector, fi->storeOffsetWithTermVector, fi->storePositionWithTermVector);

        try {
            AbstractFieldPtr f;
            
            if (compressed) {
                int32_t toRead = fieldsStream->readVInt();
                if (toRead < 0) {
                    return;
                }
                
                ByteArray b;
                try {
                    b = ByteArray::newInstance(toRead);
                    if (!b) {
                        return;
                    }
                    
                    fieldsStream->readBytes(b.get(), 0, b.size());
                    
                    try {
                        String fieldValue = uncompressString(b);
                        if (!fieldValue.empty()) {
                            f = newLucene<Field>(fi->name, fieldValue, store, index, termVector);
                        } else {
                            f = newLucene<Field>(fi->name, L"", store, index, termVector);
                        }
                    } catch (...) {
                        f = newLucene<Field>(fi->name, L"", store, index, termVector);
                    }
                } catch (...) {
                    return;
                }
            } else {
                try {
                    String fieldValue = fieldsStream->readString();
                    f = newLucene<Field>(fi->name, fieldValue, store, index, termVector);
                } catch (...) {
                    f = newLucene<Field>(fi->name, L"", store, index, termVector);
                }
            }
            
            // Set field properties and add to document only if field creation was successful
            if (f && doc) {
                f->setOmitTermFreqAndPositions(fi->omitTermFreqAndPositions);
                f->setOmitNorms(fi->omitNorms);
                doc->add(f);
            }
        } catch (...) {
            // Skip field on error
        }
    }
}

int32_t FieldsReader::addFieldSize(const DocumentPtr& doc, const FieldInfoPtr& fi, bool binary, bool compressed) {
    int32_t size = fieldsStream->readVInt();
    int32_t bytesize = (binary || compressed) ? size : 2 * size;
    ByteArray sizebytes(ByteArray::newInstance(4));
    sizebytes[0] = (uint8_t)MiscUtils::unsignedShift(bytesize, 24);
    sizebytes[1] = (uint8_t)MiscUtils::unsignedShift(bytesize, 16);
    sizebytes[2] = (uint8_t)MiscUtils::unsignedShift(bytesize, 8);
    sizebytes[3] = (uint8_t)(bytesize);
    doc->add(newLucene<Field>(fi->name, sizebytes, Field::STORE_YES));
    return size;
}

ByteArray FieldsReader::uncompress(ByteArray b) {
    if (!b) {
        return ByteArray::newInstance(0);
    }
    
    try {
        if (b.size() == 0) {
            return ByteArray::newInstance(0);
        }
        
        ByteArray result = CompressionTools::decompress(b);
        if (!result) {
            return ByteArray::newInstance(0);
        }
        
        return result;
    } catch (...) {
        // Return empty array on any exception
        return ByteArray::newInstance(0);
    }
}

String FieldsReader::uncompressString(ByteArray b) {
    if (!b) {
        return L"";
    }
    
    try {
        if (b.size() == 0) {
            return L"";
        }
        
        String result = CompressionTools::decompressString(b);
        if (result.empty()) {
            return L"";
        }
        
        return result;
    } catch (...) {
        // Return empty string on any exception
        return L"";
    }
}

LazyField::LazyField(const FieldsReaderPtr& reader, const String& name, Field::Store store, int32_t toRead, int64_t pointer, bool isBinary, bool isCompressed) :
    AbstractField(name, store, Field::INDEX_NO, Field::TERM_VECTOR_NO) {
    this->_reader = reader;
    this->toRead = toRead;
    this->pointer = pointer;
    this->_isBinary = isBinary;
    if (isBinary) {
        binaryLength = toRead;
    }
    lazy = true;
    this->isCompressed = isCompressed;
}

LazyField::LazyField(const FieldsReaderPtr& reader, const String& name, Field::Store store, Field::Index index, Field::TermVector termVector, int32_t toRead, int64_t pointer, bool isBinary, bool isCompressed) :
    AbstractField(name, store, index, termVector) {
    this->_reader = reader;
    this->toRead = toRead;
    this->pointer = pointer;
    this->_isBinary = isBinary;
    if (isBinary) {
        binaryLength = toRead;
    }
    lazy = true;
    this->isCompressed = isCompressed;
}

LazyField::~LazyField() {
}

IndexInputPtr LazyField::getFieldStream() {
    FieldsReaderPtr reader(_reader);
    IndexInputPtr localFieldsStream = reader->fieldsStreamTL.get();
    if (!localFieldsStream) {
        localFieldsStream = boost::static_pointer_cast<IndexInput>(reader->cloneableFieldsStream->clone());
        reader->fieldsStreamTL.set(localFieldsStream);
    }
    return localFieldsStream;
}

ReaderPtr LazyField::readerValue() {
    FieldsReaderPtr(_reader)->ensureOpen();
    return ReaderPtr();
}

TokenStreamPtr LazyField::tokenStreamValue() {
    FieldsReaderPtr(_reader)->ensureOpen();
    return TokenStreamPtr();
}

String LazyField::stringValue() {
    FieldsReaderPtr reader(_reader);
    if (!reader) {
        return L"";
    }
    
    try {
        reader->ensureOpen();
    } catch (...) {
        return L"";
    }
    
    if (_isBinary) {
        return L"";
    } else {
        if (VariantUtils::isNull(fieldsData)) {
            IndexInputPtr localFieldsStream;
            try {
                localFieldsStream = getFieldStream();
                if (!localFieldsStream) {
                    fieldsData = String(L"");
                    return L"";
                }
            } catch (...) {
                fieldsData = String(L"");
                return L"";
            }
            
            try {
                localFieldsStream->seek(pointer);
                
                // Check for invalid size
                if (toRead <= 0) {
                    fieldsData = String(L"");
                    return L"";
                }
                
                if (isCompressed) {
                    try {
                        ByteArray b(ByteArray::newInstance(toRead));
                        if (!b) {
                            fieldsData = String(L"");
                            return L"";
                        }
                        
                        localFieldsStream->readBytes(b.get(), 0, b.size());
                        try {
                            fieldsData = reader->uncompressString(b);
                            if (VariantUtils::isNull(fieldsData)) {
                                fieldsData = String(L"");
                            }
                        } catch (...) {
                            fieldsData = String(L"");
                        }
                    } catch (...) {
                        fieldsData = String(L"");
                    }
                } else {
                    if (reader->format >= FieldsWriter::FORMAT_VERSION_UTF8_LENGTH_IN_BYTES) {
                        try {
                            ByteArray bytes(ByteArray::newInstance(toRead));
                            if (!bytes) {
                                fieldsData = String(L"");
                                return L"";
                            }
                            
                            localFieldsStream->readBytes(bytes.get(), 0, toRead);
                            try {
                                fieldsData = StringUtils::toUnicode(bytes.get(), toRead);
                                if (VariantUtils::isNull(fieldsData)) {
                                    fieldsData = String(L"");
                                }
                            } catch (...) {
                                fieldsData = String(L"");
                            }
                        } catch (...) {
                            fieldsData = String(L"");
                        }
                    } else {
                        try {
                            // read in chars because we already know the length we need to read
                            CharArray chars(CharArray::newInstance(toRead));
                            if (!chars) {
                                fieldsData = String(L"");
                                return L"";
                            }
                            
                            int32_t length = localFieldsStream->readChars(chars.get(), 0, toRead);
                            try {
                                fieldsData = String(chars.get(), length);
                                if (VariantUtils::isNull(fieldsData)) {
                                    fieldsData = String(L"");
                                }
                            } catch (...) {
                                fieldsData = String(L"");
                            }
                        } catch (...) {
                            fieldsData = String(L"");
                        }
                    }
                }
            } catch (...) {
                fieldsData = String(L"");
            }
        }
        
        try {
            return VariantUtils::get<String>(fieldsData);
        } catch (...) {
            return L"";
        }
    }
}

int64_t LazyField::getPointer() {
    FieldsReaderPtr(_reader)->ensureOpen();
    return pointer;
}

void LazyField::setPointer(int64_t pointer) {
    FieldsReaderPtr(_reader)->ensureOpen();
    this->pointer = pointer;
}

int32_t LazyField::getToRead() {
    FieldsReaderPtr(_reader)->ensureOpen();
    return toRead;
}

void LazyField::setToRead(int32_t toRead) {
    FieldsReaderPtr(_reader)->ensureOpen();
    this->toRead = toRead;
}

ByteArray LazyField::getBinaryValue(ByteArray result) {
    FieldsReaderPtr reader(_reader);
    if (!reader) {
        return ByteArray();
    }

    try {
        reader->ensureOpen();
    } catch (...) {
        return ByteArray();
    }

    if (_isBinary) {
        if (VariantUtils::isNull(fieldsData)) {
            ByteArray b;

            try {
                // Check for invalid size
                if (toRead <= 0) {
                    fieldsData = ByteArray();
                    binaryOffset = 0;
                    binaryLength = 0;
                    return ByteArray();
                }
                
                // Allocate new buffer if result is null or too small
                if (!result || result.size() < toRead) {
                    b = ByteArray::newInstance(toRead);
                    if (!b) {
                        fieldsData = ByteArray();
                        binaryOffset = 0;
                        binaryLength = 0;
                        return ByteArray();
                    }
                } else {
                    b = result;
                }

                IndexInputPtr localFieldsStream;
                try {
                    localFieldsStream = getFieldStream();
                    if (!localFieldsStream) {
                        fieldsData = ByteArray();
                        binaryOffset = 0;
                        binaryLength = 0;
                        return ByteArray();
                    }
                } catch (...) {
                    fieldsData = ByteArray();
                    binaryOffset = 0;
                    binaryLength = 0;
                    return ByteArray();
                }

                try {
                    localFieldsStream->seek(pointer);
                    localFieldsStream->readBytes(b.get(), 0, toRead);
                    if (isCompressed) {
                        try {
                            ByteArray uncompressed = reader->uncompress(b);
                            if (uncompressed && uncompressed.size() > 0) {
                                fieldsData = uncompressed;
                            } else {
                                fieldsData = b;
                            }
                        } catch (...) {
                            fieldsData = b;
                        }
                    } else {
                        fieldsData = b;
                    }
                } catch (IOException&) {
                    fieldsData = ByteArray();
                    binaryOffset = 0;
                    binaryLength = 0;
                    return ByteArray();
                } catch (...) {
                    fieldsData = ByteArray();
                    binaryOffset = 0;
                    binaryLength = 0;
                    return ByteArray();
                }

                binaryOffset = 0;
                binaryLength = toRead;
            } catch (...) {
                fieldsData = ByteArray();
                binaryOffset = 0;
                binaryLength = 0;
                return ByteArray();
            }
        }

        try {
            return VariantUtils::get<ByteArray>(fieldsData);
        } catch (...) {
            return ByteArray();
        }
    } else {
        return ByteArray();
    }
}

}
