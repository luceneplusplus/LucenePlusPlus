/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "MergePolicy.h"
#include "SegmentInfos.h"
#include "SegmentInfo.h"
#include "StringUtils.h"

namespace Lucene
{
    MergePolicy::MergePolicy()
    {
    }
    
    MergePolicy::~MergePolicy()
    {
    }
    
    void MergePolicy::setIndexWriter(IndexWriterPtr writer)
    {
        if (!_writer.expired())
            boost::throw_exception(RuntimeException(L"The object cannot be set twice"));
        _writer = writer;
    }
    
    OneMerge::OneMerge(SegmentInfosPtr segments)
    {
        optimize = false;
        registerDone = false;
        mergeGen = 0;
        isExternal = false;
        maxNumSegmentsOptimize = 0;
        aborted = false;
        paused = false;
        
        if (segments->empty())
            boost::throw_exception(RuntimeException(L"segments must include at least one segment"));
        this->segments = segments;
    }
    
    OneMerge::~OneMerge()
    {
    }
    
    void OneMerge::setException(const LuceneException& error)
    {
        SyncLock syncLock(this);
        this->error = error;
    }
        
    LuceneException OneMerge::getException()
    {
        SyncLock syncLock(this);
        return error;
    }
    
    void OneMerge::abort()
    {
        SyncLock syncLock(this);
        aborted = true;
        notifyAll();
    }
    
    bool OneMerge::isAborted()
    {
        SyncLock syncLock(this);
        return aborted;
    }
    
    void OneMerge::checkAborted(DirectoryPtr dir)
    {
        SyncLock syncLock(this);
        if (aborted)
            boost::throw_exception(MergeAbortedException(L"merge is aborted: " + segString(dir)));
        
        while (paused)
        {
            // In theory we could wait() indefinitely, but we do 1000 msec, defensively
            wait(1000);
            if (aborted)
                boost::throw_exception(MergeAbortedException(L"merge is aborted: " + segString(dir)));
        }
    }
    
    void OneMerge::setPause(bool paused)
    {
        SyncLock syncLock(this);
        this->paused = paused;
        if (!paused)
        {
            // Wakeup merge thread, if it's waiting
            notifyAll();
        }
    }
    
    bool OneMerge::getPause()
    {
        SyncLock syncLock(this);
        return paused;
    }
    
    String OneMerge::segString(DirectoryPtr dir)
    {
        StringStream buffer;
        int32_t numSegments = segments->size();
        for (int32_t i = 0; i < numSegments; ++i)
        {
            if (i > 0)
                buffer << L" ";
            buffer << segments->info(i)->toString(dir, 0);
        }
        if (info)
            buffer << L" into " + info->name;
        if (optimize)
            buffer << L" [optimize]";
        if (aborted)
            buffer << L" [ABORTED]";
        return buffer.str();
    }
    
    int64_t OneMerge::totalBytesSize()
    {
        int64_t total = 0;
        int32_t numSegments = segments->size();
        for (int32_t i = 0; i < numSegments; ++i)
            total += segments->info(i)->sizeInBytes(true);
        return total;
    }
    
    int32_t OneMerge::totalNumDocs()
    {
        int32_t total = 0;
        int32_t numSegments = segments->size();
        for (int32_t i = 0; i < numSegments; ++i)
            total += segments->info(i)->docCount;
        return total;
    }

    MergeSpecification::MergeSpecification()
    {
        merges = Collection<OneMergePtr>::newInstance();
    }
    
    MergeSpecification::~MergeSpecification()
    {
    }
    
    void MergeSpecification::add(OneMergePtr merge)
    {
        merges.add(merge);
    }
    
    String MergeSpecification::segString(DirectoryPtr dir)
    {
        String seg(L"MergeSpec:\n");
        int32_t i = 1;
        for (Collection<OneMergePtr>::iterator merge = merges.begin(); merge != merges.end(); ++merge)
            seg += L"  " + StringUtils::toString(i++) + L": " + (*merge)->segString(dir);
        return seg;
    }
}
