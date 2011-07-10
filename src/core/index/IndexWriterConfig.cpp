/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "IndexWriterConfig.h"
#include "IndexWriter.h"
#include "IndexReader.h"
#include "IndexCommit.h"
#include "KeepOnlyLastCommitDeletionPolicy.h"
#include "Similarity.h"
#include "ConcurrentMergeScheduler.h"
#include "DocumentsWriter.h"
#include "LogByteSizeMergePolicy.h"
#include "StringUtils.h"
#include "Analyzer.h"

namespace Lucene
{
    /// Default value is 128.
    const int32_t IndexWriterConfig::DEFAULT_TERM_INDEX_INTERVAL = 128;

    /// Denotes a flush trigger is disabled.
    const int32_t IndexWriterConfig::DISABLE_AUTO_FLUSH = -1;

    /// Disabled by default (because IndexWriterConfig flushes by RAM usage by default).
    const int32_t IndexWriterConfig::DEFAULT_MAX_BUFFERED_DELETE_TERMS = IndexWriterConfig::DISABLE_AUTO_FLUSH;

    /// Disabled by default (because IndexWriterConfig flushes by RAM usage by default).
    const int32_t IndexWriterConfig::DEFAULT_MAX_BUFFERED_DOCS = IndexWriterConfig::DISABLE_AUTO_FLUSH;

    /// Default value is 16 MB (which means flush when buffered docs consume 16 MB RAM).
    const double IndexWriterConfig::DEFAULT_RAM_BUFFER_SIZE_MB = 16.0;

    /// Default value for the write lock timeout (1000 ms).
    int64_t IndexWriterConfig::WRITE_LOCK_TIMEOUT = 1000;

    /// The maximum number of simultaneous threads that may be indexing documents at once in IndexWriter; if more
    /// than this many threads arrive they will wait for others to finish.
    const int32_t IndexWriterConfig::DEFAULT_MAX_THREAD_STATES = 8;

    /// Default setting for {@link #setReaderPooling}.
    const bool IndexWriterConfig::DEFAULT_READER_POOLING = false;

    /// Disabled by default (because IndexWriterConfig flushes by RAM usage by default).
    const int32_t IndexWriterConfig::DEFAULT_READER_TERMS_INDEX_DIVISOR = IndexReader::DEFAULT_TERMS_INDEX_DIVISOR;

    IndexWriterConfig::IndexWriterConfig(LuceneVersion::Version matchVersion, AnalyzerPtr analyzer)
    {
        this->matchVersion = matchVersion;
        this->analyzer = analyzer;
        delPolicy = newLucene<KeepOnlyLastCommitDeletionPolicy>();
        openMode = CREATE_OR_APPEND;
        similarity = Similarity::getDefault();
        termIndexInterval = DEFAULT_TERM_INDEX_INTERVAL;
        mergeScheduler = newLucene<ConcurrentMergeScheduler>();
        writeLockTimeout = WRITE_LOCK_TIMEOUT;
        maxBufferedDeleteTerms = DEFAULT_MAX_BUFFERED_DELETE_TERMS;
        ramBufferSizeMB = DEFAULT_RAM_BUFFER_SIZE_MB;
        maxBufferedDocs = DEFAULT_MAX_BUFFERED_DOCS;
        indexingChain = DocumentsWriter::defaultIndexingChain();
        mergePolicy = newLucene<LogByteSizeMergePolicy>();
        maxThreadStates = DEFAULT_MAX_THREAD_STATES;
        readerPooling = DEFAULT_READER_POOLING;
        readerTermsIndexDivisor = DEFAULT_READER_TERMS_INDEX_DIVISOR;
    }

    IndexWriterConfig::~IndexWriterConfig()
    {
    }

    void IndexWriterConfig::setDefaultWriteLockTimeout(int64_t writeLockTimeout)
    {
        WRITE_LOCK_TIMEOUT = writeLockTimeout;
    }

    int64_t IndexWriterConfig::getDefaultWriteLockTimeout()
    {
        return WRITE_LOCK_TIMEOUT;
    }

    LuceneObjectPtr IndexWriterConfig::clone(LuceneObjectPtr other)
    {
        // Shallow clone is the only thing that's possible.
        LuceneObjectPtr clone = other ? other : newLucene<IndexWriterConfig>(matchVersion, analyzer);
        IndexWriterConfigPtr cloneConfig(boost::static_pointer_cast<IndexWriterConfig>(LuceneObject::clone(clone)));
        if (!cloneConfig)
            boost::throw_exception(RuntimeException(L"This reader does not implement clone()."));
        cloneConfig->analyzer = analyzer;
        cloneConfig->delPolicy = delPolicy;
        cloneConfig->commit = commit;
        cloneConfig->openMode = openMode;
        cloneConfig->similarity = similarity;
        cloneConfig->termIndexInterval = termIndexInterval;
        cloneConfig->mergeScheduler = mergeScheduler;
        cloneConfig->writeLockTimeout = writeLockTimeout;
        cloneConfig->maxBufferedDeleteTerms = maxBufferedDeleteTerms;
        cloneConfig->ramBufferSizeMB = ramBufferSizeMB;
        cloneConfig->maxBufferedDocs = maxBufferedDocs;
        cloneConfig->indexingChain = indexingChain;
        cloneConfig->mergedSegmentWarmer = mergedSegmentWarmer;
        cloneConfig->mergePolicy = mergePolicy;
        cloneConfig->maxThreadStates = maxThreadStates;
        cloneConfig->readerPooling = readerPooling;
        cloneConfig->readerTermsIndexDivisor = readerTermsIndexDivisor;
        cloneConfig->matchVersion = matchVersion;
        return other;
    }

