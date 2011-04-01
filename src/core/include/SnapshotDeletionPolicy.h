/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SNAPSHOTDELETIONPOLICY_H
#define SNAPSHOTDELETIONPOLICY_H

#include "IndexDeletionPolicy.h"
#include "IndexCommit.h"

namespace Lucene
{
    class LPPAPI SnapshotDeletionPolicy : public IndexDeletionPolicy
    {
    public:
        SnapshotDeletionPolicy(IndexDeletionPolicyPtr primary);
        virtual ~SnapshotDeletionPolicy();
        
        LUCENE_CLASS(SnapshotDeletionPolicy);
            
    protected:
        IndexCommitPtr lastCommit;
        IndexDeletionPolicyPtr primary;
        String _snapshot;
    
    public:
        /// This is called once when a writer is first instantiated to give the policy a chance to remove old
        /// commit points.
        virtual void onInit(Collection<IndexCommitPtr> commits);
        
        /// This is called each time the writer completed a commit.  This gives the policy a chance to remove 
        /// old commit points with each commit.
        virtual void onCommit(Collection<IndexCommitPtr> commits);
        
        /// Take a snapshot of the most recent commit to the index.  You must call release() to free this snapshot.
        /// Note that while the snapshot is held, the files it references will not be deleted, which will consume
        /// additional disk space in your index.  If you take a snapshot at a particularly bad time (say just before
        /// you call optimize()) then in the worst case this could consume an extra 1X of your total index size, 
        /// until you release the snapshot.
        virtual IndexCommitPtr snapshot();
        
        /// Release the currently held snapshot.
        virtual void release();
    
    protected:
        Collection<IndexCommitPtr> wrapCommits(Collection<IndexCommitPtr> commits);
        
        friend class MyCommitPoint;
    };
    
    class LPPAPI MyCommitPoint : public IndexCommit
    {
    public:
        MyCommitPoint(SnapshotDeletionPolicyPtr deletionPolicy, IndexCommitPtr cp);
        virtual ~MyCommitPoint();
        
        LUCENE_CLASS(MyCommitPoint);
            
    protected:
        SnapshotDeletionPolicyWeakPtr _deletionPolicy;
    
    public:
        IndexCommitPtr cp;
    
    public:
        virtual String toString();
        
        /// Get the segments file (segments_N) associated with this commit point.
        virtual String getSegmentsFileName();
        
        /// Returns all index files referenced by this commit point.
        virtual HashSet<String> getFileNames();
        
        /// Returns the {@link Directory} for the index.
        virtual DirectoryPtr getDirectory();
        
        /// Delete this commit point.
        virtual void deleteCommit();
        
        virtual bool isDeleted();
        
        /// Returns the version for this IndexCommit.
        virtual int64_t getVersion();
        
        /// Returns the generation (the _N in segments_N) for this IndexCommit.
        virtual int64_t getGeneration();
        
        /// Returns userData, previously passed to {@link IndexWriter#commit(Map)} for this commit.
        virtual MapStringString getUserData();
        
        virtual bool isOptimized();
    };
}

#endif
