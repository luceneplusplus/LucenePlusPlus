/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "BitUtil.h"

namespace Lucene
{
    const uint8_t BitUtil::ntzTable[] =
    {
        8, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
    };
    
    BitUtil::~BitUtil()
    {
    }
    
    uint32_t BitUtil::pop(uint64_t x)
    {
        x = x - ((x >> 1) & 0x5555555555555555LL);
        x = (x & 0x3333333333333333LL) + ((x >> 2) & 0x3333333333333333LL);
        x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0fLL;
        x = x + (x >> 8);
        x = x + (x >> 16);
        x = x + (x >> 32);
        return (uint32_t)x & 0x7f;
    }
    
    uint64_t BitUtil::pop_array(const uint64_t* A, int32_t wordOffset, int32_t numWords)
    {
        int32_t n = wordOffset + numWords;
        uint64_t tot = 0;
        uint64_t tot8 = 0;
        uint64_t ones = 0;
        uint64_t twos = 0;
        uint64_t fours = 0;
        
        int32_t i = wordOffset;
        for (; i <= n - 8; i += 8)
        {
            uint64_t twosA;
            CSA(twosA, ones, ones, A[i], A[i + 1]);
            
            uint64_t twosB;
            CSA(twosB, ones, ones, A[i + 2], A[i + 3]);
            
            uint64_t foursA;
            CSA(foursA, twos, twos, twosA, twosB);
            
            CSA(twosA, ones, ones, A[i + 4], A[i + 5]);
            
            CSA(twosB, ones, ones, A[i + 6], A[i + 7]);
            
            uint64_t foursB;
            CSA(foursB, twos, twos, twosA, twosB);
            
            uint64_t eights;
            CSA(eights, fours, fours, foursA, foursB);
            
            tot8 += pop(eights);
        }
        
        // Handle trailing words in a binary-search manner.
        // Derived from the loop above by setting specific elements to 0.
        
        if (i <= n - 4)
        {
            uint64_t twosA;
            CSA(twosA, ones, ones, A[i], A[i + 1]);
            
            uint64_t twosB;
            CSA(twosB, ones, ones, A[i + 2], A[i + 3]);
            
            uint64_t foursA;
            CSA(foursA, twos, twos, twosA, twosB);
            
            uint64_t eights = fours & foursA;
            fours = fours ^ foursA;
            
            tot8 += pop(eights);
            i += 4;
        }
        
        if (i <= n - 2)
        {
            uint64_t twosA;
            CSA(twosA, ones, ones, A[i], A[i + 1]);
            
            uint64_t foursA = twos & twosA;
            twos = twos ^ twosA;
            
            uint64_t eights = fours & foursA;
            fours = fours ^ foursA;
            
            tot8 += pop(eights);
            i += 2;
        }
        
        if (i < n)
            tot += pop(A[i]);
        
        tot += (pop(fours) << 2) + (pop(twos) << 1) + pop(ones) + (tot8 << 3);
        
        return tot;
    }
    
    uint64_t BitUtil::pop_intersect(const uint64_t* A, const uint64_t* B, int32_t wordOffset, int32_t numWords)
    {
        int32_t n = wordOffset + numWords;
        uint64_t tot = 0;
        uint64_t tot8 = 0;
        uint64_t ones = 0;
        uint64_t twos = 0;
        uint64_t fours = 0;
        
        int32_t i = wordOffset;
        for (; i <= n - 8; i += 8)
        {
            uint64_t twosA;
            CSA(twosA, ones, ones, (A[i] & B[i]), (A[i + 1] & B[i + 1]));
            
            uint64_t twosB;
            CSA(twosB, ones, ones, (A[i + 2] & B[i + 2]), (A[i + 3] & B[i + 3]));
            
            uint64_t foursA;
            CSA(foursA, twos, twos, twosA, twosB);
            
            CSA(twosA, ones, ones, (A[i + 4] & B[i + 4]), (A[i + 5] & B[i + 5]));
            
            CSA(twosB, ones, ones, (A[i + 6] & B[i + 6]), (A[i + 7] & B[i + 7]));
            
            uint64_t foursB;
            CSA(foursB, twos, twos, twosA, twosB);
            
            uint64_t eights;
            CSA(eights, fours, fours, foursA, foursB);
            
            tot8 += pop(eights);
        }
        
        if (i <= n - 4)
        {
            uint64_t twosA;
            CSA(twosA, ones, ones, (A[i] & B[i]), (A[i + 1] & B[i + 1]));
            
            uint64_t twosB;
            CSA(twosB, ones, ones, (A[i + 2] & B[i + 2]), (A[i + 3] & B[i + 3]));
            
            uint64_t foursA;
            CSA(foursA, twos, twos, twosA, twosB);
            
            uint64_t eights = fours & foursA;
            fours = fours ^ foursA;
            
            tot8 += pop(eights);
            i += 4;
        }
        
        if (i <= n - 2)
        {
            uint64_t twosA;
            CSA(twosA, ones, ones, (A[i] & B[i]), (A[i + 1] & B[i + 1]));
            
            uint64_t foursA = twos & twosA;
            twos = twos ^ twosA;
            
            uint64_t eights = fours & foursA;
            fours = fours ^ foursA;
            
            tot8 += pop(eights);
            i += 2;
        }
        
        if (i < n)
            tot += pop((A[i] & B[i]));
        
        tot += (pop(fours) << 2) + (pop(twos) << 1) + pop(ones) + (tot8 << 3);
        
        return tot;
    }
    
