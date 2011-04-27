/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SEGMENTDELETES_H
#define SEGMENTDELETES_H

#include "LuceneObject.h"

namespace Lucene
{
    /// Holds buffered deletes, by docID, term or query for a single segment. This is used to 
    /// hold buffered pending deletes against the to-be-flushed segment as well as per-segment 
    /// deletes for each segment in the index.
    ///
    /// NOTE: we are sync'd by BufferedDeletes, ie, all access to instances of this class is 
    /// via sync'd methods on BufferedDeletes
    class LPPAPI SegmentDeletes : public LuceneObject
    {
    public:
        SegmentDeletes();
        virtual ~SegmentDeletes();
        
        LUCENE_CLASS(SegmentDeletes);
    
    public:
        /// Rough logic: HashMap has an array[Entry] with varying load factor (say 2 * POINTER).
        /// Entry is object with Term key, Integer val, int hash, Entry next 
        /// (OBJ_HEADER + 3*POINTER + INT).  Term is object with String field and String text 
        /// (OBJ_HEADER + 2*POINTER).
        /// We don't count Term's field since it's interned. Term's text is 
        /// String (OBJ_HEADER + 4*INT + POINTER + OBJ_HEADER + string.length*CHAR).  
        /// Integer is OBJ_HEADER + INT.
        static const int32_t BYTES_PER_DEL_TERM;
        
        /// Rough logic: del docIDs are List<Integer>.  Say list allocates ~2X size (2*POINTER).
        /// Integer is OBJ_HEADER + int
        static const int32_t BYTES_PER_DEL_DOCID;
        
        /// Rough logic: HashMap has an array[Entry] with varying load factor (say 2 * POINTER).
        /// Entry is object with Query key, Integer val, int hash, 
        /// Entry next (OBJ_HEADER + 3*POINTER + INT).  Query we often undercount (say 24 bytes).
        /// Integer is OBJ_HEADER + INT.
        static const int32_t BYTES_PER_DEL_QUERY;
    
        AtomicLongPtr numTermDeletes;
        SortedMapTermInt terms;
        MapQueryInt queries;
        Collection<int32_t> docIDs;
        AtomicLongPtr bytesUsed;
    
    private:
        static const bool VERBOSE_DELETES;

    public:
        virtual String toString();
        
        virtual void update(SegmentDeletesPtr in, bool noLimit);
        virtual void addQuery(QueryPtr query, int32_t docIDUpto);
        virtual void addDocID(int32_t docID);
        virtual void addTerm(TermPtr term, int32_t docIDUpto);
        virtual void clear();
        virtual void clearDocIDs();
        virtual bool any();
    };
}

#endif
