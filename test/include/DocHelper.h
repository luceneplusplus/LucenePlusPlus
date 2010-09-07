/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "test_lucene.h"

namespace Lucene
{
	class DocHelper
	{
	public:
		DocHelper();
		virtual ~DocHelper();
	
	public:
		static const wchar_t* FIELD_1_TEXT;
		static const wchar_t* TEXT_FIELD_1_KEY;
		FieldPtr textField1;
		
		static const wchar_t* FIELD_2_TEXT;
		static const int32_t FIELD_2_FREQS[];
		static const wchar_t* TEXT_FIELD_2_KEY;
		FieldPtr textField2;
		
		static const wchar_t* FIELD_3_TEXT;
		static const wchar_t* TEXT_FIELD_3_KEY;
		FieldPtr textField3;
		
		static const wchar_t* KEYWORD_TEXT;
		static const wchar_t* KEYWORD_FIELD_KEY;
		FieldPtr keyField;
		
		static const wchar_t* NO_NORMS_TEXT;
		static const wchar_t* NO_NORMS_KEY;
		FieldPtr noNormsField;
		
		static const wchar_t* NO_TF_TEXT;
		static const wchar_t* NO_TF_KEY;
		FieldPtr noTFField;
		
		static const wchar_t* UNINDEXED_FIELD_TEXT;
		static const wchar_t* UNINDEXED_FIELD_KEY;
		FieldPtr unIndField;
		
		static const wchar_t* UNSTORED_1_FIELD_TEXT;
		static const wchar_t* UNSTORED_FIELD_1_KEY;
		FieldPtr unStoredField1;
		
		static const wchar_t* UNSTORED_2_FIELD_TEXT;
		static const wchar_t* UNSTORED_FIELD_2_KEY;
		FieldPtr unStoredField2;
		
		static const wchar_t* LAZY_FIELD_BINARY_KEY;
		ByteArray LAZY_FIELD_BINARY_BYTES;
		FieldPtr lazyFieldBinary;
		
		static const wchar_t* LAZY_FIELD_KEY;
		static const wchar_t* LAZY_FIELD_TEXT;
		FieldPtr lazyField;
		
		static const wchar_t* LARGE_LAZY_FIELD_KEY;
		static String LARGE_LAZY_FIELD_TEXT;
		FieldPtr largeLazyField;
		
		static const uint8_t _FIELD_UTF1_TEXT[];
		static const String FIELD_UTF1_TEXT;
		static const wchar_t* TEXT_FIELD_UTF1_KEY;
		FieldPtr textUtfField1;
		
		static const uint8_t _FIELD_UTF2_TEXT[];
		static const String FIELD_UTF2_TEXT;
		static const int32_t FIELD_UTF2_FREQS[];
		static const wchar_t* TEXT_FIELD_UTF2_KEY;
		FieldPtr textUtfField2;
		
		MapStringString nameValues;
		
		Collection<FieldPtr> fields;
		MapStringField all;
		MapStringField indexed;
		MapStringField stored;
		MapStringField unstored;
		MapStringField unindexed;
		MapStringField termvector;
		MapStringField notermvector;
		MapStringField lazy;
		MapStringField noNorms;
		MapStringField noTf;
	
	public:
		/// Adds the fields above to a document
		void setupDoc(DocumentPtr doc);
		
		/// Writes the document to the directory using a segment named "test"; returns the SegmentInfo describing the new segment
		SegmentInfoPtr writeDoc(DirectoryPtr dir, DocumentPtr doc);
		
		/// Writes the document to the directory using the analyzer and the similarity score; returns the SegmentInfo describing the new segment
		SegmentInfoPtr writeDoc(DirectoryPtr dir, AnalyzerPtr analyzer, SimilarityPtr similarity, DocumentPtr doc);
		
		int32_t numFields(DocumentPtr doc);
	};
}
