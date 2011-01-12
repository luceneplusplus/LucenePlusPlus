/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "DirectoryReader.h"
#include "RAMDirectory.h"
#include "Document.h"
#include "DocHelper.h"
#include "SegmentInfos.h"
#include "IndexReader.h"
#include "TermFreqVector.h"
#include "Field.h"
#include "DefaultSimilarity.h"
#include "MultiReader.h"
#include "StandardAnalyzer.h"
#include "IndexWriter.h"
#include "TermDocs.h"
#include "TermEnum.h"
#include "Term.h"

using namespace Lucene;

class DirectoryReaderTestFixture : public LuceneTestFixture, public DocHelper
{
public:
    DirectoryReaderTestFixture()
    {
        readers = Collection<SegmentReaderPtr>::newInstance(2);
        dir = newLucene<RAMDirectory>();
        doc1 = newLucene<Document>();
        doc2 = newLucene<Document>();
        DocHelper::setupDoc(doc1);
        DocHelper::setupDoc(doc2);
        DocHelper::writeDoc(dir, doc1);
        DocHelper::writeDoc(dir, doc2);
        sis = newLucene<SegmentInfos>();
        sis->read(dir);
    }
    
    virtual ~DirectoryReaderTestFixture()
    {
    }

protected:
    DirectoryPtr dir;
    DocumentPtr doc1;
    DocumentPtr doc2;
    Collection<SegmentReaderPtr> readers;
    SegmentInfosPtr sis;
  
public:
    void doTestDocument()
    {
        sis->read(dir);
        IndexReaderPtr reader = openReader();
        BOOST_CHECK(reader);
        DocumentPtr newDoc1 = reader->document(0);
        BOOST_CHECK(newDoc1);
        BOOST_CHECK(DocHelper::numFields(newDoc1) == DocHelper::numFields(doc1) - DocHelper::unstored.size());
        DocumentPtr newDoc2 = reader->document(1);
        BOOST_CHECK(newDoc2);
        BOOST_CHECK(DocHelper::numFields(newDoc2) == DocHelper::numFields(doc2) - DocHelper::unstored.size());
        TermFreqVectorPtr vector = reader->getTermFreqVector(0, DocHelper::TEXT_FIELD_2_KEY);
        BOOST_CHECK(vector);
        checkNorms(reader);
    }
    
    void doTestUndeleteAll()
    {
        sis->read(dir);
        IndexReaderPtr reader = openReader();
        BOOST_CHECK(reader);
        BOOST_CHECK_EQUAL(2, reader->numDocs());
        reader->deleteDocument(0);
        BOOST_CHECK_EQUAL(1, reader->numDocs());
        reader->undeleteAll();
        BOOST_CHECK_EQUAL(2, reader->numDocs());

        // Ensure undeleteAll survives commit/close/reopen
        reader->commit(MapStringString());
        reader->close();
        
        if (boost::dynamic_pointer_cast<MultiReader>(reader))
        {
            // MultiReader does not "own" the directory so it does not write the changes to sis on commit
            sis->commit(dir);
        }
        
        sis->read(dir);
        reader = openReader();
        BOOST_CHECK_EQUAL(2, reader->numDocs());

        reader->deleteDocument(0);
        BOOST_CHECK_EQUAL(1, reader->numDocs());
        reader->commit(MapStringString());
        reader->close();
        
        if (boost::dynamic_pointer_cast<MultiReader>(reader))
        {
            // MultiReader does not "own" the directory so it does not write the changes to sis on commit
            sis->commit(dir);
        }
        
        sis->read(dir);
        reader = openReader();
        BOOST_CHECK_EQUAL(1, reader->numDocs());
    }

protected:
    IndexReaderPtr openReader()
    {
        IndexReaderPtr reader = IndexReader::open(dir, false);
        BOOST_CHECK(boost::dynamic_pointer_cast<DirectoryReader>(reader));
        BOOST_CHECK(dir);
        BOOST_CHECK(sis);
        BOOST_CHECK(reader);
        return reader;
    }
    
