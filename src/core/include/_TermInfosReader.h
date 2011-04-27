/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _TERMINFOSREADER_H
#define _TERMINFOSREADER_H

#include "TermInfo.h"

namespace Lucene
{
    /// Per-thread resources managed by ThreadLocal
    class TermInfosReaderThreadResources : public LuceneObject
    {
    public:
        virtual ~TermInfosReaderThreadResources();
        LUCENE_CLASS(TermInfosReaderThreadResources);
            
    public:
        SegmentTermEnumPtr termEnum;
    };
    
    /// Just adds term's ord to TermInfo
    class TermInfoAndOrd : public TermInfo
    {
    public:
        TermInfoAndOrd(TermInfoPtr ti, int32_t termOrd);
        virtual ~TermInfoAndOrd();
        
        LUCENE_CLASS(TermInfoAndOrd);
            
    public:
        int32_t termOrd;
    };
}

#endif