    AnalyzerPtr IndexWriterConfig::getAnalyzer()
    {
        return analyzer;
    }

    IndexWriterConfigPtr IndexWriterConfig::setOpenMode(IndexWriterConfig::OpenMode openMode)
    {
        this->openMode = openMode;
        return shared_from_this();
    }

    IndexWriterConfig::OpenMode IndexWriterConfig::getOpenMode()
    {
        return openMode;
    }

    IndexWriterConfigPtr IndexWriterConfig::setIndexDeletionPolicy(IndexDeletionPolicyPtr delPolicy)
    {
        this->delPolicy = delPolicy ? delPolicy : newLucene<KeepOnlyLastCommitDeletionPolicy>();
        return shared_from_this();
    }

    IndexDeletionPolicyPtr IndexWriterConfig::getIndexDeletionPolicy()
    {
        return delPolicy;
    }

    IndexWriterConfigPtr IndexWriterConfig::setIndexCommit(IndexCommitPtr commit)
    {
        this->commit = commit;
        return shared_from_this();
    }

    IndexCommitPtr IndexWriterConfig::getIndexCommit()
    {
        return commit;
    }

    IndexWriterConfigPtr IndexWriterConfig::setSimilarity(SimilarityPtr similarity)
    {
        this->similarity = similarity ? similarity : Similarity::getDefault();
        return shared_from_this();
    }

    SimilarityPtr IndexWriterConfig::getSimilarity()
    {
        return similarity;
    }

    IndexWriterConfigPtr IndexWriterConfig::setTermIndexInterval(int32_t interval)
    {
        this->termIndexInterval = interval;
        return shared_from_this();
    }

    int32_t IndexWriterConfig::getTermIndexInterval()
    {
        return termIndexInterval;
    }

    IndexWriterConfigPtr IndexWriterConfig::setMergeScheduler(MergeSchedulerPtr mergeScheduler)
    {
        this->mergeScheduler = mergeScheduler ? mergeScheduler : newLucene<ConcurrentMergeScheduler>();
        return shared_from_this();
    }

    MergeSchedulerPtr IndexWriterConfig::getMergeScheduler()
    {
        return mergeScheduler;
    }

    IndexWriterConfigPtr IndexWriterConfig::setWriteLockTimeout(int64_t writeLockTimeout)
    {
        this->writeLockTimeout = writeLockTimeout;
        return shared_from_this();
    }

    int64_t IndexWriterConfig::getWriteLockTimeout()
    {
        return writeLockTimeout;
    }

    IndexWriterConfigPtr IndexWriterConfig::setMaxBufferedDeleteTerms(int32_t maxBufferedDeleteTerms)
    {
        if (maxBufferedDeleteTerms != DISABLE_AUTO_FLUSH && maxBufferedDeleteTerms < 1)
            boost::throw_exception(IllegalArgumentException(L"maxBufferedDeleteTerms must at least be 1 when enabled"));
        this->maxBufferedDeleteTerms = maxBufferedDeleteTerms;
        return shared_from_this();
    }

    int32_t IndexWriterConfig::getMaxBufferedDeleteTerms()
    {
        return maxBufferedDeleteTerms;
    }

    IndexWriterConfigPtr IndexWriterConfig::setRAMBufferSizeMB(double ramBufferSizeMB)
    {
        if (ramBufferSizeMB > 2048.0)
        {
            boost::throw_exception(IllegalArgumentException(L"ramBufferSize " + StringUtils::toString(ramBufferSizeMB) +
                                   L" is too large; should be comfortably less than 2048"));
        }
        if (ramBufferSizeMB != DISABLE_AUTO_FLUSH && ramBufferSizeMB <= 0.0)
            boost::throw_exception(IllegalArgumentException(L"ramBufferSize should be > 0.0 MB when enabled"));
        if (ramBufferSizeMB == DISABLE_AUTO_FLUSH && maxBufferedDocs == DISABLE_AUTO_FLUSH)
            boost::throw_exception(IllegalArgumentException(L"at least one of ramBufferSize and maxBufferedDocs must be enabled"));
        this->ramBufferSizeMB = ramBufferSizeMB;
        return shared_from_this();
    }

    double IndexWriterConfig::getRAMBufferSizeMB()
    {
        return ramBufferSizeMB;
    }

