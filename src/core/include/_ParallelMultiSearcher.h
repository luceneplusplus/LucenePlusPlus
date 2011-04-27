/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _PARALLELMULTISEARCHER_H
#define _PARALLELMULTISEARCHER_H

#include "LuceneObject.h"

namespace Lucene
{
    /// A {@link Callable} to retrieve the document frequencies for a Term array
    class DocumentFrequencyCallable : public LuceneObject
    {
    public:
        DocumentFrequencyCallable(SearchablePtr searchable, Collection<TermPtr> terms);
        virtual ~DocumentFrequencyCallable();
    
        LUCENE_CLASS(DocumentFrequencyCallable);
    
    private:
        SearchablePtr searchable;
        Collection<TermPtr> terms;
    
    public:
        Collection<int32_t> call();
    };
}

#endif
