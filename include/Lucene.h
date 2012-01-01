/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef LUCENE_H
#define LUCENE_H

#include "Config.h"

#include <wctype.h>
#include <wchar.h>
#include <sys/types.h>
#include <sstream>
#include <string>
#include <algorithm>
#include <limits>
#include <stdexcept>

#include <boost/cstdint.hpp>
#include <boost/variant.hpp>

using boost::int8_t;
using boost::uint8_t;
using boost::int16_t;
using boost::uint16_t;
using boost::int32_t;
using boost::uint32_t;
using boost::int64_t;
using boost::uint64_t;

#define SIZEOF_ARRAY(arr) (sizeof(arr) / sizeof((arr)[0]))

#include "gc.h"
#include "LuceneTypes.h"
#include "LuceneFactory.h"

namespace boost
{
    struct blank;
    class thread;
    class any;
    template < typename Signature > class function;
    namespace interprocess
    {
        class file_lock;
    }
    namespace posix_time
    {
        class ptime;
    }
}

namespace Lucene
{
    typedef std::basic_string<char> SingleString;
    typedef std::basic_ostringstream<char> SingleStringStream;
    typedef std::basic_string<wchar_t> String;
    typedef std::basic_ostringstream<wchar_t> StringStream;

    const std::basic_string<wchar_t> EmptyString;

    typedef gc_ptr<std::locale> localePtr;
}

#include "LuceneException.h"
#include "Collection.h"
#include "Map.h"
#include "Set.h"
#include "Array.h"
#include "Constants.h"

namespace Lucene
{
    typedef Array<uint8_t> ByteArray;
    typedef Array<uint16_t> ShortArray;
    typedef Array<int32_t> IntArray;
    typedef Array<int64_t> LongArray;
    typedef Array<wchar_t> Collection<CharArray>;
    typedef Array<double> DoubleArray;

    template <class T>
    struct luceneEquals
    {
        inline bool operator()(const T& first, const T& second) const
        {
            return first ? first->equals(second) : (!first && !second);
        }
    };

    template <class T>
    struct luceneEqualTo
    {
        luceneEqualTo(const T& type) : equalType(type) {}
        inline bool operator()(const T& other) const
        {
            return equalType->equals(other);
        }
        const T& equalType;
    };

    // todo
    // template <class T>
    // struct luceneWeakEquals
    // {
    //     inline bool operator()(const T& first, const T& second) const
    //     {
    //         if (first.expired() || second.expired())
    //             return (first.expired() && second.expired());
    //         return first.lock()->equals(second.lock());
    //     }
    // };

    template <class T>
    struct luceneHash : std::unary_function<T, std::size_t>
    {
        std::size_t operator()(const T& type) const
        {
            return type ? type->hashCode() : 0;
        }
    };

    // todo
    // template <class T>
    // struct luceneWeakHash : std::unary_function<T, std::size_t>
    // {
    //     std::size_t operator()(const T& type) const
    //     {
    //         return type.expired() ? 0 : type.lock()->hashCode();
    //     }
    // };

    template <class T>
    struct luceneCompare
    {
        inline bool operator()(const T& first, const T& second) const
        {
            if (!second)
                return false;
            if (!first)
                return true;
            return (first->compareTo(second) < 0);
        }
    };

    typedef SortedSet< int32_t > SetInt;
    typedef HashSet< String > SetString;
    typedef HashSet< SegmentInfoPtr, luceneHash<SegmentInfoPtr>, luceneEquals<SegmentInfoPtr> > SetSegmentInfo;
    typedef HashSet< MergeThreadPtr, luceneHash<MergeThreadPtr>, luceneEquals<MergeThreadPtr> > SetMergeThread;
    typedef HashSet< OneMergePtr, luceneHash<OneMergePtr>, luceneEquals<OneMergePtr> > SetOneMerge;
    typedef HashSet< QueryPtr, luceneHash<QueryPtr>, luceneEquals<QueryPtr> > SetQuery;
    typedef HashSet< TermPtr, luceneHash<TermPtr>, luceneEquals<TermPtr> > SetTerm;
    typedef HashSet< BooleanClausePtr, luceneHash<BooleanClausePtr>, luceneEquals<BooleanClausePtr> > SetBooleanClause;
    typedef HashSet< ReaderFieldPtr, luceneHash<ReaderFieldPtr>, luceneEquals<ReaderFieldPtr> > SetReaderField;
    typedef HashSet< ByteArray > SetByteArray;

