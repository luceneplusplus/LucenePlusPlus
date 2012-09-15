/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "FieldCacheRangeFilter.h"
#include "_FieldCacheRangeFilter.h"
#include "FieldCache.h"
#include "IndexReader.h"
#include "TermDocs.h"
#include "NumericUtils.h"
#include "MiscUtils.h"
#include "StringUtils.h"

namespace Lucene
{
    FieldCacheRangeFilter::FieldCacheRangeFilter(const String& field, ParserPtr parser, bool includeLower, bool includeUpper)
    {
        this->field = field;
        this->parser = parser;
        this->includeLower = includeLower;
        this->includeUpper = includeUpper;
    }
    
    FieldCacheRangeFilter::~FieldCacheRangeFilter()
    {
    }
    
    FieldCacheRangeFilterPtr FieldCacheRangeFilter::newStringRange(const String& field, const String& lowerVal, const String& upperVal, bool includeLower, bool includeUpper)
    {
        return newLucene<FieldCacheRangeFilterString>(field, ParserPtr(), lowerVal, upperVal, includeLower, includeUpper);
    }
    
    FieldCacheRangeFilterPtr FieldCacheRangeFilter::newByteRange(const String& field, uint8_t lowerVal, uint8_t upperVal, bool includeLower, bool includeUpper)
    {
        return newByteRange(field, ByteParserPtr(), lowerVal, upperVal, includeLower, includeUpper);
    }
    
    FieldCacheRangeFilterPtr FieldCacheRangeFilter::newByteRange(const String& field, ByteParserPtr parser, uint8_t lowerVal, uint8_t upperVal, bool includeLower, bool includeUpper)
    {
        return newLucene<FieldCacheRangeFilterByte>(field, parser, lowerVal, upperVal, includeLower, includeUpper);
    }
    
    FieldCacheRangeFilterPtr FieldCacheRangeFilter::newIntRange(const String& field, int32_t lowerVal, int32_t upperVal, bool includeLower, bool includeUpper)
    {
        return newIntRange(field, IntParserPtr(), lowerVal, upperVal, includeLower, includeUpper);
    }
    
    FieldCacheRangeFilterPtr FieldCacheRangeFilter::newIntRange(const String& field, IntParserPtr parser, int32_t lowerVal, int32_t upperVal, bool includeLower, bool includeUpper)
    {
        return newLucene<FieldCacheRangeFilterInt>(field, parser, lowerVal, upperVal, includeLower, includeUpper);
    }
    
    FieldCacheRangeFilterPtr FieldCacheRangeFilter::newLongRange(const String& field, int64_t lowerVal, int64_t upperVal, bool includeLower, bool includeUpper)
    {
        return newLongRange(field, LongParserPtr(), lowerVal, upperVal, includeLower, includeUpper);
    }
    
    FieldCacheRangeFilterPtr FieldCacheRangeFilter::newLongRange(const String& field, LongParserPtr parser, int64_t lowerVal, int64_t upperVal, bool includeLower, bool includeUpper)
    {
        return newLucene<FieldCacheRangeFilterLong>(field, parser, lowerVal, upperVal, includeLower, includeUpper);
    }
    
    FieldCacheRangeFilterPtr FieldCacheRangeFilter::newDoubleRange(const String& field, double lowerVal, double upperVal, bool includeLower, bool includeUpper)
    {
        return newDoubleRange(field, DoubleParserPtr(), lowerVal, upperVal, includeLower, includeUpper);
    }
    
    FieldCacheRangeFilterPtr FieldCacheRangeFilter::newDoubleRange(const String& field, DoubleParserPtr parser, double lowerVal, double upperVal, bool includeLower, bool includeUpper)
    {
        return newLucene<FieldCacheRangeFilterDouble>(field, parser, lowerVal, upperVal, includeLower, includeUpper);
    }
    
    String FieldCacheRangeFilter::getField()
    {
        return field;
    }
    
    bool FieldCacheRangeFilter::includesLower()
    {
        return includeLower;
    }
    
    bool FieldCacheRangeFilter::includesUpper()
    {
        return includeUpper;
    }
    
    ParserPtr FieldCacheRangeFilter::getParser()
    {
        return parser;
    }
    
    FieldCacheRangeFilterString::FieldCacheRangeFilterString(const String& field, ParserPtr parser, const String& lowerVal, const String& upperVal, bool includeLower, bool includeUpper)
        : FieldCacheRangeFilter(field, parser, includeLower, includeUpper)
    {
        this->lowerVal = lowerVal;
        this->upperVal = upperVal;
    }
    
