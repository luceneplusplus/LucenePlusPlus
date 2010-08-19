/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FieldCacheImpl.h"
#include "FieldCacheSanityChecker.h"
#include "IndexReader.h"
#include "InfoStream.h"
#include "TermEnum.h"
#include "TermDocs.h"
#include "Term.h"

namespace Lucene
{
    FieldCacheImpl::FieldCacheImpl()
    {
    }
    
    FieldCacheImpl::~FieldCacheImpl()
    {
    }
    
    void FieldCacheImpl::initialize()
    {
        caches = MapStringCache::newInstance();
        caches.put(L"Byte", newLucene<ByteCache>(shared_from_this()));
        caches.put(L"Integer", newLucene<IntCache>(shared_from_this()));
        caches.put(L"Long", newLucene<LongCache>(shared_from_this()));
        caches.put(L"Double", newLucene<DoubleCache>(shared_from_this()));
        caches.put(L"String", newLucene<StringCache>(shared_from_this()));
        caches.put(StringIndex::_getClassName(), newLucene<StringIndexCache>(shared_from_this()));
    }
    
    void FieldCacheImpl::purgeAllCaches()
    {
        initialize();
    }
    
    Collection<FieldCacheEntryPtr> FieldCacheImpl::getCacheEntries()
    {
        Collection<FieldCacheEntryPtr> result(Collection<FieldCacheEntryPtr>::newInstance());
        for (MapStringCache::iterator cache = caches.begin(); cache != caches.end(); ++cache)
        {
            for (WeakMapLuceneObjectMapEntryLuceneObject::iterator key = cache->second->readerCache.begin(); key != cache->second->readerCache.end(); ++key)
            {
                LuceneObjectPtr readerKey(key->first.lock());
                
                // we've now materialized a hard ref
                if (readerKey)
                {
                    for (MapEntryLuceneObject::iterator mapEntry = key->second.begin(); mapEntry != key->second.end(); ++mapEntry)
                        result.add(newLucene<FieldCacheEntryImpl>(readerKey, mapEntry->first->field, cache->first, mapEntry->first->custom, mapEntry->second));
                }
            }
        }
        return result;
    }
    
    Collection<uint8_t> FieldCacheImpl::getBytes(IndexReaderPtr reader, const String& field)
    {
        return getBytes(reader, field, ByteParserPtr());
    }
    
    Collection<uint8_t> FieldCacheImpl::getBytes(IndexReaderPtr reader, const String& field, ByteParserPtr parser)
    {
        return *boost::static_pointer_cast< Collection<uint8_t> >(caches.get(L"Byte")->get(reader, newLucene<Entry>(field, parser)));
    }
    
    Collection<int32_t> FieldCacheImpl::getInts(IndexReaderPtr reader, const String& field)
    {
        return getInts(reader, field, IntParserPtr());
    }
    
    Collection<int32_t> FieldCacheImpl::getInts(IndexReaderPtr reader, const String& field, IntParserPtr parser)
    {
        return *boost::static_pointer_cast< Collection<int32_t> >(caches.get(L"Integer")->get(reader, newLucene<Entry>(field, parser)));
    }
    
    Collection<int64_t> FieldCacheImpl::getLongs(IndexReaderPtr reader, const String& field)
    {
        return getLongs(reader, field, LongParserPtr());
    }
    
    Collection<int64_t> FieldCacheImpl::getLongs(IndexReaderPtr reader, const String& field, LongParserPtr parser)
    {
        return *boost::static_pointer_cast< Collection<int64_t> >(caches.get(L"Long")->get(reader, newLucene<Entry>(field, parser)));
    }
    
    Collection<double> FieldCacheImpl::getDoubles(IndexReaderPtr reader, const String& field)
    {
        return getDoubles(reader, field, DoubleParserPtr());
    }
    
    Collection<double> FieldCacheImpl::getDoubles(IndexReaderPtr reader, const String& field, DoubleParserPtr parser)
    {
        return *boost::static_pointer_cast< Collection<double> >(caches.get(L"Double")->get(reader, newLucene<Entry>(field, parser)));
    }
    
    Collection<String> FieldCacheImpl::getStrings(IndexReaderPtr reader, const String& field)
    {
        return *boost::static_pointer_cast< Collection<String> >(caches.get(L"String")->get(reader, newLucene<Entry>(field, ParserPtr())));
    }
    
    StringIndexPtr FieldCacheImpl::getStringIndex(IndexReaderPtr reader, const String& field)
    {
        return boost::static_pointer_cast<StringIndex>(caches.get(StringIndex::_getClassName())->get(reader, newLucene<Entry>(field, ParserPtr())));
    }
    
    void FieldCacheImpl::setInfoStream(InfoStreamPtr stream)
    {
        infoStream = stream;
    }
    