    uint64_t BitUtil::pop_union(const uint64_t* A, const uint64_t* B, int32_t wordOffset, int32_t numWords)
    {
        int32_t n = wordOffset + numWords;
        uint64_t tot = 0;
        uint64_t tot8 = 0;
        uint64_t ones = 0;
        uint64_t twos = 0;
        uint64_t fours = 0;
        
        int32_t i = wordOffset;
        for (; i <= n - 8; i += 8)
        {
            uint64_t twosA;
            CSA(twosA, ones, ones, (A[i] | B[i]), (A[i + 1] | B[i + 1]));
            
            uint64_t twosB;
            CSA(twosB, ones, ones, (A[i + 2] | B[i + 2]), (A[i + 3] | B[i + 3]));
            
            uint64_t foursA;
            CSA(foursA, twos, twos, twosA, twosB);
            
            CSA(twosA, ones, ones, (A[i + 4] | B[i + 4]), (A[i + 5] | B[i + 5]));
            
            CSA(twosB, ones, ones, (A[i + 6] | B[i + 6]), (A[i + 7] | B[i + 7]));
            
            uint64_t foursB;
            CSA(foursB, twos, twos, twosA, twosB);
            
            uint64_t eights;
            CSA(eights, fours, fours, foursA, foursB);
            
            tot8 += pop(eights);
        }
        
        if (i <= n - 4)
        {
            uint64_t twosA;
            CSA(twosA, ones, ones, (A[i] | B[i]), (A[i + 1] | B[i + 1]));
         
            uint64_t twosB;
            CSA(twosB, ones, ones, (A[i + 2] | B[i + 2]), (A[i + 3] | B[i + 3]));
         
            uint64_t foursA;
            CSA(foursA, twos, twos, twosA, twosB);
            
            uint64_t eights = fours & foursA;
            fours = fours ^ foursA;
            
            tot8 += pop(eights);
            i += 4;
        }
        
        if (i <= n - 2)
        {
            uint64_t twosA;
            CSA(twosA, ones, ones, (A[i] | B[i]), (A[i + 1] | B[i + 1]));
            
            uint64_t foursA = twos & twosA;
            twos = twos ^ twosA;
            
            uint64_t eights = fours & foursA;
            fours = fours ^ foursA;
            
            tot8 += pop(eights);
            i += 2;
        }
        
        if (i < n)
            tot += pop((A[i] | B[i]));
        
        tot += (pop(fours) << 2) + (pop(twos) << 1) + pop(ones) + (tot8 << 3);
        
        return tot;
    }
    