    FieldCacheRangeFilterString::~FieldCacheRangeFilterString()
    {
    }
    
    DocIdSetPtr FieldCacheRangeFilterString::getDocIdSet(IndexReaderPtr reader)
    {
        StringIndexPtr fcsi(FieldCache::DEFAULT()->getStringIndex(reader, field));
        int32_t lowerPoint = fcsi->binarySearchLookup(lowerVal);
        int32_t upperPoint = fcsi->binarySearchLookup(upperVal);
        
        int32_t inclusiveLowerPoint = 0;
        int32_t inclusiveUpperPoint = 0;
        
        // Hints:
        // * binarySearchLookup returns 0, if value was null.
        // * the value is <0 if no exact hit was found, the returned value is (-(insertion point) - 1)
        if (lowerPoint == 0)
        {
            BOOST_ASSERT(lowerVal.empty());
            inclusiveLowerPoint = 1;
        }
        else if (includeLower && lowerPoint > 0)
            inclusiveLowerPoint = lowerPoint;
        else if (lowerPoint > 0)
            inclusiveLowerPoint = lowerPoint + 1;
        else
            inclusiveLowerPoint = std::max((int32_t)1, -lowerPoint - 1);
        
        if (upperPoint == 0)
        {
            BOOST_ASSERT(upperVal.empty());
            inclusiveUpperPoint = INT_MAX;
        }
        else if (includeUpper && upperPoint > 0)
            inclusiveUpperPoint = upperPoint;
        else if (upperPoint > 0)
            inclusiveUpperPoint = upperPoint - 1;
        else
            inclusiveUpperPoint = -upperPoint - 2;
        
        if (inclusiveUpperPoint <= 0 || inclusiveLowerPoint > inclusiveUpperPoint)
            return DocIdSet::EMPTY_DOCIDSET();
        
        BOOST_ASSERT(inclusiveLowerPoint > 0 && inclusiveUpperPoint > 0);
        
        // for this DocIdSet, we never need to use TermDocs, because deleted docs have an order of 0 
        // (null entry in StringIndex)
        return newLucene<FieldCacheDocIdSetString>(reader, false, fcsi, inclusiveLowerPoint, inclusiveUpperPoint);
    }
    
    String FieldCacheRangeFilterString::toString()
    {
        StringStream buffer;
        buffer << field << L":" << (includeLower ? L"[" : L"{");
        buffer << lowerVal << L" TO " << lowerVal;
        buffer << (includeLower ? L"]" : L"}");
        return buffer.str();
    }
    
    bool FieldCacheRangeFilterString::equals(LuceneObjectPtr other)
    {
        if (Filter::equals(other))
            return true;
        FieldCacheRangeFilterStringPtr otherFilter(boost::dynamic_pointer_cast<FieldCacheRangeFilterString>(other));
        if (!otherFilter)
            return false;
        if (field != otherFilter->field || includeLower != otherFilter->includeLower || includeUpper != otherFilter->includeUpper)
            return false;
        if (lowerVal != otherFilter->lowerVal || upperVal != otherFilter->upperVal)
            return false;
        if (parser ? !parser->equals(otherFilter->parser) : otherFilter->parser)
            return false;
        return true;
    }
    
    int32_t FieldCacheRangeFilterString::hashCode()
    {
        int32_t code = StringUtils::hashCode(field);
        code ^= lowerVal.empty() ? 550356204 : StringUtils::hashCode(lowerVal);
        code = (code << 1) | MiscUtils::unsignedShift(code, 31); // rotate to distinguish lower from upper
        code ^= upperVal.empty() ? -1674416163 : StringUtils::hashCode(upperVal);
        code ^= parser ? parser->hashCode() : -1572457324;
        code ^= (includeLower ? 1549299360 : -365038026) ^ (includeUpper ? 1721088258 : 1948649653);
        return code;
    }
    
    FieldCacheRangeFilterByte::FieldCacheRangeFilterByte(const String& field, ParserPtr parser, uint8_t lowerVal, uint8_t upperVal, bool includeLower, bool includeUpper)
        : FieldCacheRangeFilterNumeric<uint8_t>(field, parser, lowerVal, upperVal, UCHAR_MAX, includeLower, includeUpper)
    {
    }
    
    FieldCacheRangeFilterByte::~FieldCacheRangeFilterByte()
    {
    }
    
