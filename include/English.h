/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef ENGLISH_H
#define ENGLISH_H

#include "LuceneObject.h"

namespace Lucene
{
    class English : public LuceneObject
    {
    private:
        English();
    
    public:        
        virtual ~English();
        LUCENE_CLASS(English);
            
    public:
        static String longToEnglish(int64_t i);
        static String intToEnglish(int32_t i);
    };
}

#endif