    InfoStreamPtr FieldCacheImpl::getInfoStream()
    {
        return infoStream;
    }
    
    Entry::Entry(const String& field, LuceneObjectPtr custom)
    {
        this->field = field;
        this->custom = custom;
    }
    
    Entry::~Entry()
    {
    }
    
    bool Entry::equals(LuceneObjectPtr other)
    {
        if (LuceneObject::equals(other))
            return true;
        
        EntryPtr otherEntry(boost::dynamic_pointer_cast<Entry>(other));
        if (otherEntry)
        {
            if (otherEntry->field == field)
            {
                if (!otherEntry->custom)
                {
                    if (!custom)
                        return true;
                }
                else if (otherEntry->custom->equals(custom))
                    return true;
            }
        }
        return false;
    }
    
    int32_t Entry::hashCode()
    {
        return StringUtils::hashCode(field) ^ (custom ? custom->hashCode() : 0);
    }
    
    Cache::Cache(FieldCachePtr wrapper)
    {
        this->_wrapper = wrapper;
        this->readerCache = WeakMapLuceneObjectMapEntryLuceneObject::newInstance();
    }
    
    Cache::~Cache()
    {
    }
    
    LuceneObjectPtr Cache::get(IndexReaderPtr reader, EntryPtr key)
    {
        MapEntryLuceneObject innerCache;
        LuceneObjectPtr value;
        LuceneObjectPtr readerKey(reader->getFieldCacheKey());
        {
            SyncLock cacheLock(&readerCache);
            innerCache = readerCache.get(readerKey);
            if (!innerCache)
            {
                innerCache = MapEntryLuceneObject::newInstance();
                readerCache.put(readerKey, innerCache);
            }
            else
                value = innerCache.get(key);
            if (!value)
            {
                value = newLucene<CreationPlaceholder>();
                innerCache.put(key, value);
            }
        }
        CreationPlaceholderPtr progress(boost::dynamic_pointer_cast<CreationPlaceholder>(value));
        if (progress)
        {
            SyncLock valueLock(value);
            if (!progress->value)
            {
                progress->value = createValue(reader, key);
                {
                    SyncLock cacheLock(&readerCache);
                    innerCache.put(key, progress->value);
                }
                
                FieldCachePtr wrapper(_wrapper);
                
                // Only check if key.custom (the parser) is non-null; else, we check twice for a single
                // call to FieldCache.getXXX
                if (key->custom && wrapper)
                {
                    InfoStreamPtr infoStream(wrapper->getInfoStream());
                    if (infoStream)
                        printNewInsanity(infoStream, progress->value);
                }
            }
            return progress->value;
        }
        return value;
    }
    
    void Cache::printNewInsanity(InfoStreamPtr infoStream, LuceneObjectPtr value)
    {
        Collection<InsanityPtr> insanities(FieldCacheSanityChecker::checkSanity(FieldCachePtr(_wrapper)));
        for (Collection<InsanityPtr>::iterator insanity = insanities.begin(); insanity != insanities.end(); ++insanity)
        {
            Collection<FieldCacheEntryPtr> entries((*insanity)->getCacheEntries());
            for (Collection<FieldCacheEntryPtr>::iterator entry = entries.begin(); entry != entries.end(); ++entry)
            {
                if ((*entry)->getValue() == value)
                {
                    // OK this insanity involves our entry
                    *infoStream << L"WARNING: new FieldCache insanity created\nDetails: " + (*insanity)->toString() << L"\n";
                    break;
                }
            }
        }
    }
    
    ByteCache::ByteCache(FieldCachePtr wrapper) : Cache(wrapper)
    {
    }
    
    ByteCache::~ByteCache()
    {
    }
    
