/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "SnapshotDeletionPolicy.h"
#include "_SnapshotDeletionPolicy.h"

namespace Lucene
{
    SnapshotDeletionPolicy::SnapshotDeletionPolicy(IndexDeletionPolicyPtr primary, MapStringString snapshotsInfo)
    {
        this->idToSnapshot = MapStringSnapshotInfo::newInstance();
        this->segmentsFileToIDs = MapStringSetString::newInstance();
        this->primary = primary;
        
        if (snapshotsInfo)
        {
            // Add the ID->segmentIDs here - the actual IndexCommits will be reconciled on the call to onInit()
            for (MapStringString::iterator entry = snapshotsInfo.begin(); entry != snapshotsInfo.end(); ++entry)
                registerSnapshotInfo(entry->first, entry->second, IndexCommitPtr());
        }
    }
    
    SnapshotDeletionPolicy::~SnapshotDeletionPolicy()
    {
    }
    
    void SnapshotDeletionPolicy::checkSnapshotted(const String& id)
    {
        if (isSnapshotted(id))
            boost::throw_exception(IllegalStateException(L"Snapshot ID " + id + L" is already used - must be unique"));
    }
    
    void SnapshotDeletionPolicy::registerSnapshotInfo(const String& id, const String& segment, IndexCommitPtr commit)
    {
        idToSnapshot.put(id, newLucene<SnapshotInfo>(id, segment, commit));
        HashSet<String> ids(segmentsFileToIDs.get(segment));
        if (!ids)
        {
            ids = HashSet<String>::newInstance();
            segmentsFileToIDs.put(segment, ids);
        }
        ids.add(id);
    }
    
    Collection<IndexCommitPtr> SnapshotDeletionPolicy::wrapCommits(Collection<IndexCommitPtr> commits)
    {
        Collection<IndexCommitPtr> wrappedCommits(Collection<IndexCommitPtr>::newInstance());
        for (Collection<IndexCommitPtr>::iterator commit = commits.begin(); commit != commits.end(); ++commit)
            wrappedCommits.add(newLucene<SnapshotCommitPoint>(shared_from_this(), *commit));
        return wrappedCommits;
    }
    
    IndexCommitPtr SnapshotDeletionPolicy::getSnapshot(const String& id)
    {
        SyncLock syncLock(this);
        SnapshotInfoPtr snapshotInfo(idToSnapshot.get(id));
        if (!snapshotInfo)
            boost::throw_exception(IllegalStateException(L"No snapshot exists by ID: " + id));
        return snapshotInfo->commit;
    }
    
    MapStringString SnapshotDeletionPolicy::getSnapshots()
    {
        SyncLock syncLock(this);
        MapStringString snapshots(MapStringString::newInstance());
        for (MapStringSnapshotInfo::iterator entry = idToSnapshot.begin(); entry != idToSnapshot.end(); ++entry)
            snapshots.put(entry->first, entry->second->segmentsFileName);
        return snapshots;
    }
    
    bool SnapshotDeletionPolicy::isSnapshotted(const String& id)
    {
        return idToSnapshot.contains(id);
    }
    
    void SnapshotDeletionPolicy::onCommit(Collection<IndexCommitPtr> commits)
    {
        SyncLock syncLock(this);
        primary->onCommit(wrapCommits(commits));
        lastCommit = commits[commits.size() - 1];
    }
    
    void SnapshotDeletionPolicy::onInit(Collection<IndexCommitPtr> commits)
    {
        SyncLock syncLock(this);
        primary->onInit(wrapCommits(commits));
        lastCommit = commits[commits.size() - 1];
        
        // Assign snapshotted IndexCommits to their correct snapshot IDs as specified in the constructor.
        for (Collection<IndexCommitPtr>::iterator commit = commits.begin(); commit != commits.end(); ++commit)
        {
            HashSet<String> ids(segmentsFileToIDs.get((*commit)->getSegmentsFileName()));
            if (ids)
            {
                for (HashSet<String>::iterator id = ids.begin(); id != ids.end(); ++id)
                    idToSnapshot.get(*id)->commit = *commit;
            }
        }
        
        // Second, see if there are any instances where a snapshot ID was specified in the constructor but 
        // an IndexCommit doesn't exist. In this case, the ID should be removed.
        // Note: This code is protective for extreme cases where IDs point to non-existent segments. As the 
        // constructor should have received its information via a call to getSnapshots(), the data should 
        // be well-formed.
        Collection<String> idsToRemove;
        for (MapStringSnapshotInfo::iterator entry = idToSnapshot.begin(); entry != idToSnapshot.end(); ++entry)
        {
            if (!entry->second->commit)
            {
                if (!idsToRemove)
                    idsToRemove = Collection<String>::newInstance();
                idsToRemove.add(entry->first);
            }
        }
        
        // Finally, remove those 'lost' snapshots.
        if (idsToRemove)
        {
            for (Collection<String>::iterator id = idsToRemove.begin(); id != idsToRemove.end(); ++id)
            {
                SnapshotInfoPtr info(idToSnapshot.get(*id));
                idToSnapshot.remove(*id);
                segmentsFileToIDs.remove(info->segmentsFileName);
            }
        }
    }
    
