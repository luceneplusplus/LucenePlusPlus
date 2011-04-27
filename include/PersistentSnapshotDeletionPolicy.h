/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef PERSISTENTSNAPSHOTDELETIONPOLICY_H
#define PERSISTENTSNAPSHOTDELETIONPOLICY_H

#include "SnapshotDeletionPolicy.h"
#include "IndexWriterConfig.h"

namespace Lucene
{
    /// A {@link SnapshotDeletionPolicy} which adds a persistence layer so that snapshots can 
    /// be maintained across the life of an application. The snapshots are persisted in a 
    /// {@link Directory} and are committed as soon as {@link #snapshot(String)} or {@link 
    /// #release(String)} is called.
    ///
    /// NOTE: this class receives a {@link Directory} to persist the data into a Lucene index. 
    /// It is highly recommended to use a dedicated directory (and on stable storage as well) 
    /// for persisting the snapshots' information, and not reuse the content index directory, 
    /// or otherwise conflicts and index corruptions will occur.
    ///
    /// NOTE: you should call {@link #close()} when you're done using this class for safety 
    /// (it will close the {@link IndexWriter} instance used).
    class LPPAPI PersistentSnapshotDeletionPolicy : public SnapshotDeletionPolicy
    {
    public:
        /// {@link PersistentSnapshotDeletionPolicy} wraps another {@link IndexDeletionPolicy} 
        /// to enable flexible snapshotting.
        /// @param primary the {@link IndexDeletionPolicy} that is used on non-snapshotted 
        /// commits. Snapshotted commits, by definition, are not deleted until explicitly 
        /// released via {@link #release(String)}.
        /// @param dir the {@link Directory} which will be used to persist the snapshots 
        /// information.
        /// @param mode specifies whether a new index should be created, deleting all existing 
        /// snapshots information (immediately), or open an existing index, initializing the 
        /// class with the snapshots information.
        /// @param matchVersion specifies the {@link Version} that should be used when opening 
        /// the IndexWriter.
        PersistentSnapshotDeletionPolicy(IndexDeletionPolicyPtr primary, DirectoryPtr dir, 
                                         IndexWriterConfig::OpenMode mode, LuceneVersion::Version matchVersion);
        virtual ~PersistentSnapshotDeletionPolicy();
        
        LUCENE_CLASS(PersistentSnapshotDeletionPolicy);
    
    private:
        /// Used to validate that the given directory includes just one document with the 
        /// given ID field. Otherwise, it's not a valid Directory for snapshotting.
        static const String SNAPSHOTS_ID;
        
        /// The index writer which maintains the snapshots metadata
        IndexWriterPtr writer;

    public:
        /// Reads the snapshots information from the given {@link Directory}. This method does can 
        /// be used if the snapshots information is needed, however you cannot instantiate the 
        /// deletion policy (because eg., some other process keeps a lock on the snapshots directory).
        static MapStringString readSnapshotsInfo(DirectoryPtr dir);
        
        virtual void onInit(Collection<IndexCommitPtr> commits);
        
        /// Snapshots the last commit using the given ID. Once this method returns, the snapshot 
        /// information is persisted in the directory.
        /// @see SnapshotDeletionPolicy#snapshot(String)
        virtual IndexCommitPtr snapshot(const String& id);
        
        /// Deletes a snapshotted commit by ID. Once this method returns, the snapshot information 
        /// is committed to the directory.
        /// @see SnapshotDeletionPolicy#release(String)
        virtual void release(const String& id);
        
        /// Closes the index which writes the snapshots to the directory.
        virtual void close();
    
    private:
        void persistSnapshotInfos(const String& id, const String& segment);
    };
}

#endif
