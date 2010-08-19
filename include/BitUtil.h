/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneObject.h"

namespace Lucene
{
	/// A variety of high efficiency bit twiddling routines.
	class LPPAPI BitUtil : public LuceneObject
	{
	public:
		virtual ~BitUtil();		
		LUCENE_CLASS(BitUtil);
	
	public:
		/// Table of number of trailing zeros in a byte
		static const uint8_t ntzTable[]; 
			
	public:
		/// Returns the number of bits set in the long
		static uint32_t pop(uint64_t x);
		
		/// Returns the number of set bits in an array of longs.
		static uint64_t pop_array(const uint64_t* A, int32_t wordOffset, int32_t numWords);
		
		/// Returns the popcount or cardinality of the two sets after an intersection.  Neither array is modified.
		static uint64_t pop_intersect(const uint64_t* A, const uint64_t* B, int32_t wordOffset, int32_t numWords);
		
		/// Returns the popcount or cardinality of the union of two sets.  Neither array is modified.
		static uint64_t pop_union(const uint64_t* A, const uint64_t* B, int32_t wordOffset, int32_t numWords);
		
		/// Returns the popcount or cardinality of A & ~B.  Neither array is modified.
		static uint64_t pop_andnot(const uint64_t* A, const uint64_t* B, int32_t wordOffset, int32_t numWords);
		
		/// Returns the popcount or cardinality of A ^ B.  Neither array is modified.
		static uint64_t pop_xor(const uint64_t* A, const uint64_t* B, int32_t wordOffset, int32_t numWords);
		
		/// Returns number of trailing zeros in a 64 bit long value.
		static uint32_t ntz(uint64_t val);
		
		/// Returns number of trailing zeros in a 32 bit int value.
		static uint32_t ntz(uint32_t val);
		
		/// Returns 0 based index of first set bit (only works for x!=0)
		/// This is an alternate implementation of ntz()
		static uint32_t ntz2(uint64_t x);
		
		/// Returns 0 based index of first set bit.
		/// This is an alternate implementation of ntz()
		static uint32_t ntz3(uint64_t x);
		
		/// Returns true if v is a power of two or zero.
		static bool isPowerOfTwo(uint32_t v);
		
		/// Returns true if v is a power of two or zero.
		static bool isPowerOfTwo(uint64_t v);
		
		/// Returns the next highest power of two, or the current value if it's already a power of two or zero.
		static uint32_t nextHighestPowerOfTwo(uint32_t v);
		
		/// Returns the next highest power of two, or the current value if it's already a power of two or zero.
		static uint64_t nextHighestPowerOfTwo(uint64_t v);
	
	protected:
		inline static void CSA(uint64_t& h, uint64_t& l, uint64_t a, uint64_t b, uint64_t c); 
	};
}
