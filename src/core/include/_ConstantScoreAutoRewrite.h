/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _CONSTANTSCOREAUTOREWRITE_H
#define _CONSTANTSCOREAUTOREWRITE_H

#include "TermCollectingRewrite.h"

namespace Lucene
{
    class CutOffTermCollector : public TermCollector
    {
    public:
        CutOffTermCollector(IndexReaderPtr reader, int32_t docCountCutoff, int32_t termCountLimit);
        virtual ~CutOffTermCollector();
        
        LUCENE_CLASS(CutOffTermCollector);
    
    public:
        int32_t docVisitCount;
        bool hasCutOff;

        IndexReaderPtr reader;
        int32_t docCountCutoff;
        int32_t termCountLimit;
        Collection<TermPtr> pendingTerms;
    
    protected:
        virtual bool collect(TermPtr t, double boost);
    };
}

#endif
