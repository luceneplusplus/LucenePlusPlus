/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _PARALLELREADER_H
#define _PARALLELREADER_H

#include "TermEnum.h"
#include "TermDocs.h"
#include "TermPositions.h"

namespace Lucene
{
    class ParallelTermEnum : public TermEnum
    {
    public:
        ParallelTermEnum(ParallelReaderPtr reader);
        ParallelTermEnum(ParallelReaderPtr reader, TermPtr term);
        virtual ~ParallelTermEnum();
        
        LUCENE_CLASS(ParallelTermEnum);
            
    protected:
        ParallelReaderWeakPtr _reader;
        String field;
        MapStringIndexReader::iterator fieldIterator;
        bool setIterator;
        TermEnumPtr termEnum;
    
    public:
        /// Increments the enumeration to the next element.  True if one exists.
        virtual bool next();
        
        /// Returns the current Term in the enumeration.
        virtual TermPtr term();
        
        /// Returns the docFreq of the current Term in the enumeration.
        virtual int32_t docFreq();
        
        /// Closes the enumeration to further activity, freeing resources.
        virtual void close();
    };
    
    /// Wrap a TermDocs in order to support seek(Term)
    class ParallelTermDocs : public TermPositions, public LuceneObject
    {
    public:
        ParallelTermDocs(ParallelReaderPtr reader);
        ParallelTermDocs(ParallelReaderPtr reader, TermPtr term);
        virtual ~ParallelTermDocs();
        
        LUCENE_CLASS(ParallelTermDocs);
            
    protected:
        ParallelReaderWeakPtr _reader;
        TermDocsPtr termDocs;
    
    public:
        virtual int32_t doc();
        virtual int32_t freq();
        virtual void seek(TermPtr term);
        virtual void seek(TermEnumPtr termEnum);
        virtual bool next();
        virtual int32_t read(Collection<int32_t> docs, Collection<int32_t> freqs);
        virtual bool skipTo(int32_t target);
        virtual void close();
    };
    
    class ParallelTermPositions : public ParallelTermDocs
    {
    public:
        ParallelTermPositions(ParallelReaderPtr reader);
        ParallelTermPositions(ParallelReaderPtr reader, TermPtr term);
        virtual ~ParallelTermPositions();
        
        LUCENE_CLASS(ParallelTermPositions);
            
    public:
        virtual void seek(TermPtr term);
        virtual int32_t nextPosition();
        virtual int32_t getPayloadLength();
        virtual ByteArray getPayload(ByteArray data, int32_t offset);
        virtual bool isPayloadAvailable();
    };
}

#endif
