/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _FILTERINDEXREADER_H
#define _FILTERINDEXREADER_H

#include "TermPositions.h"
#include "TermEnum.h"

namespace Lucene
{
    /// Base class for filtering {@link TermDocs} implementations.
    class FilterTermDocs : public TermPositions, public LuceneObject
    {
    public:
        FilterTermDocs(TermDocsPtr in);
        virtual ~FilterTermDocs();
        
        LUCENE_CLASS(FilterTermDocs);
    
    protected:
        TermDocsPtr in;
    
    public:
        virtual void seek(TermPtr term);
        virtual void seek(TermEnumPtr termEnum);
        virtual int32_t doc();
        virtual int32_t freq();
        virtual bool next();
        virtual int32_t read(Collection<int32_t> docs, Collection<int32_t> freqs);
        virtual bool skipTo(int32_t target);
        virtual void close();
    };

    /// Base class for filtering {@link TermPositions} implementations.
    class FilterTermPositions : public FilterTermDocs
    {
    public:
        FilterTermPositions(TermPositionsPtr in);
        virtual ~FilterTermPositions();
        
        LUCENE_CLASS(FilterTermPositions);
    
    public:
        virtual int32_t nextPosition();
        virtual int32_t getPayloadLength();
        virtual ByteArray getPayload(ByteArray data, int32_t offset);
        virtual bool isPayloadAvailable();
    };
    
    /// Base class for filtering {@link TermEnum} implementations.
    class FilterTermEnum : public TermEnum
    {
    public:
        FilterTermEnum(TermEnumPtr in);
        virtual ~FilterTermEnum();
        
        LUCENE_CLASS(FilterTermEnum);
    
    protected:
        TermEnumPtr in;
    
    public:
        virtual bool next();
        virtual TermPtr term();
        virtual int32_t docFreq();
        virtual void close();
    };
}

#endif
