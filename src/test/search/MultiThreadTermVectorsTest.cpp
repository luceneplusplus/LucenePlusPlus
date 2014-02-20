/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "SimpleAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexReader.h"
#include "LuceneThread.h"
#include "TermFreqVector.h"
#include "MiscUtils.h"

using namespace Lucene;

DECLARE_SHARED_PTR(MultiThreadTermVectorsReader)

class MultiThreadTermVectorsReader : public LuceneThread {
public:
    MultiThreadTermVectorsReader(const IndexReaderPtr& reader) {
        this->reader = reader;
        timeElapsed = 0;
    }

    virtual ~MultiThreadTermVectorsReader() {
    }

    LUCENE_CLASS(MultiThreadTermVectorsReader);

protected:
    IndexReaderPtr reader;
    int64_t timeElapsed;

    static const int32_t runsToDo;

public:
    virtual void run() {
        try {
            for (int32_t i = 0; i < runsToDo; ++i) {
                testTermVectors();
            }
        } catch (LuceneException& e) {
            FAIL() << "Unexpected exception: " << e.getError();
        }
    }

protected:
    void testTermVectors() {
        int32_t numDocs = reader->numDocs();
        int64_t start = 0;
        for (int32_t docId = 0; docId < numDocs; ++docId) {
            start = MiscUtils::currentTimeMillis();
            Collection<TermFreqVectorPtr> vectors = reader->getTermFreqVectors(docId);
            timeElapsed += MiscUtils::currentTimeMillis() - start;

            // verify vectors result
            verifyVectors(vectors, docId);

            start = MiscUtils::currentTimeMillis();
            TermFreqVectorPtr vector = reader->getTermFreqVector(docId, L"field");
            timeElapsed += MiscUtils::currentTimeMillis() - start;

            vectors = newCollection<TermFreqVectorPtr>(vector);

            verifyVectors(vectors, docId);
        }
    }

    void verifyVectors(Collection<TermFreqVectorPtr> vectors, int32_t num) {
        StringStream temp;
        Collection<String> terms;
        for (int32_t i = 0; i < vectors.size(); ++i) {
            terms = vectors[i]->getTerms();
            for (int32_t z = 0; z < terms.size(); ++z) {
                temp << terms[z];
            }
        }

        if (intToEnglish(num) != temp.str()) {
            FAIL() << intToEnglish(num) << "!=" << temp.str();
        }
    }
};

const int32_t MultiThreadTermVectorsReader::runsToDo = 100;

class MultiThreadTermVectorsTest : public LuceneTestFixture {
public:
    MultiThreadTermVectorsTest() {
        directory = newLucene<RAMDirectory>();
        numDocs = 100;
        numThreads = 3;

        IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        for (int32_t i = 0; i < numDocs; ++i) {
            DocumentPtr doc = newLucene<Document>();
            FieldablePtr fld = newLucene<Field>(L"field", intToEnglish(i), Field::STORE_YES, Field::INDEX_NOT_ANALYZED, Field::TERM_VECTOR_YES);
            doc->add(fld);
            writer->addDocument(doc);
        }
        writer->close();
    }

    virtual ~MultiThreadTermVectorsTest() {
    }

protected:
    RAMDirectoryPtr directory;
    int32_t numDocs;
    int32_t numThreads;

public:
    void testTermPositionVectors(const IndexReaderPtr& reader, int32_t threadCount) {
        Collection<MultiThreadTermVectorsReaderPtr> mtr = Collection<MultiThreadTermVectorsReaderPtr>::newInstance(threadCount);
        for (int32_t i = 0; i < threadCount; ++i) {
            mtr[i] = newLucene<MultiThreadTermVectorsReader>(reader);
            mtr[i]->start();
        }

        for (int32_t i = 0; i < threadCount; ++i) {
            mtr[i]->join();
        }
    }
};

TEST_F(MultiThreadTermVectorsTest, testMultiThreadTermVectors) {
    IndexReaderPtr reader;

    try {
        reader = IndexReader::open(directory, true);
        for (int32_t i = 1; i <= numThreads; ++i) {
            testTermPositionVectors(reader, i);
        }
    } catch (LuceneException& e) {
        FAIL() << e.getError();
    }
}
