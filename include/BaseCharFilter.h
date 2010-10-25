/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef BASECHARFILTER_H
#define BASECHARFILTER_H

#include "CharFilter.h"

namespace Lucene
{
    /// Base utility class for implementing a {@link CharFilter}.  You subclass this, and then record mappings by 
    /// calling {@link #addOffCorrectMap}, and then invoke the correct method to correct an offset.
    ///
    /// NOTE: This class is not particularly efficient. For example, a new class instance is created for every call 
    /// to {@link #addOffCorrectMap}, which is then appended to a private list.
    class LPPAPI BaseCharFilter : public CharFilter
    {
    public:
        BaseCharFilter(CharStreamPtr in);
        virtual ~BaseCharFilter();
        
        LUCENE_CLASS(BaseCharFilter);
    
    protected:
        Collection<OffCorrectMapPtr> pcmList;
    
    protected:
        /// Retrieve the corrected offset.  Note that this method is slow, if you correct positions far before the 
        /// most recently added position, as it's a simple linear search backwards through all offset corrections 
        /// added by {@link #addOffCorrectMap}.
        virtual int32_t correct(int32_t currentOff);
        
        int32_t getLastCumulativeDiff();
        void addOffCorrectMap(int32_t off, int32_t cumulativeDiff);
    };
    
    class LPPAPI OffCorrectMap : public LuceneObject
    {
    public:
        OffCorrectMap(int32_t off, int32_t cumulativeDiff);
        virtual ~OffCorrectMap();
    
    public:
        int32_t off;
        int32_t cumulativeDiff;
    
    public:
        virtual String toString();
    };
}

#endif
