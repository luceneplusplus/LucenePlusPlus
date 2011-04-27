/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "BufferedDeletes.h"
#include "MergeDocIDRemapper.h"
#include "SegmentInfo.h"
#include "SegmentDeletes.h"
#include "SegmentReader.h"
#include "IndexWriter.h"
#include "_IndexWriter.h"
#include "IndexSearcher.h"
#include "AtomicLong.h"
#include "DateTools.h"
#include "Weight.h"
#include "Scorer.h"
#include "Query.h"
#include "Term.h"
#include "StringUtils.h"
#include "MiscUtils.h"
#include "InfoStream.h"
#include "TermDocs.h"

namespace Lucene
{
    BufferedDeletes::BufferedDeletes(int32_t messageID)
    {
        this->deletesMap = MapSegmentInfoSegmentDeletes::newInstance();
        this->_bytesUsed = newLucene<AtomicLong>();
        this->_numTerms = newLucene<AtomicLong>();
        this->messageID = messageID;
    }
    
    BufferedDeletes::~BufferedDeletes()
    {
    }
    
    void BufferedDeletes::message(const String& message)
    {
        SyncLock syncLock(this);
        if (infoStream)
        {
            *infoStream << L"BD " << StringUtils::toString(messageID);
            *infoStream << L" [" << DateTools::timeToString(MiscUtils::currentTimeMillis(), DateTools::RESOLUTION_SECOND);
            *infoStream << L"; " << StringUtils::toString(LuceneThread::currentId());
            *infoStream << L"]: BD " << message;
        }
    }
    
    void BufferedDeletes::setInfoStream(InfoStreamPtr infoStream)
    {
        SyncLock syncLock(this);
        this->infoStream = infoStream;
    }
    
    void BufferedDeletes::pushDeletes(SegmentDeletesPtr newDeletes, SegmentInfoPtr info)
    {
        pushDeletes(newDeletes, info, false);
    }
    
    void BufferedDeletes::pushDeletes(SegmentDeletesPtr newDeletes, SegmentInfoPtr info, bool noLimit)
    {
        SyncLock syncLock(this);
        BOOST_ASSERT(newDeletes->any());
        _numTerms->addAndGet(newDeletes->numTermDeletes->get());

        if (!noLimit)
        {
            BOOST_ASSERT(!deletesMap.contains(info));
            BOOST_ASSERT(info);
            deletesMap.put(info, newDeletes);
            _bytesUsed->addAndGet(newDeletes->bytesUsed->get());
        }
        else
        {
            SegmentDeletesPtr deletes(getDeletes(info));
            _bytesUsed->addAndGet(-deletes->bytesUsed->get());
            deletes->update(newDeletes, noLimit);
            _bytesUsed->addAndGet(deletes->bytesUsed->get());
        }    
        if (infoStream)
            message(L"push deletes seg=" + info->toString() + L" dels=" + getDeletes(info)->toString());
        BOOST_ASSERT(checkDeleteStats());
    }
    
    void BufferedDeletes::clear()
    {
        SyncLock syncLock(this);
        deletesMap.clear();
        _numTerms->set(0);
        _bytesUsed->set(0);
    }
    
    bool BufferedDeletes::any()
    {
        SyncLock syncLock(this);
        return (_bytesUsed->get() != 0);
    }
    
    int32_t BufferedDeletes::numTerms()
    {
        return (int32_t)_numTerms->get();
    }
    
    int64_t BufferedDeletes::bytesUsed()
    {
        return _bytesUsed->get();
    }
    
    void BufferedDeletes::commitMerge(OneMergePtr merge)
    {
        SyncLock syncLock(this);
        BOOST_ASSERT(checkDeleteStats());
        if (infoStream)
            message(L"commitMerge merge.info=" + merge->info->toString() + L" merge.segments=" + merge->segments->toString());
        SegmentInfoPtr lastInfo(merge->segments->info(merge->segments->size() - 1));
        SegmentDeletesPtr lastDeletes(deletesMap.get(lastInfo));
        if (lastDeletes)
        {
            deletesMap.remove(lastInfo);
            BOOST_ASSERT(!deletesMap.contains(merge->info));
            deletesMap.put(merge->info, lastDeletes);
            // don't need to update numTerms/bytesUsed since we are just moving the deletes from one info to another
            if (infoStream)
                message(L"commitMerge done: new deletions=" + lastDeletes->toString());
        }
        else if (infoStream)
            message(L"commitMerge done: no new deletions");
        BOOST_ASSERT(!anyDeletes(merge->segments->range(0, merge->segments->size() - 1)));
        BOOST_ASSERT(checkDeleteStats());
    }
    
    void BufferedDeletes::clear(SegmentDeletesPtr deletes)
    {
        SyncLock syncLock(this);
        deletes->clear();
    }
    
