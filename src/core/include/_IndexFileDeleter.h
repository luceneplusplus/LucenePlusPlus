/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _INDEXFILEDELETER_H
#define _INDEXFILEDELETER_H

#include "LuceneObject.h"

namespace Lucene
{
    /// Tracks the reference count for a single index file
    class RefCount : public LuceneObject
    {
    public:
        RefCount(const String& fileName);
        virtual ~RefCount();
        
        LUCENE_CLASS(RefCount);
            
    public:
        String fileName; // fileName used only for better assert error messages
        bool initDone;
        int32_t count;
    
    public:
        int32_t IncRef();
        int32_t DecRef();
    };    
}

#endif