    uint64_t BitUtil::pop_andnot(const uint64_t* A, const uint64_t* B, int32_t wordOffset, int32_t numWords)
    {
        int32_t n = wordOffset + numWords;
        uint64_t tot = 0;
        uint64_t tot8 = 0;
        uint64_t ones = 0;
        uint64_t twos = 0;
        uint64_t fours = 0;
        
        int32_t i = wordOffset;
        for (; i <= n - 8; i += 8)
        {
            uint64_t twosA;
            CSA(twosA, ones, ones, (A[i] & ~B[i]), (A[i + 1] & ~B[i + 1]));
            
            uint64_t twosB;
            CSA(twosB, ones, ones, (A[i + 2] & ~B[i + 2]), (A[i + 3] & ~B[i + 3]));
            
            uint64_t foursA;
            CSA(foursA, twos, twos, twosA, twosB);
            
            CSA(twosA, ones, ones, (A[i + 4] & ~B[i + 4]), (A[i + 5] & ~B[i + 5]));
            
            CSA(twosB, ones, ones, (A[i + 6] & ~B[i + 6]), (A[i + 7] & ~B[i + 7]));
            
            uint64_t foursB;
            CSA(foursB, twos, twos, twosA, twosB);
            
            uint64_t eights;
            CSA(eights, fours, fours, foursA, foursB);
            
            tot8 += pop(eights);
        }
        
        if (i <= n - 4)
        {
            uint64_t twosA;
            CSA(twosA, ones, ones, (A[i] & ~B[i]), (A[i + 1] & ~B[i + 1]));
            
            uint64_t twosB;
            CSA(twosB, ones, ones, (A[i + 2] & ~B[i + 2]), (A[i + 3] & ~B[i + 3]));
            
            uint64_t foursA;
            CSA(foursA, twos, twos, twosA, twosB);
            
            uint64_t eights = fours & foursA;
            fours = fours ^ foursA;
            
            tot8 += pop(eights);
            i += 4;
        }
        
        if (i <= n - 2)
        {
            uint64_t twosA;
            CSA(twosA, ones, ones, (A[i] & ~B[i]), (A[i + 1] & ~B[i + 1]));
            
            uint64_t foursA = twos & twosA;
            twos = twos ^ twosA;
            
            uint64_t eights = fours & foursA;
            fours = fours ^ foursA;
            
            tot8 += pop(eights);
            i += 2;
        }
        
        if (i < n)
            tot += pop((A[i] & ~B[i]));
        
        tot += (pop(fours) << 2) + (pop(twos) << 1) + pop(ones) + (tot8 << 3);
        
        return tot;
    }
    
    uint64_t BitUtil::pop_xor(const uint64_t* A, const uint64_t* B, int32_t wordOffset, int32_t numWords)
    {
        int32_t n = wordOffset + numWords;
        uint64_t tot = 0;
        uint64_t tot8 = 0;
        uint64_t ones = 0;
        uint64_t twos = 0;
        uint64_t fours = 0;
        
        int32_t i = wordOffset;
        for (; i <= n - 8; i += 8)
        {
            uint64_t twosA;
            CSA(twosA, ones, ones, (A[i] ^ B[i]), (A[i + 1] ^ B[i + 1]));
            
            uint64_t twosB;
            CSA(twosB, ones, ones, (A[i + 2] ^ B[i + 2]), (A[i + 3] ^ B[i + 3]));
            
            uint64_t foursA;
            CSA(foursA, twos, twos, twosA, twosB);
            
            CSA(twosA, ones, ones, (A[i + 4] ^ B[i + 4]), (A[i + 5] ^ B[i + 5]));
            
            CSA(twosB, ones, ones, (A[i + 6] ^ B[i + 6]), (A[i + 7] ^ B[i + 7]));
            
            uint64_t foursB;
            CSA(foursB, twos, twos, twosA, twosB);
            
            uint64_t eights;
            CSA(eights, fours, fours, foursA, foursB);
            
            tot8 += pop(eights);
        }
        
        if (i <= n - 4)
        {
            uint64_t twosA;
            CSA(twosA, ones, ones, (A[i] ^ B[i]), (A[i + 1] ^ B[i + 1]));
            
            uint64_t twosB;
            CSA(twosB, ones, ones, (A[i + 2] ^ B[i + 2]), (A[i + 3] ^ B[i + 3]));
            
            uint64_t foursA;
            CSA(foursA, twos, twos, twosA, twosB);
            
            uint64_t eights = fours & foursA;
            fours = fours ^ foursA;
            
            tot8 += pop(eights);
            i += 4;
        }
        
        if (i <= n - 2)
        {
            uint64_t twosA;
            CSA(twosA, ones, ones, (A[i] ^ B[i]), (A[i + 1] ^ B[i + 1]));
            
            uint64_t foursA = twos & twosA;
            twos = twos ^ twosA;
            
            uint64_t eights = fours & foursA;
            fours = fours ^ foursA;
            
            tot8 += pop(eights);
            i += 2;
        }
        
        if (i < n)
            tot += pop((A[i] ^ B[i]));
        
        tot += (pop(fours) << 2) + (pop(twos) << 1) + pop(ones) + (tot8 << 3);
        
        return tot;
    }

    void BitUtil::CSA(uint64_t& h, uint64_t& l, uint64_t a, uint64_t b, uint64_t c)
    {
        uint64_t u = a ^ b;
        h = (a & b) | (u & c);
        l = u ^ c;
    }
    
