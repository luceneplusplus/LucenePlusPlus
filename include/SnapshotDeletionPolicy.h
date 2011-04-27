/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SNAPSHOTDELETIONPOLICY_H
#define SNAPSHOTDELETIONPOLICY_H

#include "IndexDeletionPolicy.h"

namespace Lucene
{
    /// An {@link IndexDeletionPolicy} that wraps around any other {@link IndexDeletionPolicy} and adds the 
    /// ability to hold and later release snapshots of an index. While a snapshot is held, the {@link 
    /// IndexWriter} will not remove any files associated with it even if the index is otherwise being
    /// actively, arbitrarily changed. Because we wrap another arbitrary {@link IndexDeletionPolicy}, this 
    /// gives you the freedom to continue using whatever {@link IndexDeletionPolicy} you would normally want 
    /// to use with your index.
    ///
    /// This class maintains all snapshots in-memory, and so the information is not persisted and not 
    /// protected against system failures. If persistence is important, you can use {@link 
    /// PersistentSnapshotDeletionPolicy} (or your own extension) and when creating a new instance of this 
    /// deletion policy, pass the persistent snapshots information to {@link 
    /// #SnapshotDeletionPolicy(IndexDeletionPolicy, Map)}.
    class LPPAPI SnapshotDeletionPolicy : public IndexDeletionPolicy
    {
    public:
        /// {@link SnapshotDeletionPolicy} wraps another {@link IndexDeletionPolicy} to enable flexible 
        /// snapshotting.
        /// @param primary The {@link IndexDeletionPolicy} that is used on non-snapshotted commits. 
        /// Snapshotted commits, are not deleted until explicitly released via {@link #release(String)}
        /// @param snapshotsInfo A mapping of snapshot ID to the segments filename that is being 
        /// snapshotted. The expected input would be the output of {@link #getSnapshots()}. A null value 
        /// signals that there are no initial snapshots to maintain.
        SnapshotDeletionPolicy(IndexDeletionPolicyPtr primary, MapStringString snapshotsInfo);
        
        virtual ~SnapshotDeletionPolicy();
        
        LUCENE_CLASS(SnapshotDeletionPolicy);
            
    protected:
        MapStringSnapshotInfo idToSnapshot;
        MapStringSetString segmentsFileToIDs;
        IndexDeletionPolicyPtr primary;
        IndexCommitPtr lastCommit;
    
    public:
        /// Get a snapshotted IndexCommit by ID. The IndexCommit can then be used to open an IndexReader on a 
        /// specific commit point, or rollback the index by opening an IndexWriter with the IndexCommit specified 
        /// in its {@link IndexWriterConfig}.
        /// @param id A unique identifier of the commit that was snapshotted.
        /// @return The {@link IndexCommit} for this particular snapshot.
        virtual IndexCommitPtr getSnapshot(const String& id);
        
        /// Get all the snapshots in a map of snapshot IDs to the segments they 'cover.' This can be passed to
        /// {@link #SnapshotDeletionPolicy(IndexDeletionPolicy, Map)} in order to initialize snapshots at 
        /// construction.
        virtual MapStringString getSnapshots();
        
        /// Returns true if the given ID is already used by a snapshot. You can call this method before {@link 
        /// #snapshot(String)} if you are not sure whether the ID is already used or not.
        virtual bool isSnapshotted(const String& id);
        
        virtual void onCommit(Collection<IndexCommitPtr> commits);
        
        virtual void onInit(Collection<IndexCommitPtr> commits);
        
        /// Release the currently held snapshot.
        /// @param id A unique identifier of the commit that is un-snapshotted.
        virtual void release(const String& id);
        
        /// Snapshots the last commit. Once a commit is 'snapshotted,' it is protected from deletion (as long as 
        /// this {@link IndexDeletionPolicy} is used). The commit can be removed by calling {@link 
        /// #release(String)} using the same ID parameter followed by a call to {@link 
        /// IndexWriter#deleteUnusedFiles()}.
        ///
        /// NOTE: ID must be unique in the system. If the same ID is used twice, an {@link IllegalStateException} 
        /// is thrown.
        /// NOTE: while the snapshot is held, the files it references will not be deleted, which will consume 
        /// additional disk space in your index. If you take a snapshot at a particularly bad time (say just 
        /// before you call optimize()) then in the worst case this could consume an extra 1X of your total index 
        /// size, until you release the snapshot.
        /// @param id A unique identifier of the commit that is being snapshotted.
        /// @return The {@link IndexCommit} that was snapshotted.
        virtual IndexCommitPtr snapshot(const String& id);
    
    protected:
        /// Checks if the given id is already used by another snapshot, and throws {@link IllegalStateException} 
        /// if it is.
        void checkSnapshotted(const String& id);
        
        /// Registers the given snapshot information.
        void registerSnapshotInfo(const String& id, const String& segment, IndexCommitPtr commit);
        
        Collection<IndexCommitPtr> wrapCommits(Collection<IndexCommitPtr> commits);
        
        friend class SnapshotCommitPoint;
    };
}

#endif
