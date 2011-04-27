/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _MULTITERMQUERY_H
#define _MULTITERMQUERY_H

#include "LuceneObject.h"

namespace Lucene
{
    class ConstantScoreFilterRewrite : public RewriteMethod
    {
    public:
        virtual ~ConstantScoreFilterRewrite();
        LUCENE_CLASS(ConstantScoreFilterRewrite);
    
    public:
        virtual QueryPtr rewrite(IndexReaderPtr reader, MultiTermQueryPtr query);
    };
}

#endif
