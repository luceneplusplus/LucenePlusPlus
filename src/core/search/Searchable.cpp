/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "Searchable.h"

namespace Lucene
{
    void Searchable::search(WeightPtr weight, FilterPtr filter, CollectorPtr collector)
    {
        BOOST_ASSERT(false);
        // override
    }
    
    void Searchable::close()
    {
        BOOST_ASSERT(false);
        // override
    }
    
    int32_t Searchable::docFreq(TermPtr term)
    {
        BOOST_ASSERT(false);
        return 0; // override
    }
    
    Collection<int32_t> Searchable::docFreqs(Collection<TermPtr> terms)
    {
        BOOST_ASSERT(false);
        return Collection<int32_t>(); // override
    }
    
    int32_t Searchable::maxDoc()
    {
        BOOST_ASSERT(false);
        return 0; // override
    }
    
    TopDocsPtr Searchable::search(WeightPtr weight, FilterPtr filter, int32_t n)
    {
        BOOST_ASSERT(false);
        return TopDocsPtr(); // override
    }
    
    DocumentPtr Searchable::doc(int32_t n)
    {
        BOOST_ASSERT(false);
        return DocumentPtr(); // override
    }
    
    DocumentPtr Searchable::doc(int32_t n, FieldSelectorPtr fieldSelector)
    {
        BOOST_ASSERT(false);
        return DocumentPtr(); // override
    }
    
    QueryPtr Searchable::rewrite(QueryPtr query)
    {
        BOOST_ASSERT(false);
        return QueryPtr(); // override
    }
    
    ExplanationPtr Searchable::explain(WeightPtr weight, int32_t doc)
    {
        BOOST_ASSERT(false);
        return ExplanationPtr(); // override
    }
    
    TopFieldDocsPtr Searchable::search(WeightPtr weight, FilterPtr filter, int32_t n, SortPtr sort)
    {
        BOOST_ASSERT(false);
        return TopFieldDocsPtr(); // override
    }
}
