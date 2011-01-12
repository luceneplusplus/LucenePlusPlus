/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef FIELDCACHERANGEFILTER_H
#define FIELDCACHERANGEFILTER_H

#include "Filter.h"
#include "DocIdSet.h"
#include "DocIdSetIterator.h"
#include "FieldCache.h"

namespace Lucene
{
    /// A range filter built on top of a cached single term field (in {@link FieldCache}).
    ///
    /// FieldCacheRangeFilter builds a single cache for the field the first time it is used.  Each subsequent 
    /// FieldCacheRangeFilter on the same field then reuses this cache, even if the range itself changes. 
    ///
    /// This means that FieldCacheRangeFilter is much faster (sometimes more than 100x as fast) as building a 
    /// {@link TermRangeFilter}, if using a {@link #newStringRange}.  However, if the range never changes it is 
    /// slower (around 2x as slow) than building a CachingWrapperFilter on top of a single {@link TermRangeFilter}.
    ///
    /// For numeric data types, this filter may be significantly faster than {@link NumericRangeFilter}. 
    /// Furthermore, it does not need the numeric values encoded by {@link NumericField}. But it has the problem 
    /// that it only works with exact one value/document (see below).
    ///
    /// As with all {@link FieldCache} based functionality, FieldCacheRangeFilter is only valid for fields which 
    /// exact one term for each document (except for {@link #newStringRange} where 0 terms are also allowed). Due 
    /// to a restriction of {@link FieldCache}, for numeric ranges all terms that do not have a numeric value, 0 
    /// is assumed.
    ///
    /// Thus it works on dates, prices and other single value fields but will not work on regular text fields. It 
    /// is preferable to use a NOT_ANALYZED field to ensure that there is only a single term. 
    ///
    /// Do not instantiate this template directly, use one of the static factory methods available, that create a 
    /// correct instance for different data types supported by {@link FieldCache}.
    class LPPAPI FieldCacheRangeFilter : public Filter
    {
    public:
        FieldCacheRangeFilter(const String& field, ParserPtr parser, bool includeLower, bool includeUpper);
        virtual ~FieldCacheRangeFilter();
        
        LUCENE_CLASS(FieldCacheRangeFilter);
    
    public:
        String field;
        ParserPtr parser;
        bool includeLower;
        bool includeUpper;
    
    public:
        /// Creates a string range filter using {@link FieldCache#getStringIndex}. This works with all fields containing 
        /// zero or one term in the field. The range can be half-open by setting one of the values to null.
        static FieldCacheRangeFilterPtr newStringRange(const String& field, const String& lowerVal, const String& upperVal, bool includeLower, bool includeUpper);
        
        /// Creates a numeric range filter using {@link FieldCache#getBytes(IndexReaderPtr, String)}. This works with all
        /// byte fields containing exactly one numeric term in the field. The range can be half-open by setting one of the 
        /// values to null.
        static FieldCacheRangeFilterPtr newByteRange(const String& field, uint8_t lowerVal, uint8_t upperVal, bool includeLower, bool includeUpper);
        
        /// Creates a numeric range filter using {@link FieldCache#getBytes(IndexReaderPtr, String, ByteParserPtr)}. This 
        /// works with all byte fields containing exactly one numeric term in the field.  The range can be half-open by 
        /// setting one of the values to null.
        static FieldCacheRangeFilterPtr newByteRange(const String& field, ByteParserPtr parser, uint8_t lowerVal, uint8_t upperVal, bool includeLower, bool includeUpper);
        
        /// Creates a numeric range filter using {@link FieldCache#getInts(IndexReaderPtr, String)}. This works with all
        /// int fields containing exactly one numeric term in the field. The range can be half-open by setting one of the 
        /// values to null.
        static FieldCacheRangeFilterPtr newIntRange(const String& field, int32_t lowerVal, int32_t upperVal, bool includeLower, bool includeUpper);
        
        /// Creates a numeric range filter using {@link FieldCache#getInts(IndexReaderPtr, String, IntParserPtr)}. This 
        /// works with all int fields containing exactly one numeric term in the field.  The range can be half-open by 
        /// setting one of the values to null.
        static FieldCacheRangeFilterPtr newIntRange(const String& field, IntParserPtr parser, int32_t lowerVal, int32_t upperVal, bool includeLower, bool includeUpper);
        