    IndexWriterConfigPtr IndexWriterConfig::setMaxBufferedDocs(int32_t maxBufferedDocs)
    {
        if (maxBufferedDocs != DISABLE_AUTO_FLUSH && maxBufferedDocs < 2)
            boost::throw_exception(IllegalArgumentException(L"maxBufferedDocs must at least be 2 when enabled"));
        if (maxBufferedDocs == DISABLE_AUTO_FLUSH && ramBufferSizeMB == DISABLE_AUTO_FLUSH)
            boost::throw_exception(IllegalArgumentException(L"at least one of ramBufferSize and maxBufferedDocs must be enabled"));
        this->maxBufferedDocs = maxBufferedDocs;
        return shared_from_this();
    }

    int32_t IndexWriterConfig::getMaxBufferedDocs()
    {
        return maxBufferedDocs;
    }

    IndexWriterConfigPtr IndexWriterConfig::setMergedSegmentWarmer(IndexReaderWarmerPtr mergeSegmentWarmer)
    {
        this->mergedSegmentWarmer = mergeSegmentWarmer;
        return shared_from_this();
    }

    IndexReaderWarmerPtr IndexWriterConfig::getMergedSegmentWarmer()
    {
        return mergedSegmentWarmer;
    }

    IndexWriterConfigPtr IndexWriterConfig::setMergePolicy(MergePolicyPtr mergePolicy)
    {
        this->mergePolicy = mergePolicy ? mergePolicy : newLucene<LogByteSizeMergePolicy>();
        return shared_from_this();
    }

    MergePolicyPtr IndexWriterConfig::getMergePolicy()
    {
        return mergePolicy;
    }

    IndexWriterConfigPtr IndexWriterConfig::setMaxThreadStates(int32_t maxThreadStates)
    {
        this->maxThreadStates = maxThreadStates < 1 ? DEFAULT_MAX_THREAD_STATES : maxThreadStates;
        return shared_from_this();
    }

    int32_t IndexWriterConfig::getMaxThreadStates()
    {
        return maxThreadStates;
    }

    IndexWriterConfigPtr IndexWriterConfig::setReaderPooling(bool readerPooling)
    {
        this->readerPooling = readerPooling;
        return shared_from_this();
    }

    bool IndexWriterConfig::getReaderPooling()
    {
        return readerPooling;
    }

    IndexWriterConfigPtr IndexWriterConfig::setIndexingChain(IndexingChainPtr indexingChain)
    {
        this->indexingChain = indexingChain ? indexingChain : DocumentsWriter::defaultIndexingChain();
        return shared_from_this();
    }

    IndexingChainPtr IndexWriterConfig::getIndexingChain()
    {
        return indexingChain;
    }

    IndexWriterConfigPtr IndexWriterConfig::setReaderTermsIndexDivisor(int32_t divisor)
    {
        if (divisor <= 0 && divisor != -1)
            boost::throw_exception(IllegalArgumentException(L"divisor must be >= 1, or -1 (got " + StringUtils::toString(divisor) + L")"));
        readerTermsIndexDivisor = divisor;
        return shared_from_this();
    }

    int32_t IndexWriterConfig::getReaderTermsIndexDivisor()
    {
        return readerTermsIndexDivisor;
    }

    String IndexWriterConfig::toString()
    {
        StringStream buffer;
        buffer << L"matchVersion=" << matchVersion << L"\n";
        buffer << L"analyzer=" << (analyzer ? analyzer->getClassName() : L"null") << L"\n";
        buffer << L"delPolicy=" << delPolicy->getClassName() << L"\n";
        buffer << L"commit=" << (commit ? commit->toString() : L"null") << L"\n";
        buffer << L"openMode=" << openMode << L"\n";
        buffer << L"similarity=" << similarity->getClassName() << L"\n";
        buffer << L"termIndexInterval=" << termIndexInterval << L"\n";
        buffer << L"mergeScheduler=" << mergeScheduler->getClassName() << L"\n";
        buffer << L"default WRITE_LOCK_TIMEOUT=" << WRITE_LOCK_TIMEOUT << L"\n";
        buffer << L"writeLockTimeout=" << writeLockTimeout << L"\n";
        buffer << L"maxBufferedDeleteTerms=" << maxBufferedDeleteTerms << L"\n";
        buffer << L"ramBufferSizeMB=" << ramBufferSizeMB << L"\n";
        buffer << L"maxBufferedDocs=" << maxBufferedDocs << L"\n";
        buffer << L"mergedSegmentWarmer=" << mergedSegmentWarmer << L"\n";
        buffer << L"mergePolicy=" << mergePolicy << L"\n";
        buffer << L"maxThreadStates=" << maxThreadStates << L"\n";
        buffer << L"readerPooling=" << readerPooling << L"\n";
        buffer << L"readerTermsIndexDivisor=" << readerTermsIndexDivisor << L"\n";
        return buffer.str();
    }
}

