/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneObject.h"

namespace Lucene
{
	/// Constants representing filenames and extensions used by Lucene.
	class LPPAPI IndexFileNames : public LuceneObject
	{
	public:
		virtual ~IndexFileNames();		
		LUCENE_CLASS(IndexFileNames);
		
	public:
		/// Name of the index segment file.
		static String& SEGMENTS();
		
		/// Name of the generation reference file name.
		static String& SEGMENTS_GEN();
		
		/// Name of the index deletable file (only used in pre-lockless indices).
		static String& DELETABLE();
		
		/// Extension of norms file.
		static String& NORMS_EXTENSION();
		
		/// Extension of freq postings file.
		static String& FREQ_EXTENSION();
		
		/// Extension of prox postings file.
		static String& PROX_EXTENSION();
		
		/// Extension of terms file.
		static String& TERMS_EXTENSION();
		
		/// Extension of terms index file.
		static String& TERMS_INDEX_EXTENSION();
		
		/// Extension of stored fields index file.
		static String& FIELDS_INDEX_EXTENSION();
		
		/// Extension of stored fields file.
		static String& FIELDS_EXTENSION();
		
		/// Extension of vectors fields file.
		static String& VECTORS_FIELDS_EXTENSION();
		
		/// Extension of vectors documents file.
		static String& VECTORS_DOCUMENTS_EXTENSION();
		
		/// Extension of vectors index file.
		static String& VECTORS_INDEX_EXTENSION();
		
		/// Extension of compound file.
		static String& COMPOUND_FILE_EXTENSION();
		
		/// Extension of compound file for doc store files.
		static String& COMPOUND_FILE_STORE_EXTENSION();
		
		/// Extension of deletes.
		static String& DELETES_EXTENSION();
		
		/// Extension of field infos.
		static String& FIELD_INFOS_EXTENSION();
		
		/// Extension of plain norms.
		static String& PLAIN_NORMS_EXTENSION();
		
		/// Extension of separate norms.
		static String& SEPARATE_NORMS_EXTENSION();
		
		/// Extension of gen file.
		static String& GEN_EXTENSION();
		
		/// This array contains all filename extensions used by Lucene's index 
		/// files, with two exceptions, namely the extension made up from 
		/// ".f" + number and from ".s" + number.  Also note that Lucene's
		/// "segments_N" files do not have any filename extension.
		static HashSet<String> INDEX_EXTENSIONS();
		
		/// File extensions that are added to a compound file (same as 
		/// {@link #INDEX_EXTENSIONS}, minus "del", "gen", "cfs").
		static HashSet<String> INDEX_EXTENSIONS_IN_COMPOUND_FILE();
		
		static HashSet<String> STORE_INDEX_EXTENSIONS();
		static HashSet<String> NON_STORE_INDEX_EXTENSIONS();
		
		/// File extensions of old-style index files.
		static HashSet<String> COMPOUND_EXTENSIONS();
		
		/// File extensions for term vector support.
		static HashSet<String> VECTOR_EXTENSIONS();
		
		/// Computes the full file name from base, extension and generation.  
		/// If the generation is {@link SegmentInfo#NO}, the file name is null.
		/// If it's {@link SegmentInfo#WITHOUT_GEN} the file name is base+extension.
		/// If it's > 0, the file name is base_generation+extension.
		static String fileNameFromGeneration(const String& base, const String& extension, int64_t gen);
		
		/// Returns true if the provided filename is one of the doc store files 
		/// (ends with an extension in STORE_INDEX_EXTENSIONS).
		static bool isDocStoreFile(const String& fileName);
		
		/// Return segment file name.
		static String segmentFileName(const String& segmentName, const String& ext);
	};
}