        /// Creates a numeric range filter using {@link FieldCache#getLongs(IndexReaderPtr, String)}. This works with all
        /// long fields containing exactly one numeric term in the field. The range can be half-open by setting one of the 
        /// values to null.
        static FieldCacheRangeFilterPtr newLongRange(const String& field, int64_t lowerVal, int64_t upperVal, bool includeLower, bool includeUpper);
        
        /// Creates a numeric range filter using {@link FieldCache#getLongs(IndexReaderPtr, String, LongParserPtr)}. This 
        /// works with all long fields containing exactly one numeric term in the field.  The range can be half-open by 
        /// setting one of the values to null.
        static FieldCacheRangeFilterPtr newLongRange(const String& field, LongParserPtr parser, int64_t lowerVal, int64_t upperVal, bool includeLower, bool includeUpper);
        
        /// Creates a numeric range filter using {@link FieldCache#getDoubles(IndexReaderPtr, String)}. This works with all
        /// long fields containing exactly one numeric term in the field. The range can be half-open by setting one of the 
        /// values to null.
        static FieldCacheRangeFilterPtr newDoubleRange(const String& field, double lowerVal, double upperVal, bool includeLower, bool includeUpper);
        
        /// Creates a numeric range filter using {@link FieldCache#getDoubles(IndexReaderPtr, String, DoubleParserPtr)}. This 
        /// works with all long fields containing exactly one numeric term in the field.  The range can be half-open by 
        /// setting one of the values to null.
        static FieldCacheRangeFilterPtr newDoubleRange(const String& field, DoubleParserPtr parser, double lowerVal, double upperVal, bool includeLower, bool includeUpper);
        
        virtual String toString() = 0;
        virtual bool equals(LuceneObjectPtr other) = 0;
        virtual int32_t hashCode() = 0;
        
        /// Returns the field name for this filter
        virtual String getField();
        
        /// Returns true if the lower endpoint is inclusive
        virtual bool includesLower();
        
        /// Returns true if the upper endpoint is inclusive
        virtual bool includesUpper();
        
        /// Returns the current numeric parser
        virtual ParserPtr getParser();
    };
    
    class LPPAPI FieldCacheRangeFilterString : public FieldCacheRangeFilter
    {
    public:
        FieldCacheRangeFilterString(const String& field, ParserPtr parser, const String& lowerVal, const String& upperVal, bool includeLower, bool includeUpper);
        virtual ~FieldCacheRangeFilterString();
        
        LUCENE_CLASS(FieldCacheRangeFilterString);
    
    public:
        String lowerVal;
        String upperVal;
    
    public:
        virtual DocIdSetPtr getDocIdSet(IndexReaderPtr reader);
        
        virtual String toString();
        virtual bool equals(LuceneObjectPtr other);
        virtual int32_t hashCode();
    };
    
    class LPPAPI FieldCacheDocIdSet : public DocIdSet
    {
    public:
        FieldCacheDocIdSet(IndexReaderPtr reader, bool mayUseTermDocs);
        virtual ~FieldCacheDocIdSet();
        
        LUCENE_CLASS(FieldCacheDocIdSet);
    
    protected:
        IndexReaderPtr reader;
        bool mayUseTermDocs;
    
    public:
        /// This method checks, if a doc is a hit, should throw ArrayIndexOutOfBounds, when position invalid
        virtual bool matchDoc(int32_t doc) = 0;
        
        /// This DocIdSet is cacheable, if it works solely with FieldCache and no TermDocs.
        virtual bool isCacheable();
        
        virtual DocIdSetIteratorPtr iterator();
    };
    
    template <typename TYPE>
    class FieldCacheDocIdSetNumeric : public FieldCacheDocIdSet
    {
    public:
        FieldCacheDocIdSetNumeric(IndexReaderPtr reader, bool mayUseTermDocs, Collection<TYPE> values, TYPE inclusiveLowerPoint, TYPE inclusiveUpperPoint) : FieldCacheDocIdSet(reader, mayUseTermDocs)
        {
            this->values = values;
            this->inclusiveLowerPoint = inclusiveLowerPoint;
            this->inclusiveUpperPoint = inclusiveUpperPoint;
        }
        
