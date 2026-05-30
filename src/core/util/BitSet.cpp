/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "BitSet.h"
#include "BitUtil.h"
#include <boost/version.hpp>
#include <boost/iterator/function_output_iterator.hpp>

namespace Lucene {

BitSet::BitSet(uint32_t size) : bitSet(size) {
}

BitSet::~BitSet() {
}

BitSet::get_bits_result BitSet::getBits() {
#if BOOST_VERSION < 109000
    return bitSet.empty() ? NULL : static_cast<const uint64_t*>(&bitSet.m_bits[0]);
#else
    return bitSet;
#endif
}

void BitSet::clear() {
    bitSet.clear();
}

void BitSet::clear(uint32_t bitIndex) {
    if (bitIndex <= bitSet.size()) {
        bitSet.set(bitIndex, false);
    }
}

void BitSet::fastClear(uint32_t bitIndex) {
    bitSet.set(bitIndex, false);
}

void BitSet::clear(uint32_t fromIndex, uint32_t toIndex) {
#if BOOST_VERSION >= 106900
    fromIndex = std::min<bitset_type::size_type>(fromIndex, bitSet.size());
    toIndex = std::min<bitset_type::size_type>(toIndex, bitSet.size());
    bitSet.reset(fromIndex, toIndex - fromIndex);
#else
    toIndex = std::min(toIndex, (uint32_t)bitSet.size());
    for (bitset_type::size_type i = std::min(fromIndex, (uint32_t)bitSet.size()); i < toIndex; ++i) {
        bitSet.set(i, false);
    }
#endif
}

void BitSet::fastClear(uint32_t fromIndex, uint32_t toIndex) {
    fastSet(fromIndex, toIndex, false);
}

void BitSet::set(uint32_t bitIndex) {
    if (bitIndex >= bitSet.size()) {
        resize(bitIndex + 1);
    }
    bitSet.set(bitIndex, true);
}

void BitSet::fastSet(uint32_t bitIndex) {
    bitSet.set(bitIndex, true);
}

void BitSet::set(uint32_t bitIndex, bool value) {
    if (bitIndex >= bitSet.size()) {
        resize(bitIndex + 1);
    }
    bitSet.set(bitIndex, value);
}

void BitSet::fastSet(uint32_t bitIndex, bool value) {
    bitSet.set(bitIndex, value);
}

void BitSet::set(uint32_t fromIndex, uint32_t toIndex) {
    set(fromIndex, toIndex, true);
}

void BitSet::fastSet(uint32_t fromIndex, uint32_t toIndex) {
    fastSet(fromIndex, toIndex, true);
}

void BitSet::set(uint32_t fromIndex, uint32_t toIndex, bool value) {
    if (toIndex >= bitSet.size()) {
        resize(toIndex + 1);
    }
    fastSet(fromIndex, toIndex, value);
}

void BitSet::fastSet(uint32_t fromIndex, uint32_t toIndex, bool value) {
#if BOOST_VERSION >= 106900
    bitSet.set(fromIndex, toIndex - fromIndex, value);
#else
    for (bitset_type::size_type i = fromIndex; i < toIndex; ++i) {
        bitSet.set(i, value);
    }
#endif
}

void BitSet::flip(uint32_t bitIndex) {
    if (bitIndex >= bitSet.size()) {
        resize(bitIndex + 1);
    }
    bitSet.flip(bitIndex);
}

void BitSet::fastFlip(uint32_t bitIndex) {
    bitSet.flip(bitIndex);
}

void BitSet::flip(uint32_t fromIndex, uint32_t toIndex) {
    if (toIndex >= bitSet.size()) {
        resize(toIndex + 1);
    }
    fastFlip(fromIndex, toIndex);
}

void BitSet::fastFlip(uint32_t fromIndex, uint32_t toIndex) {
#if BOOST_VERSION >= 106900
    bitSet.flip(fromIndex, toIndex - fromIndex);
#else
    for (bitset_type::size_type i = fromIndex; i < toIndex; ++i) {
        bitSet.flip(i);
    }
#endif
}

uint32_t BitSet::size() const {
    return bitSet.num_blocks() * sizeof(bitset_type::block_type) * 8;
}

uint32_t BitSet::numBlocks() const {
    return bitSet.num_blocks();
}

bool BitSet::isEmpty() const {
    return bitSet.none();
}

bool BitSet::get(uint32_t bitIndex) const {
    return bitIndex < bitSet.size() ? bitSet.test(bitIndex) : false;
}

bool BitSet::fastGet(uint32_t bitIndex) const {
    return bitSet.test(bitIndex);
}

int32_t BitSet::nextSetBit(uint32_t fromIndex) const {
#if BOOST_VERSION >= 108800
    return bitSet.find_first(fromIndex);
#else
    bitset_type::size_type next = fromIndex == 0 ? bitSet.find_first() : bitSet.find_next(fromIndex - 1);
    return next == bitset_type::npos ? -1 : next;
#endif
}

void BitSet::_and(const BitSetPtr& set) {
    bitset_type other = set->bitSet;
    other.resize(bitSet.size());
    bitSet &= other;
}

void BitSet::_or(const BitSetPtr& set) {
    if (set->bitSet.size() > bitSet.size()) {
        resize(set->bitSet.size());
        bitSet |= set->bitSet;
    } else {
        bitset_type other = set->bitSet;
        other.resize(bitSet.size());
        bitSet |= other;
    }
}

void BitSet::_xor(const BitSetPtr& set) {
    if (set->bitSet.size() > bitSet.size()) {
        resize(set->bitSet.size());
        bitSet ^= set->bitSet;
    } else {
        bitset_type other = set->bitSet;
        other.resize(bitSet.size());
        bitSet ^= other;
    }
}

void BitSet::andNot(const BitSetPtr& set) {
    bitset_type other = set->bitSet;
    other.resize(bitSet.size());
    bitSet &= other.flip();
}

bool BitSet::intersectsBitSet(const BitSetPtr& set) const {
    return bitSet.intersects(set->bitSet);
}

uint32_t BitSet::cardinality() {
    return bitSet.count();
}

void BitSet::resize(uint32_t size) {
    bitSet.resize(size);
}

bool BitSet::equals(const LuceneObjectPtr& other) {
    if (LuceneObject::equals(other)) {
        return true;
    }
    BitSetPtr otherBitSet(boost::dynamic_pointer_cast<BitSet>(other));
    if (!otherBitSet) {
        return false;
    }
    BitSetPtr first = bitSet.num_blocks() < otherBitSet->bitSet.num_blocks() ? otherBitSet : shared_from_this();
    BitSetPtr second = bitSet.num_blocks() < otherBitSet->bitSet.num_blocks() ? shared_from_this() : otherBitSet;
    bitset_type::size_type f = first->bitSet.find_first();
    bitset_type::size_type s = second->bitSet.find_first();
    while (f == s) {
        if (f == bitset_type::npos) {
            return true;
        }
        f = first->bitSet.find_next(f);
        s = second->bitSet.find_next(s);
    }
    return false;
}

int32_t BitSet::hashCode() {
    // Start with a zero hash and use a mix that results in zero if the input is zero.
    // This effectively truncates trailing zeros without an explicit check.
    int64_t hash = 0;
    auto computeHash = [&hash](bitset_type::block_type block) {
        hash ^= block;
        hash = (hash << 1) | (hash >> 63); // rotate left
    };
    to_block_range(bitSet, boost::make_function_output_iterator(std::ref(computeHash)));
    // Fold leftmost bits into right and add a constant to prevent empty sets from
    // returning 0, which is too common.
    return (int32_t)((hash >> 32) ^ hash) + 0x98761234;
}

LuceneObjectPtr BitSet::clone(const LuceneObjectPtr& other) {
    LuceneObjectPtr clone = other ? other : newLucene<BitSet>();
    BitSetPtr cloneBitSet(boost::dynamic_pointer_cast<BitSet>(LuceneObject::clone(clone)));
    cloneBitSet->bitSet = bitSet;
    return cloneBitSet;
}

}