    typedef HashMap< String, String > MapStringString;
    typedef HashMap< wchar_t, NormalizeCharMapPtr > MapCharNormalizeCharMap;
    typedef HashMap< String, AnalyzerPtr > MapStringAnalyzer;
    typedef HashMap< String, ByteArray > MapStringByteArray;
    typedef HashMap< String, int32_t > MapStringInt;
    typedef HashMap< String, FieldInfoPtr > MapStringFieldInfo;
    typedef HashMap< String, Collection<TermVectorEntryPtr> > MapStringCollectionTermVectorEntry;
    typedef HashMap< String, RefCountPtr > MapStringRefCount;
    typedef HashMap< int32_t, TermVectorsPositionInfoPtr > MapIntTermVectorsPositionInfo;
    typedef HashMap< String, MapIntTermVectorsPositionInfo > MapStringMapIntTermVectorsPositionInfo;
    typedef HashMap< String, NormPtr > MapStringNorm;
    typedef HashMap< String, TermVectorEntryPtr > MapStringTermVectorEntry;
    typedef HashMap< String, RAMFilePtr > MapStringRAMFile;
    typedef HashMap< int32_t, ByteArray > MapIntByteArray;
    typedef HashMap< int32_t, FilterItemPtr > MapIntFilterItem;
    typedef HashMap< int32_t, double > MapIntDouble;
    typedef HashMap< int64_t, int32_t > MapLongInt;
    typedef HashMap< String, double > MapStringDouble;
    typedef HashMap< int32_t, CachePtr > MapStringCache;
    typedef HashMap< String, LockPtr > MapStringLock;

    typedef HashMap< SegmentInfoPtr, SegmentReaderPtr, luceneHash<SegmentInfoPtr>, luceneEquals<SegmentInfoPtr> > MapSegmentInfoSegmentReader;
    typedef HashMap< SegmentInfoPtr, int32_t, luceneHash<SegmentInfoPtr>, luceneEquals<SegmentInfoPtr> > MapSegmentInfoInt;
    typedef HashMap< DocFieldConsumerPerThreadPtr, Collection<DocFieldConsumerPerFieldPtr>, luceneHash<DocFieldConsumerPerThreadPtr>, luceneEquals<DocFieldConsumerPerThreadPtr> > MapDocFieldConsumerPerThreadCollectionDocFieldConsumerPerField;
    typedef HashMap< InvertedDocConsumerPerThreadPtr, Collection<InvertedDocConsumerPerFieldPtr>, luceneHash<InvertedDocConsumerPerThreadPtr>, luceneEquals<InvertedDocConsumerPerThreadPtr> > MapInvertedDocConsumerPerThreadCollectionInvertedDocConsumerPerField;
    typedef HashMap< InvertedDocEndConsumerPerThreadPtr, Collection<InvertedDocEndConsumerPerFieldPtr>, luceneHash<InvertedDocEndConsumerPerThreadPtr>, luceneEquals<InvertedDocEndConsumerPerThreadPtr> > MapInvertedDocEndConsumerPerThreadCollectionInvertedDocEndConsumerPerField;
    typedef HashMap< TermsHashConsumerPerThreadPtr, Collection<TermsHashConsumerPerFieldPtr>, luceneHash<TermsHashConsumerPerThreadPtr>, luceneEquals<TermsHashConsumerPerThreadPtr> > MapTermsHashConsumerPerThreadCollectionTermsHashConsumerPerField;
    typedef HashMap< FieldInfoPtr, Collection<NormsWriterPerFieldPtr>, luceneHash<FieldInfoPtr>, luceneEquals<FieldInfoPtr> > MapFieldInfoCollectionNormsWriterPerField;
    typedef HashMap< IndexReaderPtr, HashSet<String>, luceneHash<IndexReaderPtr>, luceneEquals<IndexReaderPtr> > MapIndexReaderSetString;
    typedef HashMap< TermPtr, int32_t, luceneHash<TermPtr>, luceneEquals<TermPtr> > MapTermInt;
    typedef HashMap< QueryPtr, int32_t, luceneHash<QueryPtr>, luceneEquals<QueryPtr> > MapQueryInt;
    typedef HashMap< EntryPtr, boost::any, luceneHash<EntryPtr>, luceneEquals<EntryPtr> > MapEntryAny;
    typedef HashMap< PhrasePositionsPtr, LuceneObjectPtr, luceneHash<PhrasePositionsPtr>, luceneEquals<PhrasePositionsPtr> > MapPhrasePositionsLuceneObject;
    typedef HashMap< ReaderFieldPtr, SetReaderField, luceneHash<ReaderFieldPtr>, luceneEquals<ReaderFieldPtr> > MapReaderFieldSetReaderField;

