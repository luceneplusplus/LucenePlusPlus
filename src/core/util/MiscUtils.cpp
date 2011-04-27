/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "MiscUtils.h"
#include "StringUtils.h"
#include "LuceneObject.h"

namespace Lucene
{
    const uint32_t MiscUtils::SINGLE_EXPONENT_MASK = 0x7f800000;
    const uint32_t MiscUtils::SINGLE_MANTISSA_MASK = 0x007fffff;
    const uint32_t MiscUtils::SINGLE_NAN_BITS = (MiscUtils::SINGLE_EXPONENT_MASK | 0x00400000);

    const uint64_t MiscUtils::DOUBLE_SIGN_MASK = 0x8000000000000000LL;
    const uint64_t MiscUtils::DOUBLE_EXPONENT_MASK = 0x7ff0000000000000LL;
    const uint64_t MiscUtils::DOUBLE_MANTISSA_MASK = 0x000fffffffffffffLL;
    const uint64_t MiscUtils::DOUBLE_NAN_BITS = DOUBLE_EXPONENT_MASK | 0x0008000000000000LL;

    uint64_t MiscUtils::getTimeMillis(boost::posix_time::ptime time)
    {
        return boost::posix_time::time_duration(time - boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1))).total_milliseconds();
    }
    
    uint64_t MiscUtils::currentTimeMillis()
    {
        return getTimeMillis(boost::posix_time::microsec_clock::universal_time());
    }
    
    int32_t MiscUtils::oversize(int32_t minTargetSize, int32_t bytesPerElement)
    {
        // catch usage that accidentally overflows int
        if (minTargetSize < 0)
            boost::throw_exception(IllegalArgumentException(L"invalid array size " + StringUtils::toString(minTargetSize)));

        // wait until at least one element is requested
        if (minTargetSize == 0)
            return 0;
    
        // asymptotic exponential growth by 1/8th, favors spending a bit more CPU to not tie 
        // up too much wasted RAM
        int32_t extra = minTargetSize >> 3;

        // for very small arrays, where constant overhead of realloc is presumably relatively high, we grow faster
        if (extra < 3)
            extra = 3;

        int32_t newSize = minTargetSize + extra;

        // add 7 to allow for worst case byte alignment addition below
        if (newSize+7 < 0)
        {
            // int overflowed - return max allowed array size
            return INT_MAX;
        }

        #ifdef LPP_BUILD_64
        // round up to 8 byte alignment in 64bit env
        switch (bytesPerElement)
        {
            case 4:
                // round up to multiple of 2
                return (newSize + 1) & 0x7ffffffe;
            case 2:
                // round up to multiple of 4
                return (newSize + 3) & 0x7ffffffc;
            case 1:
                // round up to multiple of 8
                return (newSize + 7) & 0x7ffffff8;
            case 8:
                // no rounding
            default:
                // odd (invalid?) size
                return newSize;
        }
        #else
        // round up to 4 byte alignment in 32bit env
        switch (bytesPerElement)
        {
            case 2:
                // round up to multiple of 2
                return (newSize + 1) & 0x7ffffffe;
            case 1:
                // round up to multiple of 4
                return (newSize + 3) & 0x7ffffffc;
            case 4:
            case 8:
                // no rounding
            default:
                // odd (invalid?) size
                return newSize;
        }
        #endif
    }
    
    int32_t MiscUtils::getShrinkSize(int32_t currentSize, int32_t targetSize, int32_t bytesPerElement)
    {
        int32_t newSize = oversize(targetSize, bytesPerElement);
        return (newSize < currentSize / 2) ? newSize : currentSize;
    }
    
    int32_t MiscUtils::bytesDifference(uint8_t* bytes1, int32_t len1, uint8_t* bytes2, int32_t len2)
    {
        int32_t len = std::min(len1, len2);
        for (int32_t i = 0; i < len; ++i)
        {
            if (bytes1[i] != bytes2[i])
                return i;
        }
        return len;
    }
    
    int32_t MiscUtils::hashCode(const wchar_t* array, int32_t start, int32_t end)
    {
        return hashCode(array + start, array + end, hashNumeric<wchar_t>);
    }
    
    int32_t MiscUtils::hashCode(const uint8_t* array, int32_t start, int32_t end)
    {
        return hashCode(array + start, array + end, hashNumeric<uint8_t>);
    }
    
    int32_t MiscUtils::hashCode(bool value)
    {
        return value ? 1231 : 1237;
    }
    
    int32_t MiscUtils::doubleToIntBits(double value)
    {
        int32_t intValue = 0;
        float floatValue = (float)value;
        std::memcpy(&intValue, &floatValue, sizeof(float));
        
        if ((intValue & SINGLE_EXPONENT_MASK) == SINGLE_EXPONENT_MASK)
        {
            if (intValue & SINGLE_MANTISSA_MASK)
                return SINGLE_NAN_BITS;
        }
        
        return intValue;
    }
    
    int32_t MiscUtils::doubleToRawIntBits(double value)
    {
        int32_t intValue = 0;
        float floatValue = (float)value;
        std::memcpy(&intValue, &floatValue, sizeof(float));
        return intValue;
    }
    
    double MiscUtils::intBitsToDouble(int32_t bits)
    {
        float floatValue = 0;
        std::memcpy(&floatValue, &bits, sizeof(int32_t));
        return (double)floatValue;
    }
    
    int64_t MiscUtils::doubleToLongBits(double value)
    {
        int64_t longValue = 0;
        std::memcpy(&longValue, &value, sizeof(double));
        
        if ((longValue & DOUBLE_EXPONENT_MASK) == DOUBLE_EXPONENT_MASK)
        {
            if (longValue & DOUBLE_MANTISSA_MASK)
                return DOUBLE_NAN_BITS;
        }
        
        return longValue;
    }
    
    int64_t MiscUtils::doubleToRawLongBits(double value)
    {
        int64_t longValue = 0;
        std::memcpy(&longValue, &value, sizeof(double));
        return longValue;
    }
    
    double MiscUtils::longBitsToDouble(int64_t bits)
    {
        double doubleValue = 0;
        std::memcpy(&doubleValue, &bits, sizeof(int64_t));
        return doubleValue;
    }

    bool MiscUtils::isInfinite(double value)
    {
        return (value == std::numeric_limits<double>::infinity() || value == -std::numeric_limits<double>::infinity());
    }
    
    bool MiscUtils::isNaN(double value)
    {
        return (value != value);
    }
    
    bool MiscUtils::equalTypes(LuceneObjectPtr first, LuceneObjectPtr second)
    {
        return (typeid(*first) == typeid(*second));
    }
    
    int64_t MiscUtils::unsignedShift(int64_t num, int64_t shift)
    {
        return (shift & 0x3f) == 0 ? num : (((uint64_t)num >> 1) & 0x7fffffffffffffffLL) >> ((shift & 0x3f) - 1);
    }
    
    int32_t MiscUtils::unsignedShift(int32_t num, int32_t shift)
    {
        return (shift & 0x1f) == 0 ? num : (((uint32_t)num >> 1) & 0x7fffffff) >> ((shift & 0x1f) - 1);
    }
}