    LuceneObjectPtr ByteCache::createValue(IndexReaderPtr reader, EntryPtr key)
    {
        EntryPtr entry(key);
        String field(entry->field);
        ByteParserPtr parser(boost::dynamic_pointer_cast<ByteParser>(entry->custom));
        if (!parser)
            return FieldCachePtr(_wrapper)->getBytes(reader, field, FieldCache::DEFAULT_BYTE_PARSER()).ptr();
        Collection<uint8_t> retArray(Collection<uint8_t>::newInstance(reader->maxDoc()));
        TermDocsPtr termDocs(reader->termDocs());
        TermEnumPtr termEnum(reader->terms(newLucene<Term>(field)));
        LuceneException finally;
        try
        {
            do
            {
                TermPtr term(termEnum->term());
                if (!term || term->field() != field)
                    break;
                uint8_t termval = parser->parseByte(term->text());
                termDocs->seek(termEnum);
                while (termDocs->next())
                    retArray[termDocs->doc()] = termval;
            }
            while (termEnum->next());
        }
        catch (StopFillCacheException&)
        {
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        termDocs->close();
        termEnum->close();
        finally.throwException();
        return retArray.ptr();
    }
    
    IntCache::IntCache(FieldCachePtr wrapper) : Cache(wrapper)
    {
    }
    
    IntCache::~IntCache()
    {
    }
    
    LuceneObjectPtr IntCache::createValue(IndexReaderPtr reader, EntryPtr key)
    {
        EntryPtr entry(key);
        String field(entry->field);
        IntParserPtr parser(boost::dynamic_pointer_cast<IntParser>(entry->custom));
        if (!parser)
        {
            FieldCachePtr wrapper(_wrapper);
            LuceneObjectPtr ints;
            try
            {
                ints = wrapper->getInts(reader, field, FieldCache::DEFAULT_INT_PARSER()).ptr();
            }
            catch (NumberFormatException&)
            {
                ints = wrapper->getInts(reader, field, FieldCache::NUMERIC_UTILS_INT_PARSER()).ptr();
            }
            return ints;
        }
        Collection<int32_t> retArray;
        TermDocsPtr termDocs(reader->termDocs());
        TermEnumPtr termEnum(reader->terms(newLucene<Term>(field)));
        LuceneException finally;
        try
        {
            do
            {
                TermPtr term(termEnum->term());
                if (!term || term->field() != field)
                    break;
                int32_t termval = parser->parseInt(term->text());
                if (!retArray) // late init
                    retArray = Collection<int32_t>::newInstance(reader->maxDoc());
                termDocs->seek(termEnum);
                while (termDocs->next())
                    retArray[termDocs->doc()] = termval;
            }
            while (termEnum->next());
        }
        catch (StopFillCacheException&)
        {
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        termDocs->close();
        termEnum->close();
        finally.throwException();
        if (!retArray) // no values
            retArray = Collection<int32_t>::newInstance(reader->maxDoc());
        return retArray.ptr();
    }
    
    LongCache::LongCache(FieldCachePtr wrapper) : Cache(wrapper)
    {
    }
    
    LongCache::~LongCache()
    {
    }
    
    LuceneObjectPtr LongCache::createValue(IndexReaderPtr reader, EntryPtr key)
    {
        EntryPtr entry(key);
        String field(entry->field);
        LongParserPtr parser(boost::dynamic_pointer_cast<LongParser>(entry->custom));
        if (!parser)
        {
            FieldCachePtr wrapper(_wrapper);
            LuceneObjectPtr longs;
            try
            {
                longs = wrapper->getLongs(reader, field, FieldCache::DEFAULT_LONG_PARSER()).ptr();
            }
            catch (NumberFormatException&)
            {
                longs = wrapper->getLongs(reader, field, FieldCache::NUMERIC_UTILS_LONG_PARSER()).ptr();
            }
            return longs;
        }
        Collection<int64_t> retArray;
        TermDocsPtr termDocs(reader->termDocs());
        TermEnumPtr termEnum(reader->terms(newLucene<Term>(field)));
        LuceneException finally;
        try
        {
            do
            {
                TermPtr term(termEnum->term());
                if (!term || term->field() != field)
                    break;
                int64_t termval = parser->parseLong(term->text());
                if (!retArray) // late init
                    retArray = Collection<int64_t>::newInstance(reader->maxDoc());
                termDocs->seek(termEnum);
                while (termDocs->next())
                    retArray[termDocs->doc()] = termval;
            }
            while (termEnum->next());
        }
        catch (StopFillCacheException&)
        {
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        termDocs->close();
        termEnum->close();
        finally.throwException();
        if (!retArray) // no values
            retArray = Collection<int64_t>::newInstance(reader->maxDoc());
        return retArray.ptr();
    }
    
    DoubleCache::DoubleCache(FieldCachePtr wrapper) : Cache(wrapper)
    {
    }
    
    DoubleCache::~DoubleCache()
    {
    }
    
    LuceneObjectPtr DoubleCache::createValue(IndexReaderPtr reader, EntryPtr key)
    {
        EntryPtr entry(key);
        String field(entry->field);
        DoubleParserPtr parser(boost::dynamic_pointer_cast<DoubleParser>(entry->custom));
        if (!parser)
        {
            FieldCachePtr wrapper(_wrapper);
            LuceneObjectPtr doubles;
            try
            {
                doubles = wrapper->getDoubles(reader, field, FieldCache::DEFAULT_DOUBLE_PARSER()).ptr();
            }
            catch (NumberFormatException&)
            {
                doubles = wrapper->getDoubles(reader, field, FieldCache::NUMERIC_UTILS_DOUBLE_PARSER()).ptr();
            }
            return doubles;
        }
        Collection<double> retArray;
        TermDocsPtr termDocs(reader->termDocs());
        TermEnumPtr termEnum(reader->terms(newLucene<Term>(field)));
        LuceneException finally;
        try
        {
            do
            {
                TermPtr term(termEnum->term());
                if (!term || term->field() != field)
                    break;
                double termval = parser->parseDouble(term->text());
                if (!retArray) // late init
                    retArray = Collection<double>::newInstance(reader->maxDoc());
                termDocs->seek(termEnum);
                while (termDocs->next())
                    retArray[termDocs->doc()] = termval;
            }
            while (termEnum->next());
        }
        catch (StopFillCacheException&)
        {
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        termDocs->close();
        termEnum->close();
        finally.throwException();
        if (!retArray) // no values
            retArray = Collection<double>::newInstance(reader->maxDoc());
        return retArray.ptr();
    }
    
    StringCache::StringCache(FieldCachePtr wrapper) : Cache(wrapper)
    {
    }
    
    StringCache::~StringCache()
    {
    }
    
    LuceneObjectPtr StringCache::createValue(IndexReaderPtr reader, EntryPtr key)
    {
        EntryPtr entry(key);
        String field(entry->field);
        Collection<String> retArray(Collection<String>::newInstance(reader->maxDoc()));
        TermDocsPtr termDocs(reader->termDocs());
        TermEnumPtr termEnum(reader->terms(newLucene<Term>(field)));
        LuceneException finally;
        try
        {
            do
            {
                TermPtr term(termEnum->term());
                if (!term || term->field() != field)
                    break;
                String termval(term->text());
                termDocs->seek(termEnum);
                while (termDocs->next())
                    retArray[termDocs->doc()] = termval;
            }
            while (termEnum->next());
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        termDocs->close();
        termEnum->close();
        finally.throwException();
        return retArray.ptr();
    }
    
    StringIndexCache::StringIndexCache(FieldCachePtr wrapper) : Cache(wrapper)
    {
    }
    
    StringIndexCache::~StringIndexCache()
    {
    }
    
    LuceneObjectPtr StringIndexCache::createValue(IndexReaderPtr reader, EntryPtr key)
    {
        EntryPtr entry(key);
        String field(entry->field);
        Collection<int32_t> retArray(Collection<int32_t>::newInstance(reader->maxDoc()));
        Collection<String> mterms(Collection<String>::newInstance(reader->maxDoc() + 1));
        TermDocsPtr termDocs(reader->termDocs());
        TermEnumPtr termEnum(reader->terms(newLucene<Term>(field)));
        int32_t t = 0; // current term number
        
        // an entry for documents that have no terms in this field should a document with no terms be at 
        // top or bottom?  This puts them at the top - if it is changed, FieldDocSortedHitQueue needs to 
        // change as well.
        mterms[t++] = L"";
        
        LuceneException finally;
        try
        {
            do
            {
                TermPtr term(termEnum->term());
                if (!term || term->field() != field)
                    break;
                
                // store term text
                // we expect that there is at most one term per document
                if (t >= mterms.size())
                {
                    boost::throw_exception(RuntimeException(L"There are more terms than documents in field\"" +
                                                            field + L"\", but it's impossible to sort on tokenized fields"));
                }
                mterms[t] = term->text();
                
                termDocs->seek(termEnum);
                while (termDocs->next())
                    retArray[termDocs->doc()] = t;
                
                ++t;
            }
            while (termEnum->next());
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        termDocs->close();
        termEnum->close();
        finally.throwException();
        
        if (t == 0)
        {
            // if there are no terms, make the term array have a single null entry
            mterms = Collection<String>::newInstance(1);
        }
        else if (t < mterms.size())
        {
            // if there are less terms than documents, trim off the dead array space
            mterms.resize(t);
        }
        
        return newLucene<StringIndex>(retArray, mterms);
    }
    
    FieldCacheEntryImpl::FieldCacheEntryImpl(LuceneObjectPtr readerKey, const String& fieldName, const String& cacheType, LuceneObjectPtr custom, LuceneObjectPtr value)
    {
        this->readerKey = readerKey;
        this->fieldName = fieldName;
        this->cacheType = cacheType;
        this->custom = custom;
        this->value = value;
    }
    
    FieldCacheEntryImpl::~FieldCacheEntryImpl()
    {
    }
    
    LuceneObjectPtr FieldCacheEntryImpl::getReaderKey()
    {
        return readerKey;
    }
    
    String FieldCacheEntryImpl::getFieldName()
    {
        return fieldName;
    }
    
    String FieldCacheEntryImpl::getCacheType()
    {
        return cacheType;
    }
    
    LuceneObjectPtr FieldCacheEntryImpl::getCustom()
    {
        return custom;
    }
    
    LuceneObjectPtr FieldCacheEntryImpl::getValue()
    {
        return value;
    }
}
