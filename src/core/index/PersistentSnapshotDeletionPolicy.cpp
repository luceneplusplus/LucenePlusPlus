/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "PersistentSnapshotDeletionPolicy.h"
#include "IndexWriter.h"
#include "Document.h"
#include "Field.h"
#include "IndexReader.h"
#include "IndexCommit.h"
#include "StringUtils.h"

namespace Lucene
{
    /// Used to validate that the given directory includes just one document with the 
    /// given ID field. Otherwise, it's not a valid Directory for snapshotting.
    const String PersistentSnapshotDeletionPolicy::SNAPSHOTS_ID = L"$SNAPSHOTS_DOC$";
    
    PersistentSnapshotDeletionPolicy::PersistentSnapshotDeletionPolicy(IndexDeletionPolicyPtr primary, DirectoryPtr dir, 
                                                                       IndexWriterConfig::OpenMode mode, 
                                                                       LuceneVersion::Version matchVersion)
        : SnapshotDeletionPolicy(primary, MapStringString())
    {
        // Initialize the index writer over the snapshot directory.
        writer = newLucene<IndexWriter>(dir, newLucene<IndexWriterConfig>(matchVersion, AnalyzerPtr())->setOpenMode(mode));
        if (mode != IndexWriterConfig::APPEND)
        {
            // IndexWriter no longer creates a first commit on an empty Directory. So if we were 
            // asked to CREATE*, call commit() just to be sure. If the index contains information 
            // and mode is CREATE_OR_APPEND, it's a no-op.
            writer->commit();
        }

        // Initializes the snapshots information. This code should basically run only if mode != CREATE, 
        // but if it is, it's no harm as we only open the reader once and immediately close it.
        MapStringString snapshots(readSnapshotsInfo(dir));
        for (MapStringString::iterator e = snapshots.begin(); e != snapshots.end(); ++e)
            registerSnapshotInfo(e->first, e->second, IndexCommitPtr());
    }
    
    PersistentSnapshotDeletionPolicy::~PersistentSnapshotDeletionPolicy()
    {
    }
    
    MapStringString PersistentSnapshotDeletionPolicy::readSnapshotsInfo(DirectoryPtr dir)
    {
        IndexReaderPtr r(IndexReader::open(dir, true));
        MapStringString snapshots(MapStringString::newInstance());
        LuceneException finally;
        try
        {
            int32_t numDocs = r->numDocs();
            // index is allowed to have exactly one document or 0.
            if (numDocs == 1)
            {
                DocumentPtr doc(r->document(r->maxDoc() - 1));
                FieldPtr sid(doc->getField(SNAPSHOTS_ID));
                if (!sid)
                    boost::throw_exception(IllegalStateException(L"directory is not a valid snapshots store!"));
                doc->removeField(SNAPSHOTS_ID);
                Collection<FieldablePtr> fields(doc->getFields());
                for (Collection<FieldablePtr>::iterator f = fields.begin(); f != fields.end(); ++f)
                    snapshots.put((*f)->name(), (*f)->stringValue());
            }
            else if (numDocs != 0)
                boost::throw_exception(IllegalStateException(L"should be at most 1 document in the snapshots directory: " + StringUtils::toString(numDocs)));
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        r->close();
        finally.throwException();
        return snapshots;
    }
    
    void PersistentSnapshotDeletionPolicy::onInit(Collection<IndexCommitPtr> commits)
    {
        SyncLock syncLock(this);
        // SnapshotDeletionPolicy::onInit() needs to be called first to ensure that initialization
        // behaves as expected. The superclass, SnapshotDeletionPolicy, ensures that any snapshot 
        // IDs with empty IndexCommits are released. Since this  happens, this class needs to 
        // persist these changes.
        SnapshotDeletionPolicy::onInit(commits);
        persistSnapshotInfos(L"", L"");
    }
    
    IndexCommitPtr PersistentSnapshotDeletionPolicy::snapshot(const String& id)
    {
        SyncLock syncLock(this);
        checkSnapshotted(id);
        if (SNAPSHOTS_ID == id)
            boost::throw_exception(IllegalArgumentException(id + L" is reserved and cannot be used as a snapshot id"));
        persistSnapshotInfos(id, lastCommit->getSegmentsFileName());
        return SnapshotDeletionPolicy::snapshot(id);
    }
    
    void PersistentSnapshotDeletionPolicy::release(const String& id)
    {
        SyncLock syncLock(this);
        SnapshotDeletionPolicy::release(id);
        persistSnapshotInfos(L"", L"");
    }
    
    void PersistentSnapshotDeletionPolicy::close()
    {
        writer->close();
    }
    
    void PersistentSnapshotDeletionPolicy::persistSnapshotInfos(const String& id, const String& segment)
    {
        writer->deleteAll();
        DocumentPtr d(newLucene<Document>());
        d->add(newLucene<Field>(SNAPSHOTS_ID, L"", Field::STORE_YES, Field::INDEX_NO));
        MapStringString snapshots(SnapshotDeletionPolicy::getSnapshots());
        for (MapStringString::iterator e = snapshots.begin(); e != snapshots.end(); ++e)
            d->add(newLucene<Field>(e->first, e->second, Field::STORE_YES, Field::INDEX_NO));
        if (!id.empty())
            d->add(newLucene<Field>(id, segment, Field::STORE_YES, Field::INDEX_NO));
        writer->addDocument(d);
        writer->commit();
    }
}