    // todo
    // typedef WeakHashMap< LuceneObjectWeakPtr, LuceneObjectPtr, luceneWeakHash<LuceneObjectWeakPtr>, luceneWeakEquals<LuceneObjectWeakPtr> > WeakMapObjectObject;
    // typedef WeakHashMap< LuceneObjectWeakPtr, MapEntryAny, luceneWeakHash<LuceneObjectWeakPtr>, luceneWeakEquals<LuceneObjectWeakPtr> > WeakMapLuceneObjectMapEntryAny;

    typedef HashMap< LuceneObjectPtr, LuceneObjectPtr, luceneHash<LuceneObjectPtr>, luceneEquals<LuceneObjectPtr> > WeakMapObjectObject;
    typedef HashMap< LuceneObjectPtr, MapEntryAny, luceneHash<LuceneObjectPtr>, luceneEquals<LuceneObjectPtr> > WeakMapLuceneObjectMapEntryAny;

    typedef SortedMap< String, AttributePtr > MapStringAttribute;
    typedef SortedMap< int64_t, DocumentsWriterThreadStatePtr > MapThreadDocumentsWriterThreadState;
    typedef SortedMap< String, IndexReaderPtr > MapStringIndexReader;
    typedef SortedMap< TermPtr, NumPtr, luceneCompare<TermPtr> > MapTermNum;

    typedef boost::function<bool (const TermVectorEntryPtr&, const TermVectorEntryPtr&)> TermVectorEntryComparator;

    template < class KEY, class VALUE, class HASH = boost::hash<KEY>, class EQUAL = std::equal_to<KEY> > class SimpleLRUCache;
    typedef SimpleLRUCache< TermPtr, TermInfoPtr, luceneHash<TermPtr>, luceneEquals<TermPtr> > TermInfoCache;
    typedef gc_ptr<TermInfoCache> TermInfoCachePtr;

    typedef boost::blank VariantNull;
    typedef boost::variant<String, int32_t, int64_t, double, ReaderPtr, ByteArray, VariantNull> FieldsData;
    typedef boost::variant<String, uint8_t, int32_t, int64_t, double, VariantNull> ComparableValue;
    typedef boost::variant<int32_t, int64_t, double, VariantNull> NumericValue;
    typedef boost::variant<String, VariantNull> StringValue;
    typedef boost::variant<Collection<uint8_t>, Collection<int32_t>, Collection<double>, VariantNull> CollectionValue;
}

#include "Synchronize.h"
#if defined(LPP_BUILDING_LIB) || defined(LPP_EXPOSE_INTERNAL)
#define INTERNAL public
#else
#define INTERNAL protected
#endif

#endif
