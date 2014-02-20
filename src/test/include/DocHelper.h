/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef DOCHELPER_H
#define DOCHELPER_H

#include "test_lucene.h"

namespace Lucene {

class DocHelper {
public:
    DocHelper();
    virtual ~DocHelper();

public:
    static const wchar_t* FIELD_1_TEXT;
    static const wchar_t* TEXT_FIELD_1_KEY;
    static FieldPtr textField1;

    static const wchar_t* FIELD_2_TEXT;
    static const int32_t FIELD_2_FREQS[];
    static const wchar_t* TEXT_FIELD_2_KEY;
    static FieldPtr textField2;

    static const wchar_t* FIELD_3_TEXT;
    static const wchar_t* TEXT_FIELD_3_KEY;
    static FieldPtr textField3;

    static const wchar_t* KEYWORD_TEXT;
    static const wchar_t* KEYWORD_FIELD_KEY;
    static FieldPtr keyField;

    static const wchar_t* NO_NORMS_TEXT;
    static const wchar_t* NO_NORMS_KEY;
    static FieldPtr noNormsField;

    static const wchar_t* NO_TF_TEXT;
    static const wchar_t* NO_TF_KEY;
    static FieldPtr noTFField;

    static const wchar_t* UNINDEXED_FIELD_TEXT;
    static const wchar_t* UNINDEXED_FIELD_KEY;
    static FieldPtr unIndField;

    static const wchar_t* UNSTORED_1_FIELD_TEXT;
    static const wchar_t* UNSTORED_FIELD_1_KEY;
    static FieldPtr unStoredField1;

    static const wchar_t* UNSTORED_2_FIELD_TEXT;
    static const wchar_t* UNSTORED_FIELD_2_KEY;
    static FieldPtr unStoredField2;

    static const wchar_t* LAZY_FIELD_BINARY_KEY;
    static ByteArray LAZY_FIELD_BINARY_BYTES;
    static FieldPtr lazyFieldBinary;

    static const wchar_t* LAZY_FIELD_KEY;
    static const wchar_t* LAZY_FIELD_TEXT;
    static FieldPtr lazyField;

    static const wchar_t* LARGE_LAZY_FIELD_KEY;
    static String LARGE_LAZY_FIELD_TEXT;
    static FieldPtr largeLazyField;

    static const uint8_t _FIELD_UTF1_TEXT[];
    static const String FIELD_UTF1_TEXT;
    static const wchar_t* TEXT_FIELD_UTF1_KEY;
    static FieldPtr textUtfField1;

    static const uint8_t _FIELD_UTF2_TEXT[];
    static const String FIELD_UTF2_TEXT;
    static const int32_t FIELD_UTF2_FREQS[];
    static const wchar_t* TEXT_FIELD_UTF2_KEY;
    static FieldPtr textUtfField2;

    static MapStringString nameValues;

    static Collection<FieldPtr> fields;
    static MapStringField all;
    static MapStringField indexed;
    static MapStringField stored;
    static MapStringField unstored;
    static MapStringField unindexed;
    static MapStringField termvector;
    static MapStringField notermvector;
    static MapStringField lazy;
    static MapStringField noNorms;
    static MapStringField noTf;

public:
    /// Adds the fields above to a document
    void setupDoc(const DocumentPtr& doc);

    /// Writes the document to the directory using a segment named "test"; returns the SegmentInfo describing the new segment
    SegmentInfoPtr writeDoc(const DirectoryPtr& dir, const DocumentPtr& doc);

    /// Writes the document to the directory using the analyzer and the similarity score; returns the SegmentInfo describing the new segment
    SegmentInfoPtr writeDoc(const DirectoryPtr& dir, const AnalyzerPtr& analyzer, const SimilarityPtr& similarity, const DocumentPtr& doc);

    int32_t numFields(const DocumentPtr& doc);

protected:
    /// One-time setup to initialise static members
    void setup();
};

}

#endif
