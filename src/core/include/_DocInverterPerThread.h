/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _DOCINVERTERPERTHREAD_H
#define _DOCINVERTERPERTHREAD_H

#include "AttributeSource.h"

namespace Lucene
{
    class SingleTokenAttributeSource : public AttributeSource
    {
    public:
        SingleTokenAttributeSource();
        virtual ~SingleTokenAttributeSource();
        
        LUCENE_CLASS(SingleTokenAttributeSource);
                
    public:
        CharTermAttributePtr termAttribute;
        OffsetAttributePtr offsetAttribute;
    
    public:
        void reinit(const String& stringValue, int32_t startOffset, int32_t endOffset);
    };
}

#endif