    Collection<uint8_t> FieldCacheRangeFilterByte::getValues(IndexReaderPtr reader)
    {
        return FieldCache::DEFAULT()->getBytes(reader, field, boost::static_pointer_cast<ByteParser>(parser));
    }
    
    FieldCacheRangeFilterInt::FieldCacheRangeFilterInt(const String& field, ParserPtr parser, int32_t lowerVal, int32_t upperVal, bool includeLower, bool includeUpper)
        : FieldCacheRangeFilterNumeric<int32_t>(field, parser, lowerVal, upperVal, INT_MAX, includeLower, includeUpper)
    {
    }
    
    FieldCacheRangeFilterInt::~FieldCacheRangeFilterInt()
    {
    }
    
    Collection<int32_t> FieldCacheRangeFilterInt::getValues(IndexReaderPtr reader)
    {
        return FieldCache::DEFAULT()->getInts(reader, field, boost::static_pointer_cast<IntParser>(parser));
    }
    
    FieldCacheRangeFilterLong::FieldCacheRangeFilterLong(const String& field, ParserPtr parser, int64_t lowerVal, int64_t upperVal, bool includeLower, bool includeUpper)
        : FieldCacheRangeFilterNumeric<int64_t>(field, parser, lowerVal, upperVal, std::numeric_limits<int64_t>::max(), includeLower, includeUpper)
    {
    }
    
    FieldCacheRangeFilterLong::~FieldCacheRangeFilterLong()
    {
    }
    
    Collection<int64_t> FieldCacheRangeFilterLong::getValues(IndexReaderPtr reader)
    {
        return FieldCache::DEFAULT()->getLongs(reader, field, boost::static_pointer_cast<LongParser>(parser));
    }
    
    FieldCacheRangeFilterDouble::FieldCacheRangeFilterDouble(const String& field, ParserPtr parser, double lowerVal, double upperVal, bool includeLower, bool includeUpper)
        : FieldCacheRangeFilterNumeric<double>(field, parser, lowerVal, upperVal, std::numeric_limits<double>::infinity(), includeLower, includeUpper)
    {
    }
    
    FieldCacheRangeFilterDouble::~FieldCacheRangeFilterDouble()
    {
    }
    
    DocIdSetPtr FieldCacheRangeFilterDouble::getDocIdSet(IndexReaderPtr reader)
    {
        if (!includeLower && lowerVal > 0.0 && MiscUtils::isInfinite(lowerVal))
            return DocIdSet::EMPTY_DOCIDSET();
        int64_t lower = NumericUtils::doubleToSortableLong(lowerVal);
        double inclusiveLowerPoint = NumericUtils::sortableLongToDouble(includeLower ? lower : (lower + 1));
        
        if (!includeUpper && upperVal < 0.0 && MiscUtils::isInfinite(upperVal))
            return DocIdSet::EMPTY_DOCIDSET();
        int64_t upper = NumericUtils::doubleToSortableLong(upperVal);
        double inclusiveUpperPoint = NumericUtils::sortableLongToDouble(includeUpper ? upper : (upper - 1));
        
        if (inclusiveLowerPoint > inclusiveUpperPoint)
            return DocIdSet::EMPTY_DOCIDSET();
        
        // we only request the usage of termDocs, if the range contains 0
        return newLucene< FieldCacheDocIdSetNumeric<double> >(reader, (inclusiveLowerPoint <= 0 && inclusiveUpperPoint >= 0), getValues(reader), inclusiveLowerPoint, inclusiveUpperPoint);
    }
    
    Collection<double> FieldCacheRangeFilterDouble::getValues(IndexReaderPtr reader)
    {
        return FieldCache::DEFAULT()->getDoubles(reader, field, boost::static_pointer_cast<DoubleParser>(parser));
    }
    
    FieldCacheDocIdSet::FieldCacheDocIdSet(IndexReaderPtr reader, bool mayUseTermDocs)
    {
        this->reader = reader;
        this->mayUseTermDocs = mayUseTermDocs;
    }
    
    FieldCacheDocIdSet::~FieldCacheDocIdSet()
    {
    }
    
    bool FieldCacheDocIdSet::isCacheable()
    {
        return !(mayUseTermDocs && reader->hasDeletions());
    }
    