    bool BufferedDeletes::applyDeletes(ReaderPoolPtr readerPool, SegmentInfosPtr segmentInfos, SegmentInfosPtr applyInfos)
    {
        SyncLock syncLock(this);
        if (!any())
            return false;
        int64_t t0 = MiscUtils::currentTimeMillis();

        if (infoStream)
            message(L"applyDeletes: applyInfos=" + applyInfos->toString() + L"; index=" + segmentInfos->toString());

        BOOST_ASSERT(checkDeleteStats());
        BOOST_ASSERT(!applyInfos->empty());

        bool any = false;

        SegmentInfoPtr lastApplyInfo(applyInfos->info(applyInfos->size() - 1));
        int32_t lastIdx = segmentInfos->find(lastApplyInfo);

        SegmentInfoPtr firstInfo(applyInfos->info(0));
        int32_t firstIdx = segmentInfos->find(firstInfo);

        // applyInfos must be a slice of segmentInfos
        BOOST_ASSERT(lastIdx - firstIdx + 1 == applyInfos->size());

        // iterate over all segment infos backwards coalescing deletes along the way 
        // when we're at or below the last of the segments to apply to, start applying 
        // the deletes we traverse up to the first apply infos
        SegmentDeletesPtr coalescedDeletes;
        bool hasDeletes = false;
        for (int32_t segIdx = segmentInfos->size() - 1; segIdx >= firstIdx; --segIdx)
        {
            SegmentInfoPtr info(segmentInfos->info(segIdx));
            SegmentDeletesPtr deletes(deletesMap.get(info));
            BOOST_ASSERT(!deletes || deletes->any());

            if (!deletes && !coalescedDeletes)
                continue;
            
            if (infoStream)
            {
                message(L"applyDeletes: seg=" + info->toString() + 
                        L" segment's deletes=[" + (deletes ? deletes->toString() : L"null") + 
                        L"]; coalesced deletes=[" + (coalescedDeletes ? coalescedDeletes->toString() : L"null") + L"]");
            }
            
            if (deletes)
                hasDeletes = true;

            if (segIdx <= lastIdx && hasDeletes)
            {
                int64_t delCountInc = applyDeletes(readerPool, info, coalescedDeletes, deletes);

                if (delCountInc != 0)
                    any = true;

                if (infoStream)
                    message(L"deletes touched " + StringUtils::toString(delCountInc) + L" docIDs");
                
                if (deletes)
                {
                    // we've applied doc ids, and they're only applied on the current segment
                    _bytesUsed->addAndGet(-deletes->docIDs.size() * SegmentDeletes::BYTES_PER_DEL_DOCID);
                    deletes->clearDocIDs();
                }
            }

            // now coalesce at the max limit
            if (deletes)
            {
                if (!coalescedDeletes)
                    coalescedDeletes = newLucene<SegmentDeletes>();
                coalescedDeletes->update(deletes, true);
            }
        }

        // move all deletes to segment just before our merge.
        if (firstIdx > 0)
        {
            SegmentDeletesPtr mergedDeletes;
            int32_t applySize = applyInfos->size();
            for (int32_t i = 0; i < applySize; ++i)
            {
                SegmentInfoPtr info(applyInfos->info(i));
                SegmentDeletesPtr deletes = deletesMap.get(info);
                if (deletes)
                {
                    BOOST_ASSERT(deletes->any());
                    if (!mergedDeletes)
                    {
                        mergedDeletes = getDeletes(segmentInfos->info(firstIdx - 1));
                        _numTerms->addAndGet(-mergedDeletes->numTermDeletes->get());
                        BOOST_ASSERT(_numTerms->get() >= 0);
                        _bytesUsed->addAndGet(-mergedDeletes->bytesUsed->get());
                        BOOST_ASSERT(_bytesUsed->get() >= 0);
                    }

                    mergedDeletes->update(deletes, true);
                }
            }

            if (mergedDeletes)
            {
                _numTerms->addAndGet(mergedDeletes->numTermDeletes->get());
                _bytesUsed->addAndGet(mergedDeletes->bytesUsed->get());
            }

            if (infoStream)
            {
                if (mergedDeletes)
                    message(L"applyDeletes: merge all deletes into seg=" + segmentInfos->info(firstIdx - 1)->toString() + L": " + mergedDeletes->toString());
                else
                    message(L"applyDeletes: no deletes to merge");
            }
        }
        else
        {
            // We drop the deletes in this case, because we've
            // applied them to segment infos starting w/ the first
            // segment.  There are no prior segments so there's no
            // reason to keep them around.  When the applyInfos ==
            // segmentInfos this means all deletes have been
            // removed
        }
        remove(applyInfos);

        BOOST_ASSERT(checkDeleteStats());
        BOOST_ASSERT(applyInfos != segmentInfos || !this->any());

        if (infoStream)
            message(L"applyDeletes took " + StringUtils::toString(MiscUtils::currentTimeMillis() - t0) + L" msec");
        return any;
    }
    
