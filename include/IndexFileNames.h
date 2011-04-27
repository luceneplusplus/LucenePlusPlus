/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef INDEXFILENAMES_H
#define INDEXFILENAMES_H

#include "LuceneObject.h"

namespace Lucene
{
    /// This class contains useful constants representing filenames and extensions used by lucene, as 
    /// well as convenience methods for querying whether a file name matches an extension ({@link 
    /// #matchesExtension(String, String) matchesExtension}), as well as generating file names from a 
    /// segment name, generation and extension ({@link 
    /// #fileNameFromGeneration(String, String, long) fileNameFromGeneration}, * {@link 
    /// #segmentFileName(String, String) segmentFileName}).
    class IndexFileNames : public LuceneObject
    {
    public:
        virtual ~IndexFileNames();        
        LUCENE_CLASS(IndexFileNames);
        
    public:
        /// Name of the index segment file.
        static const String& SEGMENTS();
        
        /// Name of the generation reference file name.
        static const String& SEGMENTS_GEN();
        
        /// Name of the index deletable file (only used in pre-lockless indices).
        static const String& DELETABLE();
        
        /// Extension of norms file.
        static const String& NORMS_EXTENSION();
        
        /// Extension of freq postings file.
        static const String& FREQ_EXTENSION();
        
        /// Extension of prox postings file.
        static const String& PROX_EXTENSION();
        
        /// Extension of terms file.
        static const String& TERMS_EXTENSION();
        
        /// Extension of terms index file.
        static const String& TERMS_INDEX_EXTENSION();
        
        /// Extension of stored fields index file.
        static const String& FIELDS_INDEX_EXTENSION();
        
        /// Extension of stored fields file.
        static const String& FIELDS_EXTENSION();
        
        /// Extension of vectors fields file.
        static const String& VECTORS_FIELDS_EXTENSION();
        
        /// Extension of vectors documents file.
        static const String& VECTORS_DOCUMENTS_EXTENSION();
        
        /// Extension of vectors index file.
        static const String& VECTORS_INDEX_EXTENSION();
        
        /// Extension of compound file.
        static const String& COMPOUND_FILE_EXTENSION();
        
        /// Extension of compound file for doc store files.
        static const String& COMPOUND_FILE_STORE_EXTENSION();
        
        /// Extension of deletes.
        static const String& DELETES_EXTENSION();
        
        /// Extension of field infos.
        static const String& FIELD_INFOS_EXTENSION();
        
        /// Extension of plain norms.
        static const String& PLAIN_NORMS_EXTENSION();
        
        /// Extension of separate norms.
        static const String& SEPARATE_NORMS_EXTENSION();
        
        /// Extension of gen file.
        static const String& GEN_EXTENSION();
        
        /// This array contains all filename extensions used by Lucene's index 
        /// files, with two exceptions, namely the extension made up from 
        /// ".f" + number and from ".s" + number.  Also note that Lucene's
        /// "segments_N" files do not have any filename extension.
        static const HashSet<String> INDEX_EXTENSIONS();
        
        /// File extensions that are added to a compound file (same as 
        /// {@link #INDEX_EXTENSIONS}, minus "del", "gen", "cfs").
        static const HashSet<String> INDEX_EXTENSIONS_IN_COMPOUND_FILE();
        
        static const HashSet<String> STORE_INDEX_EXTENSIONS();
        static const HashSet<String> NON_STORE_INDEX_EXTENSIONS();
        
        /// File extensions of old-style index files.
        static const HashSet<String> COMPOUND_EXTENSIONS();
        
        /// File extensions for term vector support.
        static const HashSet<String> VECTOR_EXTENSIONS();
        
        /// Computes the full file name from base, extension and generation. If the generation is -1, 
        /// the file name is null. If it's 0, the file name is <base>.<ext>. If it's > 0, the file 
        /// name is <base>_<gen>.<ext>.<br>
        ///
        /// NOTE: .<ext> is added to the name only if ext is not an empty string.
        /// @param base main part of the file name
        /// @param ext extension of the filename
        /// @param gen generation
        static String fileNameFromGeneration(const String& base, const String& ext, int64_t gen);
        
        /// Returns true if the provided filename is one of the doc store files (ends with an 
        /// extension in {@link #STORE_INDEX_EXTENSIONS}).
        static bool isDocStoreFile(const String& fileName);
        
        /// Returns the file name that matches the given segment name and extension. This method 
        /// takes care to return the full file name in the form <segmentName>.<ext>, therefore you 
        /// don't need to prefix the extension with a '.'.
        ///
        /// NOTE: .<ext> is added to the result file name only if ext is not empty.
        static String segmentFileName(const String& segmentName, const String& ext);
        
        /// Returns true if the given filename ends with the given extension. One should provide a 
        /// pure extension, without '.'.
        static bool matchesExtension(const String& filename, const String& ext);
        
        /// Strips the segment file name out of the given one. If you used {@link #segmentFileName} 
        /// or {@link #fileNameFromGeneration} to create your files, then this method simply removes 
        /// whatever comes before the first '.', or the second '_' (excluding both), in case of 
        /// deleted docs.
        /// @return the filename with the segment name removed, or the given filename if it does not 
        /// contain a '.' and '_'.
        static String stripSegmentName(const String& filename);
    };
}

#endif