    uint32_t BitUtil::ntz(uint64_t val)
    {
        // A full binary search to determine the low byte was slower than a linear search for nextSetBit().  
        // This is most likely because the implementation of nextSetBit() shifts bits to the right, increasing
        // the probability that the first non-zero byte is in the rhs.
        
        // This implementation does a single binary search at the top level only so that all other bit shifting 
        // can be done on ints instead of longs to remain friendly to 32 bit architectures.  In addition, the 
        // case of a non-zero first byte is checked for first because it is the most common in dense bit arrays.
        
        uint32_t lower = (uint32_t)val;
        uint32_t lowByte = lower & 0xff;
        if (lowByte != 0)
            return ntzTable[lowByte];
        
        if (lower != 0)
        {
            lowByte = (lower >> 8) & 0xff;
            if (lowByte != 0)
                return ntzTable[lowByte] + 8;
            lowByte = (lower >> 16) & 0xff;
            if (lowByte != 0)
                return ntzTable[lowByte] + 16;
            // no need to mask off low byte for the last byte in the 32 bit word
            // no need to check for zero on the last byte either.
            return ntzTable[lower >> 24] + 24;
        }
        else
        {
            // grab upper 32 bits
            uint32_t upper = (uint32_t)(val >> 32);
            lowByte = upper & 0xff;
            if (lowByte != 0)
                return ntzTable[lowByte] + 32;
            lowByte = (upper >> 8) & 0xff;
            if (lowByte != 0)
                return ntzTable[lowByte] + 40;
            lowByte = (upper >> 16) & 0xff;
            if (lowByte != 0)
                return ntzTable[lowByte] + 48;
            // no need to mask off low byte for the last byte in the 32 bit word
            // no need to check for zero on the last byte either.
            return ntzTable[upper >> 24] + 56;
        }
    }
    
    uint32_t BitUtil::ntz(uint32_t val)
    {
        // This implementation does a single binary search at the top level only.  In addition, the case 
        // of a non-zero first byte is checked for first because it is the most common in dense bit arrays.
        
        uint32_t lowByte = val & 0xff;
        if (lowByte != 0)
            return ntzTable[lowByte];
        lowByte = (val >> 8) & 0xff;
        if (lowByte != 0)
            return ntzTable[lowByte] + 8;
        lowByte = (val >> 16) & 0xff;
        if (lowByte != 0)
            return ntzTable[lowByte] + 16;
        // no need to mask off low byte for the last byte.
        // no need to check for zero on the last byte either.
        return ntzTable[val >> 24] + 24;
    }
    
    uint32_t BitUtil::ntz2(uint64_t x)
    {
        uint32_t n = 0;
        uint32_t y = (uint32_t)x;
        if (y == 0) // the only 64 bit shift necessary
        {
            n += 32;
            y = (uint32_t)(x >> 32);
        }
        if ((y & 0x0000ffff) == 0)
        {
            n += 16;
            y >>= 16;
        }
        if ((y & 0x000000ff) == 0)
        {
            n += 8;
            y >>= 8;
        }
        return (ntzTable[y & 0xff]) + n;
    }
    
    uint32_t BitUtil::ntz3(uint64_t x)
    {
        uint32_t n = 1;
        
        // do the first step as a long, all others as ints.
        uint32_t y = (uint32_t)x;
        if (y == 0)
        {
            n += 32;
            y = (uint32_t)(x >> 32);
        }
        if ((y & 0x0000ffff) == 0)
        {
            n += 16;
            y >>= 16;
        }
        if ((y & 0x000000ff) == 0)
        {
            n += 8;
            y >>= 8;
        }
        if ((y & 0x0000000f) == 0)
        {
            n += 4;
            y >>= 4;
        }
        if ((y & 0x00000003) == 0)
        {
            n += 2;
            y >>= 2;
        }
        return n - (y & 1);
    }
    
    bool BitUtil::isPowerOfTwo(uint32_t v)
    {
        return ((v & (v - 1)) == 0);
    }
    
    bool BitUtil::isPowerOfTwo(uint64_t v)
    {
        return ((v & (v - 1)) == 0);
    }
    
    uint32_t BitUtil::nextHighestPowerOfTwo(uint32_t v)
    {
        --v;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        return ++v;
    }
    
    uint64_t BitUtil::nextHighestPowerOfTwo(uint64_t v)
    {
        --v;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v |= v >> 32;
        return ++v;
    }
}