    int64_t BufferedDeletes::applyDeletes(ReaderPoolPtr readerPool, SegmentInfoPtr info, SegmentDeletesPtr coalescedDeletes, SegmentDeletesPtr segmentDeletes)
    {
        SyncLock syncLock(this);
        BOOST_ASSERT(readerPool->infoIsLive(info));

        BOOST_ASSERT(!coalescedDeletes || coalescedDeletes->docIDs.empty());

        int64_t delCount = 0;

        // Lock order: IW -> BD -> RP
        SegmentReaderPtr reader(readerPool->get(info, false));
        LuceneException finally;
        try
        {
            if (coalescedDeletes)
                delCount += applyDeletes(coalescedDeletes, reader);
            if (segmentDeletes)
                delCount += applyDeletes(segmentDeletes, reader);
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        readerPool->release(reader);
        finally.throwException();
        return delCount;
    }
    
    int64_t BufferedDeletes::applyDeletes(SegmentDeletesPtr deletes, SegmentReaderPtr reader)
    {
        SyncLock syncLock(this);
        int64_t delCount = 0;

        BOOST_ASSERT(checkDeleteTerm(TermPtr()));

        if (!deletes->terms.empty())
        {
            TermDocsPtr docs(reader->termDocs());

            for (SortedMapTermInt::iterator entry = deletes->terms.begin(); entry != deletes->terms.end(); ++entry)
            {
                TermPtr term(entry->first);
                // Since we visit terms sorted, we gain performance by re-using the same TermsEnum 
                // and seeking only forwards
                BOOST_ASSERT(checkDeleteTerm(term));
                docs->seek(term);

                int32_t limit = entry->second;
                while (docs->next())
                {
                    int32_t docID = docs->doc();
                    if (docID >= limit)
                        break;
                    ++delCount;
                    reader->deleteDocument(docID);
                }
            }
        }

        // Delete by docID
        for (Collection<int32_t>::iterator docIdInt = deletes->docIDs.begin(); docIdInt != deletes->docIDs.end(); ++docIdInt)
        {
            reader->deleteDocument(*docIdInt);
            ++delCount;
        }

        // Delete by query
        if (!deletes->queries.empty())
        {
            IndexSearcherPtr searcher(newLucene<IndexSearcher>(reader));
            LuceneException finally;
            try
            {
                for (MapQueryInt::iterator entry = deletes->queries.begin(); entry != deletes->queries.end(); ++entry)
                {
                    QueryPtr query(entry->first);
                    int32_t limit = entry->second;
                    WeightPtr weight(query->weight(searcher));
                    ScorerPtr scorer(weight->scorer(reader, true, false));
                    if (scorer)
                    {
                        while (true)
                        {
                            int32_t doc = scorer->nextDoc();
                            if (doc >= limit)
                                break;

                            reader->deleteDocument(doc);
                            ++delCount;
                        }
                    }
                }
            }
            catch (LuceneException& e)
            {
                finally = e;
            }
            searcher->close();
            finally.throwException();
        }

        return delCount;
    }
    
    SegmentDeletesPtr BufferedDeletes::getDeletes(SegmentInfoPtr info)
    {
        SyncLock syncLock(this);
        SegmentDeletesPtr deletes(deletesMap.get(info));
        if (!deletes)
        {
            deletes = newLucene<SegmentDeletes>();
            deletesMap.put(info, deletes);
        }
        return deletes;
    }
    
    void BufferedDeletes::remove(SegmentInfosPtr infos)
    {
        SyncLock syncLock(this);
        BOOST_ASSERT(!infos->empty());
        int32_t infosSize = infos->size();
        for (int32_t i = 0; i < infosSize; ++i)
        {
            SegmentInfoPtr info(infos->info(i));
            SegmentDeletesPtr deletes(deletesMap.get(info));
            if (deletes)
            {
                _bytesUsed->addAndGet(-deletes->bytesUsed->get());
                BOOST_ASSERT(_bytesUsed->get() >= 0);
                _numTerms->addAndGet(-deletes->numTermDeletes->get());
                BOOST_ASSERT(_numTerms->get() >= 0);
                deletesMap.remove(info);
            }
        }
    }
    
    bool BufferedDeletes::anyDeletes(SegmentInfosPtr infos)
    {
        int32_t infosSize = infos->size();
        for (int32_t i = 0; i < infosSize; ++i)
        {
            SegmentInfoPtr info(infos->info(i));
            if (deletesMap.contains(info))
                return true;
        }
        return false;
    }
    
    bool BufferedDeletes::checkDeleteTerm(TermPtr term)
    {
        if (term)
            BOOST_ASSERT(!lastDeleteTerm || term->compareTo(lastDeleteTerm) > 0);
        lastDeleteTerm = term;
        return true;
    }
    
    bool BufferedDeletes::checkDeleteStats()
    {
        int32_t numTerms2 = 0;
        int64_t bytesUsed2 = 0;
        for (MapSegmentInfoSegmentDeletes::iterator deletes = deletesMap.begin(); deletes != deletesMap.end(); ++deletes)
        {
            numTerms2 += deletes->second->numTermDeletes->get();
            bytesUsed2 += deletes->second->bytesUsed->get();
        }
        BOOST_ASSERT(numTerms2 == _numTerms->get());
        BOOST_ASSERT(bytesUsed2 == _bytesUsed->get());
        return true;
    }
}
