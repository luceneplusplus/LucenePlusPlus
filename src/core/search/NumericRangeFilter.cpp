/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "NumericRangeFilter.h"
#include "NumericRangeQuery.h"

namespace Lucene
{
    NumericRangeFilter::NumericRangeFilter(NumericRangeQueryPtr query) : MultiTermQueryWrapperFilter(query)
    {
    }
    
    NumericRangeFilter::~NumericRangeFilter()
    {
    }
    
    NumericRangeFilterPtr NumericRangeFilter::newLongRange(const String& field, int32_t precisionStep, int64_t min, int64_t max, bool minInclusive, bool maxInclusive)
    {
        return newNumericRange(field, precisionStep, min, max, minInclusive, maxInclusive);
    }
    
    NumericRangeFilterPtr NumericRangeFilter::newLongRange(const String& field, int64_t min, int64_t max, bool minInclusive, bool maxInclusive)
    {
        return newNumericRange(field, min, max, minInclusive, maxInclusive);
    }
    
    NumericRangeFilterPtr NumericRangeFilter::newIntRange(const String& field, int32_t precisionStep, int32_t min, int32_t max, bool minInclusive, bool maxInclusive)
    {
        return newNumericRange(field, precisionStep, min, max, minInclusive, maxInclusive);
    }
    
    NumericRangeFilterPtr NumericRangeFilter::newIntRange(const String& field, int32_t min, int32_t max, bool minInclusive, bool maxInclusive)
    {
        return newNumericRange(field, min, max, minInclusive, maxInclusive);
    }
    
    NumericRangeFilterPtr NumericRangeFilter::newDoubleRange(const String& field, int32_t precisionStep, double min, double max, bool minInclusive, bool maxInclusive)
    {
        return newNumericRange(field, precisionStep, min, max, minInclusive, maxInclusive);
    }
    
    NumericRangeFilterPtr NumericRangeFilter::newDoubleRange(const String& field, double min, double max, bool minInclusive, bool maxInclusive)
    {
        return newNumericRange(field, min, max, minInclusive, maxInclusive);
    }
    
    NumericRangeFilterPtr NumericRangeFilter::newNumericRange(const String& field, int32_t precisionStep, NumericValue min, NumericValue max, bool minInclusive, bool maxInclusive)
    {
        return newLucene<NumericRangeFilter>(NumericRangeQuery::newNumericRange(field, precisionStep, min, max, minInclusive, maxInclusive));
    }
    
    NumericRangeFilterPtr NumericRangeFilter::newNumericRange(const String& field, NumericValue min, NumericValue max, bool minInclusive, bool maxInclusive)
    {
        return newLucene<NumericRangeFilter>(NumericRangeQuery::newNumericRange(field, min, max, minInclusive, maxInclusive));
    }
    
    String NumericRangeFilter::getField()
    {
        return LuceneStaticCast<NumericRangeQuery>(query)->field;
    }
    
    bool NumericRangeFilter::includesMin()
    {
        return LuceneStaticCast<NumericRangeQuery>(query)->minInclusive;
    }
    
    bool NumericRangeFilter::includesMax()
    {
        return LuceneStaticCast<NumericRangeQuery>(query)->maxInclusive;
    }
    
    NumericValue NumericRangeFilter::getMin()
    {
        return LuceneStaticCast<NumericRangeQuery>(query)->min;
    }
    
    NumericValue NumericRangeFilter::getMax()
    {
        return LuceneStaticCast<NumericRangeQuery>(query)->min;
    }
}