        virtual ~FieldCacheDocIdSetNumeric()
        {
        }
        
    protected:
        Collection<TYPE> values;
        TYPE inclusiveLowerPoint;
        TYPE inclusiveUpperPoint;
    
    public:
        virtual bool matchDoc(int32_t doc)
        {
            if (doc < 0 || doc >= values.size())
                boost::throw_exception(IndexOutOfBoundsException());
            return (values[doc] >= inclusiveLowerPoint && values[doc] <= inclusiveUpperPoint);
        }
    };    
    
    template <typename TYPE>
    class FieldCacheRangeFilterNumeric : public FieldCacheRangeFilter
    {
    public:
        FieldCacheRangeFilterNumeric(const String& field, ParserPtr parser, TYPE lowerVal, TYPE upperVal, TYPE maxVal, bool includeLower, bool includeUpper) : FieldCacheRangeFilter(field, parser, includeLower, includeUpper)
        {
            this->lowerVal = lowerVal;
            this->upperVal = upperVal;
            this->maxVal = maxVal;
        }
        
        virtual ~FieldCacheRangeFilterNumeric()
        {
        }
        
    public:
        TYPE lowerVal;
        TYPE upperVal;
        TYPE maxVal;
    
    public:
        virtual DocIdSetPtr getDocIdSet(IndexReaderPtr reader)
        {
            if (!includeLower && lowerVal == maxVal)
                return DocIdSet::EMPTY_DOCIDSET();
            TYPE inclusiveLowerPoint = (TYPE)(includeLower ? lowerVal : (lowerVal + 1));
            
            if (!includeUpper && upperVal == 0)
                return DocIdSet::EMPTY_DOCIDSET();
            TYPE inclusiveUpperPoint = (TYPE)(includeUpper ? upperVal : (upperVal - 1));
            
            if (inclusiveLowerPoint > inclusiveUpperPoint)
                return DocIdSet::EMPTY_DOCIDSET();
            
            // we only request the usage of termDocs, if the range contains 0
            return newLucene< FieldCacheDocIdSetNumeric<TYPE> >(reader, (inclusiveLowerPoint <= 0 && inclusiveUpperPoint >= 0), getValues(reader), inclusiveLowerPoint, inclusiveUpperPoint);
        }
        
        virtual Collection<TYPE> getValues(IndexReaderPtr reader) = 0;
        
        virtual String toString()
        {
            StringStream buffer;
            buffer << field << L":" << (includeLower ? L"[" : L"{");
            buffer << lowerVal << L" TO " << lowerVal;
            buffer << (includeLower ? L"]" : L"}");
            return buffer.str();
        }
        
        virtual bool equals(LuceneObjectPtr other)
        {
            if (Filter::equals(other))
                return true;
            boost::shared_ptr< FieldCacheRangeFilterNumeric<TYPE> > otherFilter(boost::dynamic_pointer_cast< FieldCacheRangeFilterNumeric<TYPE> >(other));
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
        
        int32_t hashCode()
        {
            int32_t code = StringUtils::hashCode(field);
            code ^= lowerVal == 0 ? 550356204 : (int32_t)lowerVal;
            code = (code << 1) | MiscUtils::unsignedShift(code, 31); // rotate to distinguish lower from upper
            code ^= upperVal == 0 ? -1674416163 : (int32_t)upperVal;
            code ^= parser ? parser->hashCode() : -1572457324;
            code ^= (includeLower ? 1549299360 : -365038026) ^ (includeUpper ? 1721088258 : 1948649653);
            return code;
        }
    };
    
    class LPPAPI FieldCacheRangeFilterByte : public FieldCacheRangeFilterNumeric<uint8_t>
    {
    public:
        FieldCacheRangeFilterByte(const String& field, ParserPtr parser, uint8_t lowerVal, uint8_t upperVal, bool includeLower, bool includeUpper);
        virtual ~FieldCacheRangeFilterByte();
        
