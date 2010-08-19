/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "QueryWrapperFilter.h"
#include "Query.h"
#include "Weight.h"
#include "Scorer.h"
#include "IndexSearcher.h"

namespace Lucene
{
    QueryWrapperFilter::QueryWrapperFilter(QueryPtr query)
    {
        this->query = query;
    }
    
    QueryWrapperFilter::~QueryWrapperFilter()
    {
    }
    
    DocIdSetPtr QueryWrapperFilter::getDocIdSet(IndexReaderPtr reader)
    {
        WeightPtr weight(query->weight(newLucene<IndexSearcher>(reader)));
        return newLucene<QueryWrapperFilterDocIdSet>(reader, weight);
    }
    
    String QueryWrapperFilter::toString()
    {
        return L"QueryWrapperFilter(" + query->toString() + L")";
    }
    
    bool QueryWrapperFilter::equals(LuceneObjectPtr other)
    {
        QueryWrapperFilterPtr otherQueryWrapperFilter(boost::dynamic_pointer_cast<QueryWrapperFilter>(other));
        if (!otherQueryWrapperFilter)
            return false;
        return this->query->equals(otherQueryWrapperFilter->query);
    }
    
    int32_t QueryWrapperFilter::hashCode()
    {
        return query->hashCode() ^ 0x923F64B9;
    }
    
    QueryWrapperFilterDocIdSet::QueryWrapperFilterDocIdSet(IndexReaderPtr reader, WeightPtr weight)
    {
        this->reader = reader;
        this->weight = weight;
    }
    
    QueryWrapperFilterDocIdSet::~QueryWrapperFilterDocIdSet()
    {
    }
    
    DocIdSetIteratorPtr QueryWrapperFilterDocIdSet::iterator()
    {
        return weight->scorer(reader, true, false);
    }
    
    bool QueryWrapperFilterDocIdSet::isCacheable()
    {
        return false;
    }
}
