/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _SNAPSHOTDELETIONPOLICY_H
#define _SNAPSHOTDELETIONPOLICY_H

#include "IndexCommit.h"

namespace Lucene
{
    /// Holds a Snapshot's information.
    class SnapshotInfo : public LuceneObject
    {
    public:
        SnapshotInfo(const String& id, const String& segmentsFileName, IndexCommitPtr commit);
        virtual ~SnapshotInfo();
        
        LUCENE_CLASS(SnapshotInfo);

    public:
        String id;
        String segmentsFileName;
        IndexCommitPtr commit;
    
    public:
        virtual String toString();
    };

    class SnapshotCommitPoint : public IndexCommit
    {
    public:
        SnapshotCommitPoint(SnapshotDeletionPolicyPtr deletionPolicy, IndexCommitPtr cp);
        virtual ~SnapshotCommitPoint();
        
        LUCENE_CLASS(SnapshotCommitPoint);
            
    protected:
        SnapshotDeletionPolicyWeakPtr _deletionPolicy;
    
    public:
        IndexCommitPtr cp;
    
    public:
        virtual String toString();
        
        /// Delete this commit point.
        virtual void _delete();
        
        /// Returns the {@link Directory} for the index.
        virtual DirectoryPtr getDirectory();
        
        /// Returns all index files referenced by this commit point.
        virtual HashSet<String> getFileNames();
        
        /// Returns the generation (the _N in segments_N) for this IndexCommit.
        virtual int64_t getGeneration();
        
        /// Get the segments file (segments_N) associated with this commit point.
        virtual String getSegmentsFileName();
        
        /// Returns userData, previously passed to {@link IndexWriter#commit(Map)} for this commit.
        virtual MapStringString getUserData();
        
        /// Returns the version for this IndexCommit.
        virtual int64_t getVersion();
        
        virtual bool isDeleted();
        
        virtual bool isOptimized();

    protected:
        /// Returns true if this segment can be deleted. The default implementation returns false 
        /// if this segment is currently held as snapshot.
        bool shouldDelete(const String& segmentsFileName);
    };
}

#endif