        LUCENE_CLASS(FieldCacheRangeFilterByte);
    
    public:
        virtual Collection<uint8_t> getValues(IndexReaderPtr reader);
    };
    
    class LPPAPI FieldCacheRangeFilterInt : public FieldCacheRangeFilterNumeric<int32_t>
    {
    public:
        FieldCacheRangeFilterInt(const String& field, ParserPtr parser, int32_t lowerVal, int32_t upperVal, bool includeLower, bool includeUpper);
        virtual ~FieldCacheRangeFilterInt();
        
        LUCENE_CLASS(FieldCacheRangeFilterInt);
    
    public:
        virtual Collection<int32_t> getValues(IndexReaderPtr reader);
    };
    
    class LPPAPI FieldCacheRangeFilterLong : public FieldCacheRangeFilterNumeric<int64_t>
    {
    public:
        FieldCacheRangeFilterLong(const String& field, ParserPtr parser, int64_t lowerVal, int64_t upperVal, bool includeLower, bool includeUpper);
        virtual ~FieldCacheRangeFilterLong();
        
        LUCENE_CLASS(FieldCacheRangeFilterLong);
    
    public:
        virtual Collection<int64_t> getValues(IndexReaderPtr reader);
    };
    
    class LPPAPI FieldCacheRangeFilterDouble : public FieldCacheRangeFilterNumeric<double>
    {
    public:
        FieldCacheRangeFilterDouble(const String& field, ParserPtr parser, double lowerVal, double upperVal, bool includeLower, bool includeUpper);
        virtual ~FieldCacheRangeFilterDouble();
        
        LUCENE_CLASS(FieldCacheRangeFilterDouble);
    
    public:
        virtual DocIdSetPtr getDocIdSet(IndexReaderPtr reader);
        virtual Collection<double> getValues(IndexReaderPtr reader);
    };
    
    class LPPAPI FieldCacheDocIdSetString : public FieldCacheDocIdSet
    {
    public:
        FieldCacheDocIdSetString(IndexReaderPtr reader, bool mayUseTermDocs, StringIndexPtr fcsi, int32_t inclusiveLowerPoint, int32_t inclusiveUpperPoint);
        virtual ~FieldCacheDocIdSetString();
        
        LUCENE_CLASS(FieldCacheDocIdSetString);
    
    protected:
        StringIndexPtr fcsi;
        int32_t inclusiveLowerPoint;
        int32_t inclusiveUpperPoint;
    
    public:
        virtual bool matchDoc(int32_t doc);
    };
    
    /// A DocIdSetIterator using TermDocs to iterate valid docIds
    class LPPAPI FieldDocIdSetIteratorTermDocs : public DocIdSetIterator
    {
    public:
        FieldDocIdSetIteratorTermDocs(FieldCacheDocIdSetPtr cacheDocIdSet, TermDocsPtr termDocs);
        virtual ~FieldDocIdSetIteratorTermDocs();
        
        LUCENE_CLASS(FieldDocIdSetIteratorTermDocs);
    
    protected:
        FieldCacheDocIdSetWeakPtr _cacheDocIdSet;
        TermDocsPtr termDocs;
        int32_t doc;
    
    public:
        virtual int32_t docID();
        virtual int32_t nextDoc();
        virtual int32_t advance(int32_t target);
    };
    
    /// A DocIdSetIterator generating docIds by incrementing a variable - this one can be used if there 
    /// are no deletions are on the index.
    class LPPAPI FieldDocIdSetIteratorIncrement : public DocIdSetIterator
    {
    public:
        FieldDocIdSetIteratorIncrement(FieldCacheDocIdSetPtr cacheDocIdSet);
        virtual ~FieldDocIdSetIteratorIncrement();
        
        LUCENE_CLASS(FieldDocIdSetIteratorIncrement);
    
    protected:
        FieldCacheDocIdSetWeakPtr _cacheDocIdSet;
        int32_t doc;
    
    public:
        virtual int32_t docID();
        virtual int32_t nextDoc();
        virtual int32_t advance(int32_t target);
    };
}

#endif
