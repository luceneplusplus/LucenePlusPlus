/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef READERUTIL_H
#define READERUTIL_H

#include "LuceneObject.h"

namespace Lucene
{
    /// Common util methods for dealing with {@link IndexReader}s.
    class ReaderUtil : public LuceneObject
    {
    private:
        ReaderUtil();
        
    public:
        virtual ~ReaderUtil();
        LUCENE_CLASS(ReaderUtil);
    
    public:
        /// Gathers sub-readers from reader into a List.
        static void gatherSubReaders(Collection<IndexReaderPtr> allSubReaders, IndexReaderPtr reader);
        
        /// Returns sub IndexReader that contains the given document id.
        ///
        /// @param doc Id of document
        /// @param reader Parent reader
        /// @return Sub reader of parent which contains the specified doc id
        static IndexReaderPtr subReader(int32_t doc, IndexReaderPtr reader);
        
        /// Returns sub-reader subIndex from reader.
        ///
        /// @param reader Parent reader
        /// @param subIndex Index of desired sub reader
        /// @return The subreader at subIndex
        static IndexReaderPtr subReader(IndexReaderPtr reader, int32_t subIndex);
        
        /// Returns index of the searcher/reader for document n in the array used to construct this 
        /// searcher/reader.
        static int32_t subIndex(int32_t n, Collection<int32_t> docStarts);
    };
    
    /// Recursively visits all sub-readers of a reader.  You should subclass this and override 
    /// the add method to gather what you need.
    class ReaderGather : public LuceneObject
    {
    public:
        ReaderGather(IndexReaderPtr r);
        virtual ~ReaderGather();
        
        LUCENE_CLASS(ReaderGather);
    
    private:
        IndexReaderPtr topReader;

    private:
        int32_t run(int32_t base, IndexReaderPtr reader);
    
    public:
        int32_t run(int32_t docBase = 0);

    protected:
        virtual void add(int32_t base, IndexReaderPtr r) = 0;
    };
}

#endif
