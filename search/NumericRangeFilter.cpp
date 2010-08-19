#include "stdafx.h"
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
        return boost::static_pointer_cast<NumericRangeQuery>(query)->field;
    }
    
    bool NumericRangeFilter::includesMin()
    {
        return boost::static_pointer_cast<NumericRangeQuery>(query)->minInclusive;
    }
    
    bool NumericRangeFilter::includesMax()
    {
        return boost::static_pointer_cast<NumericRangeQuery>(query)->maxInclusive;
    }
    
    NumericValue NumericRangeFilter::getMin()
    {
        return boost::static_pointer_cast<NumericRangeQuery>(query)->min;
    }
    
    NumericValue NumericRangeFilter::getMax()
    {
        return boost::static_pointer_cast<NumericRangeQuery>(query)->min;
    }
}