    void SnapshotDeletionPolicy::release(const String& id)
    {
        SyncLock syncLock(this);
        SnapshotInfoPtr info(idToSnapshot.get(id));
        if (!info)
            boost::throw_exception(IllegalStateException(L"Snapshot doesn't exist: " + id));
        idToSnapshot.remove(id);
        HashSet<String> ids(segmentsFileToIDs.get(info->segmentsFileName));
        if (ids)
        {
            ids.remove(id);
            if (ids.empty())
                segmentsFileToIDs.remove(info->segmentsFileName);
        }
    }
    
    IndexCommitPtr SnapshotDeletionPolicy::snapshot(const String& id)
    {
        SyncLock syncLock(this);
        if (!lastCommit)
        {
            // no commit exists. Really shouldn't happen, but might be if SDP is accessed before onInit or onCommit were called.
            boost::throw_exception(IllegalStateException(L"no index commit to snapshot"));
        }
        // Can't use the same snapshot ID twice
        checkSnapshotted(id);

        registerSnapshotInfo(id, lastCommit->getSegmentsFileName(), lastCommit);
        return lastCommit;
    }
    
    SnapshotInfo::SnapshotInfo(const String& id, const String& segmentsFileName, IndexCommitPtr commit)
    {
        this->id = id;
        this->segmentsFileName = segmentsFileName;
        this->commit = commit;
    }
    
    SnapshotInfo::~SnapshotInfo()
    {
    }
    
    String SnapshotInfo::toString()
    {
        return id + L" : " + segmentsFileName;
    }
    
    SnapshotCommitPoint::SnapshotCommitPoint(SnapshotDeletionPolicyPtr deletionPolicy, IndexCommitPtr cp)
    {
        this->_deletionPolicy = deletionPolicy;
        this->cp = cp;
    }
    
    SnapshotCommitPoint::~SnapshotCommitPoint()
    {
    }
    
    String SnapshotCommitPoint::toString()
    {
        return L"SnapshotDeletionPolicy.SnapshotCommitPoint(" + cp->toString() + L")";
    }
    
    bool SnapshotCommitPoint::shouldDelete(const String& segmentsFileName)
    {
        SnapshotDeletionPolicyPtr deletionPolicy(_deletionPolicy);
        return !deletionPolicy->segmentsFileToIDs.contains(segmentsFileName);
    }
    
    void SnapshotCommitPoint::_delete()
    {
        SnapshotDeletionPolicyPtr deletionPolicy(_deletionPolicy);
        SyncLock policyLock(deletionPolicy);
        // Suppress the delete request if this commit point is our current snapshot.
        if (shouldDelete(getSegmentsFileName()))
            cp->_delete();
    }
    
    DirectoryPtr SnapshotCommitPoint::getDirectory()
    {
        return cp->getDirectory();
    }
    
    HashSet<String> SnapshotCommitPoint::getFileNames()
    {
        return cp->getFileNames();
    }
    
    int64_t SnapshotCommitPoint::getGeneration()
    {
        return cp->getGeneration();
    }
    
    String SnapshotCommitPoint::getSegmentsFileName()
    {
        return cp->getSegmentsFileName();
    }
    
    MapStringString SnapshotCommitPoint::getUserData()
    {
        return cp->getUserData();
    }
    
    int64_t SnapshotCommitPoint::getVersion()
    {
        return cp->getVersion();
    }
    
    bool SnapshotCommitPoint::isDeleted()
    {
        return cp->isDeleted();
    }
    
    bool SnapshotCommitPoint::isOptimized()
    {
        return cp->isOptimized();
    }
}
