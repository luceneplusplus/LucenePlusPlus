/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "TermInfosReader.h"
#include "_TermInfosReader.h"
#include "SegmentTermEnum.h"
#include "Directory.h"
#include "IndexFileNames.h"
#include "Term.h"
#include "StringUtils.h"

namespace Lucene
{
    const int32_t TermInfosReader::DEFAULT_CACHE_SIZE = 1024;
    
    TermInfosReader::TermInfosReader(DirectoryPtr dir, const String& seg, FieldInfosPtr fis, int32_t readBufferSize, int32_t indexDivisor)
    {
        this->termsCache = newLucene<TermInfoCache>(DEFAULT_CACHE_SIZE);
        
        bool success = false;
        
        if (indexDivisor < 1 && indexDivisor != -1)
            boost::throw_exception(IllegalArgumentException(L"indexDivisor must be -1 (don't load terms index) or greater than 0: got " + StringUtils::toString(indexDivisor)));
        
        LuceneException finally;
        try
        {
            directory = dir;
            segment = seg;
            fieldInfos = fis;
            
            origEnum = newLucene<SegmentTermEnum>(directory->openInput(IndexFileNames::segmentFileName(segment, IndexFileNames::TERMS_EXTENSION()), readBufferSize), fieldInfos, false);
            _size = origEnum->size;
            
            if (indexDivisor != -1)
            {
                // Load terms index
                totalIndexInterval = origEnum->indexInterval * indexDivisor;
                SegmentTermEnumPtr indexEnum(newLucene<SegmentTermEnum>(directory->openInput(IndexFileNames::segmentFileName(segment, IndexFileNames::TERMS_INDEX_EXTENSION()), readBufferSize), fieldInfos, true));
                
                try
                {
                    int32_t indexSize = 1 + ((int32_t)indexEnum->size - 1) / indexDivisor; // otherwise read index
                    
                    indexTerms = Collection<TermPtr>::newInstance(indexSize);
                    indexInfos = Collection<TermInfoPtr>::newInstance(indexSize);
                    indexPointers = Collection<int64_t>::newInstance(indexSize);
                    
                    for (int32_t i = 0; indexEnum->next(); ++i)
                    {
                        indexTerms[i] = indexEnum->term();
                        indexInfos[i] = indexEnum->termInfo();
                        indexPointers[i] = indexEnum->indexPointer;
                        
                        for (int32_t j = 1; j < indexDivisor; ++j)
                        {
                            if (!indexEnum->next())
                                break;
                        }
                    }
                }
                catch (LuceneException& e)
                {
                    finally = e;
                }
                indexEnum->close();
            }
            else
            {
                // Do not load terms index
                totalIndexInterval = -1;
            }
            success = true;
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        // With lock-less commits, it's entirely possible (and fine) to hit a FileNotFound exception above. 
        // In this case, we want to explicitly close any subset of things that were opened.
        if (!success)
            close();
        finally.throwException();
    }
    
    TermInfosReader::~TermInfosReader()
    {
    }
    
    int32_t TermInfosReader::getMaxSkipLevels()
    {
        return origEnum->maxSkipLevels;
    }
    
    int32_t TermInfosReader::getSkipInterval()
    {
        return origEnum->skipInterval;
    }
    
    void TermInfosReader::close()
    {
        if (origEnum)
            origEnum->close();
        threadResources.close();
    }
    
    int64_t TermInfosReader::size()
    {
        return _size;
    }
    
    TermInfosReaderThreadResourcesPtr TermInfosReader::getThreadResources()
    {
        TermInfosReaderThreadResourcesPtr resources(threadResources.get());
        if (!resources)
        {
            resources = newLucene<TermInfosReaderThreadResources>();
            resources->termEnum = terms();
            threadResources.set(resources);
        }
        return resources;
    }
    
    int32_t TermInfosReader::getIndexOffset(TermPtr term)
    {
        // binary search indexTerms
        Collection<TermPtr>::iterator indexTerm = std::upper_bound(indexTerms.begin(), indexTerms.end(), term, luceneCompare<TermPtr>());
        return (std::distance(indexTerms.begin(), indexTerm) - 1);
    }
    
    void TermInfosReader::seekEnum(SegmentTermEnumPtr enumerator, int32_t indexOffset)
    {
        enumerator->seek(indexPointers[indexOffset], ((int64_t)indexOffset * (int64_t)totalIndexInterval) - 1, indexTerms[indexOffset], indexInfos[indexOffset]);
    }
    
    TermInfoPtr TermInfosReader::get(TermPtr term)
    {
        return get(term, false);
    }
    
    TermInfoPtr TermInfosReader::get(TermPtr term, bool mustSeekEnum)
    {
        if (_size == 0)
            return TermInfoPtr();
        
        ensureIndexIsRead();
        
        TermPtr cacheKey(term);
        TermInfoAndOrdPtr tiOrd(termsCache->get(cacheKey));
        TermInfosReaderThreadResourcesPtr resources(getThreadResources());
        
        if (!mustSeekEnum && tiOrd)
            return tiOrd;
        
        // optimize sequential access: first try scanning cached enum without seeking
        SegmentTermEnumPtr enumerator = resources->termEnum;
        
        if (enumerator->term() && // term is at or past current
            ((enumerator->prev() && term->compareTo(enumerator->prev()) > 0) ||
            term->compareTo(enumerator->term()) >= 0))
        {
            int32_t enumOffset = (int32_t)(enumerator->position / totalIndexInterval ) + 1;
            if (indexTerms.size() == enumOffset || // but before end of block
                term->compareTo(indexTerms[enumOffset]) < 0)
            {
                // no need to seek
                TermInfoPtr ti;
                
                int32_t numScans = enumerator->scanTo(term);
                if (enumerator->term() && term->compareTo(enumerator->term()) == 0)
                {
                    ti = enumerator->termInfo();
                    if (numScans > 1)
                    {
                        // we only  want to put this TermInfo into the cache if scanEnum skipped more than one dictionary entry.
                        // This prevents RangeQueries or WildcardQueries to wipe out the cache when they iterate over a large 
                        // numbers of terms in order
                        if (!tiOrd)
                            termsCache->put(cacheKey, newLucene<TermInfoAndOrd>(ti, (int32_t)enumerator->position));
                        else
                        {
                            BOOST_ASSERT(sameTermInfo(ti, tiOrd, enumerator));
                            BOOST_ASSERT((int32_t)enumerator->position == tiOrd->termOrd);
                        }
                    }
                }
                else
                    ti.reset();
                return ti;
            }
        }
        
        // random-access: must seek
        int32_t indexPos;
        if (tiOrd)
            indexPos = tiOrd->termOrd / totalIndexInterval;
        else
        {
            // Must do binary search
            indexPos = getIndexOffset(term);
        }

        seekEnum(enumerator, indexPos);
        enumerator->scanTo(term);
        TermInfoPtr ti;
        if (enumerator->term() && term->compareTo(enumerator->term()) == 0)
        {
            ti = enumerator->termInfo();
            if (!tiOrd)
                termsCache->put(cacheKey, newLucene<TermInfoAndOrd>(ti, (int32_t)enumerator->position));
            else
            {
                BOOST_ASSERT(sameTermInfo(ti, tiOrd, enumerator));
                BOOST_ASSERT((int32_t)enumerator->position == tiOrd->termOrd);
            }
        }
        else
            ti.reset();
        return ti;
    }
    
    bool TermInfosReader::sameTermInfo(TermInfoPtr ti1, TermInfoPtr ti2, SegmentTermEnumPtr enumerator)
    {
        if (ti1->docFreq != ti2->docFreq)
            return false;
        if (ti1->freqPointer != ti2->freqPointer)
            return false;
        if (ti1->proxPointer != ti2->proxPointer)
            return false;
        // skipOffset is only valid when docFreq >= skipInterval
        if (ti1->docFreq >= enumerator->skipInterval && ti1->skipOffset != ti2->skipOffset)
            return false;
        return true;
    }
    
    void TermInfosReader::ensureIndexIsRead()
    {
        if (!indexTerms)
            boost::throw_exception(IllegalStateException(L"terms index was not loaded when this reader was created"));
    }
    
    int64_t TermInfosReader::getPosition(TermPtr term)
    {
        if (_size == 0)
            return -1;
        
        ensureIndexIsRead();
        int32_t indexOffset = getIndexOffset(term);
        
        SegmentTermEnumPtr enumerator(getThreadResources()->termEnum);
        seekEnum(enumerator, indexOffset);
        
        while (term->compareTo(enumerator->term()) > 0 && enumerator->next())
        {
        }
        
        return term->compareTo(enumerator->term()) == 0 ? enumerator->position : -1;
    }
    
    SegmentTermEnumPtr TermInfosReader::terms()
    {
        return boost::static_pointer_cast<SegmentTermEnum>(origEnum->clone());
    }
    
    SegmentTermEnumPtr TermInfosReader::terms(TermPtr term)
    {
        get(term, true);
        return boost::static_pointer_cast<SegmentTermEnum>(getThreadResources()->termEnum->clone());
    }
    
    TermInfosReaderThreadResources::~TermInfosReaderThreadResources()
    {
    }
    
    TermInfoAndOrd::TermInfoAndOrd(TermInfoPtr ti, int32_t termOrd) : TermInfo(ti)
    {
        this->termOrd = termOrd;
    }
    
    TermInfoAndOrd::~TermInfoAndOrd()
    {
    }
}