    DocIdSetIteratorPtr FieldCacheDocIdSet::iterator()
    {
        // Synchronization needed because deleted docs BitVector can change after call to hasDeletions until 
        // TermDocs creation.  We only use an iterator with termDocs, when this was requested (eg. range 
        // contains 0) and the index has deletions
        TermDocsPtr termDocs;
        {
            SyncLock instancesLock(reader);
            termDocs = isCacheable() ? TermDocsPtr() : reader->termDocs(TermPtr());
        }
        if (termDocs)
        {
            // a DocIdSetIterator using TermDocs to iterate valid docIds
            return newLucene<FieldDocIdSetIteratorTermDocs>(shared_from_this(), termDocs);
        }
        else
        {
            // a DocIdSetIterator generating docIds by incrementing a variable - this one can be used if there 
            // are no deletions are on the index
            return newLucene<FieldDocIdSetIteratorIncrement>(shared_from_this());
        }
    }
    
    FieldCacheDocIdSetString::FieldCacheDocIdSetString(IndexReaderPtr reader, bool mayUseTermDocs, StringIndexPtr fcsi, int32_t inclusiveLowerPoint, int32_t inclusiveUpperPoint) : FieldCacheDocIdSet(reader, mayUseTermDocs)
    {
        this->fcsi = fcsi;
        this->inclusiveLowerPoint = inclusiveLowerPoint;
        this->inclusiveUpperPoint = inclusiveUpperPoint;
    }
    
    FieldCacheDocIdSetString::~FieldCacheDocIdSetString()
    {
    }
    
    bool FieldCacheDocIdSetString::matchDoc(int32_t doc)
    {
        if (doc < 0 || doc >= fcsi->order.size())
            boost::throw_exception(IndexOutOfBoundsException());
        return (fcsi->order[doc] >= inclusiveLowerPoint && fcsi->order[doc] <= inclusiveUpperPoint);
    }
    
    FieldDocIdSetIteratorTermDocs::FieldDocIdSetIteratorTermDocs(FieldCacheDocIdSetPtr cacheDocIdSet, TermDocsPtr termDocs)
    {
        this->_cacheDocIdSet = cacheDocIdSet;
        this->termDocs = termDocs;
        this->doc = -1;
    }
    
    FieldDocIdSetIteratorTermDocs::~FieldDocIdSetIteratorTermDocs()
    {
    }
    
    int32_t FieldDocIdSetIteratorTermDocs::docID()
    {
        return doc;
    }
    
    int32_t FieldDocIdSetIteratorTermDocs::nextDoc()
    {
        FieldCacheDocIdSetPtr cacheDocIdSet(_cacheDocIdSet);
        do
        {
            if (!termDocs->next())
            {
                doc = NO_MORE_DOCS;
                return doc;
            }
        }
        while (!cacheDocIdSet->matchDoc(doc = termDocs->doc()));
        return doc;
    }
    
    int32_t FieldDocIdSetIteratorTermDocs::advance(int32_t target)
    {
        FieldCacheDocIdSetPtr cacheDocIdSet(_cacheDocIdSet);
        if (!termDocs->skipTo(target))
        {
            doc = NO_MORE_DOCS;
            return doc;
        }
        while (!cacheDocIdSet->matchDoc(doc = termDocs->doc()))
        {
            if (!termDocs->next())
            {
                doc = NO_MORE_DOCS;
                return doc;
            }
        }
        return doc;
    }
    
    FieldDocIdSetIteratorIncrement::FieldDocIdSetIteratorIncrement(FieldCacheDocIdSetPtr cacheDocIdSet)
    {
        this->_cacheDocIdSet = cacheDocIdSet;
        this->doc = -1;
    }
    
    FieldDocIdSetIteratorIncrement::~FieldDocIdSetIteratorIncrement()
    {
    }
    
    int32_t FieldDocIdSetIteratorIncrement::docID()
    {
        return doc;
    }
    
    int32_t FieldDocIdSetIteratorIncrement::nextDoc()
    {
        FieldCacheDocIdSetPtr cacheDocIdSet(_cacheDocIdSet);
        try
        {
            do
            {
                ++doc;
            }
            while (!cacheDocIdSet->matchDoc(doc));
            return doc;
        }
        catch (IndexOutOfBoundsException&)
        {
            doc = NO_MORE_DOCS;
            return doc;
        }
    }
    
    int32_t FieldDocIdSetIteratorIncrement::advance(int32_t target)
    {
        FieldCacheDocIdSetPtr cacheDocIdSet(_cacheDocIdSet);
        try
        {
            doc = target;
            while (!cacheDocIdSet->matchDoc(doc))
                ++doc;
            return doc;
        }
        catch (IndexOutOfBoundsException&)
        {
            doc = NO_MORE_DOCS;
            return doc;
        }
    }
}