    void checkNorms(IndexReaderPtr reader)
    {
        for (Collection<FieldPtr>::iterator field = DocHelper::fields.begin(); field != DocHelper::fields.end(); ++field)
        {
            if ((*field)->isIndexed())
            {
                BOOST_CHECK_EQUAL(reader->hasNorms((*field)->name()), !(*field)->getOmitNorms());
                BOOST_CHECK_EQUAL(reader->hasNorms((*field)->name()), !DocHelper::noNorms.contains((*field)->name()));
                if (!reader->hasNorms((*field)->name()))
                {
                    // test for fake norms of 1.0 or null depending on the flag
                    ByteArray norms = reader->norms((*field)->name());
                    uint8_t norm1 = DefaultSimilarity::encodeNorm(1.0);
                    BOOST_CHECK(!norms);
                    norms = ByteArray::newInstance(reader->maxDoc());
                    reader->norms((*field)->name(), norms, 0);
                    for (int32_t j = 0; j < reader->maxDoc(); ++j)
                        BOOST_CHECK_EQUAL(norms[j], norm1);
                }
            }
        }
    }
    
    void addDoc(RAMDirectoryPtr ramDir1, const String& s, bool create)
    {
        IndexWriterPtr iw = newLucene<IndexWriter>(ramDir1, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), create, IndexWriter::MaxFieldLengthLIMITED);
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"body", s, Field::STORE_YES, Field::INDEX_ANALYZED));
        iw->addDocument(doc);
        iw->close();
    }
};

BOOST_FIXTURE_TEST_SUITE(DirectoryReaderTest, DirectoryReaderTestFixture)

BOOST_AUTO_TEST_CASE(testDirectoryReader)
{
    doTestDocument();
    doTestUndeleteAll();
}

BOOST_AUTO_TEST_CASE(testIsCurrent)
{
    RAMDirectoryPtr ramDir1 = newLucene<RAMDirectory>();
    addDoc(ramDir1, L"test foo", true);
    RAMDirectoryPtr ramDir2 = newLucene<RAMDirectory>();
    addDoc(ramDir2, L"test blah", true);
    MultiReaderPtr mr = newLucene<MultiReader>(newCollection<IndexReaderPtr>(IndexReader::open(ramDir1, false), IndexReader::open(ramDir2, false)));
    BOOST_CHECK(mr->isCurrent()); // just opened, must be current
    addDoc(ramDir1, L"more text", false);
    BOOST_CHECK(!mr->isCurrent()); // has been modified, not current anymore
    addDoc(ramDir2, L"even more text", false);
    BOOST_CHECK(!mr->isCurrent()); // has been modified even more, not current anymore
    BOOST_CHECK_EXCEPTION(mr->getVersion(), LuceneException, check_exception(LuceneException::UnsupportedOperation));
    mr->close();
}

BOOST_AUTO_TEST_CASE(testMultiTermDocs)
{
    RAMDirectoryPtr ramDir1 = newLucene<RAMDirectory>();
    addDoc(ramDir1, L"test foo", true);
    RAMDirectoryPtr ramDir2 = newLucene<RAMDirectory>();
    addDoc(ramDir2, L"test blah", true);
    RAMDirectoryPtr ramDir3 = newLucene<RAMDirectory>();
    addDoc(ramDir3, L"test wow", true);

    Collection<IndexReaderPtr> readers1 = newCollection<IndexReaderPtr>(IndexReader::open(ramDir1, false), IndexReader::open(ramDir3, false));
    Collection<IndexReaderPtr> readers2 = newCollection<IndexReaderPtr>(IndexReader::open(ramDir1, false), IndexReader::open(ramDir2, false), IndexReader::open(ramDir3, false));

    MultiReaderPtr mr2 = newLucene<MultiReader>(readers1);
    MultiReaderPtr mr3 = newLucene<MultiReader>(readers2);

    // test mixing up TermDocs and TermEnums from different readers.
    TermDocsPtr td2 = mr2->termDocs();
    TermEnumPtr te3 = mr3->terms(newLucene<Term>(L"body", L"wow"));
    td2->seek(te3);
    int32_t ret = 0;

    // This should blow up if we forget to check that the TermEnum is from the same reader as the TermDocs.
    while (td2->next())
        ret += td2->doc();
    td2->close();
    te3->close();

    // really a dummy check to ensure that we got some docs and to ensure that nothing is optimized out.
    BOOST_CHECK(ret > 0);
}

BOOST_AUTO_TEST_CASE(testAllTermDocs)
{
    IndexReaderPtr reader = openReader();
    int32_t NUM_DOCS = 2;
    TermDocsPtr td = reader->termDocs(TermPtr());
    for (int32_t i = 0; i < NUM_DOCS; ++i)
    {
        BOOST_CHECK(td->next());
        BOOST_CHECK_EQUAL(i, td->doc());
        BOOST_CHECK_EQUAL(1, td->freq());
    }
    td->close();
    reader->close();
}

BOOST_AUTO_TEST_SUITE_END()
