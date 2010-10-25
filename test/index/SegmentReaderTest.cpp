/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "DocHelper.h"
#include "RAMDirectory.h"
#include "Document.h"
#include "SegmentReader.h"
#include "SegmentInfo.h"
#include "Field.h"
#include "TermEnum.h"
#include "Term.h"
#include "TermDocs.h"
#include "TermPositions.h"
#include "DefaultSimilarity.h"
#include "TermFreqVector.h"

using namespace Lucene;

class SegmentReaderTestFixture : public LuceneTestFixture, public DocHelper
{
public:
    SegmentReaderTestFixture()
    {
        dir = newLucene<RAMDirectory>();
        testDoc = newLucene<Document>();
        DocHelper::setupDoc(testDoc);
        SegmentInfoPtr info = DocHelper::writeDoc(dir, testDoc);
        reader = SegmentReader::get(true, info, IndexReader::DEFAULT_TERMS_INDEX_DIVISOR);
    }
    
    virtual ~SegmentReaderTestFixture()
    {
    }

protected:
    RAMDirectoryPtr dir;
    DocumentPtr testDoc;
    SegmentReaderPtr reader;
};

BOOST_FIXTURE_TEST_SUITE(SegmentReaderTest, SegmentReaderTestFixture)

BOOST_AUTO_TEST_CASE(testSegmentReader)
{
    BOOST_CHECK(dir);
    BOOST_CHECK(reader);
    BOOST_CHECK(DocHelper::nameValues.size() > 0);
    BOOST_CHECK_EQUAL(DocHelper::numFields(testDoc), DocHelper::all.size());
}

BOOST_AUTO_TEST_CASE(testDocument)
{
    BOOST_CHECK_EQUAL(reader->numDocs(), 1);
    BOOST_CHECK(reader->maxDoc() >= 1);
    DocumentPtr result = reader->document(0);
    BOOST_CHECK(result);
    
    // There are 2 unstored fields on the document that are not preserved across writing
    BOOST_CHECK_EQUAL(DocHelper::numFields(result), DocHelper::numFields(testDoc) - DocHelper::unstored.size());

    Collection<FieldablePtr> fields = result->getFields();
    for (Collection<FieldablePtr>::iterator field = fields.begin(); field != fields.end(); ++field)
    {
        BOOST_CHECK(*field);
        BOOST_CHECK(DocHelper::nameValues.contains((*field)->name()));
    }
}

BOOST_AUTO_TEST_CASE(testDelete)
{
    DocumentPtr docToDelete = newLucene<Document>();
    DocHelper::setupDoc(docToDelete);
    SegmentInfoPtr info = DocHelper::writeDoc(dir, docToDelete);
    SegmentReaderPtr deleteReader = SegmentReader::get(false, info, IndexReader::DEFAULT_TERMS_INDEX_DIVISOR);
    BOOST_CHECK(deleteReader);
    BOOST_CHECK_EQUAL(deleteReader->numDocs(), 1);
    deleteReader->deleteDocument(0);
    BOOST_CHECK(deleteReader->isDeleted(0));
    BOOST_CHECK(deleteReader->hasDeletions());
    BOOST_CHECK_EQUAL(deleteReader->numDocs(), 0);
}

BOOST_AUTO_TEST_CASE(testGetFieldNameVariations)
{
    HashSet<String> result = reader->getFieldNames(IndexReader::FIELD_OPTION_ALL);
    BOOST_CHECK(result);
    BOOST_CHECK_EQUAL(result.size(), DocHelper::all.size());
    for (HashSet<String>::iterator field = result.begin(); field != result.end(); ++field)
        BOOST_CHECK(DocHelper::nameValues.contains(*field) || field->empty());
    result = reader->getFieldNames(IndexReader::FIELD_OPTION_INDEXED);
    BOOST_CHECK(result);
    BOOST_CHECK_EQUAL(result.size(), DocHelper::indexed.size());
    for (HashSet<String>::iterator field = result.begin(); field != result.end(); ++field)
        BOOST_CHECK(DocHelper::indexed.contains(*field) || field->empty());
    result = reader->getFieldNames(IndexReader::FIELD_OPTION_UNINDEXED);
    BOOST_CHECK(result);
    BOOST_CHECK_EQUAL(result.size(), DocHelper::unindexed.size());
    
    // Get all indexed fields that are storing term vectors
    result = reader->getFieldNames(IndexReader::FIELD_OPTION_INDEXED_WITH_TERMVECTOR);
    BOOST_CHECK(result);
    BOOST_CHECK_EQUAL(result.size(), DocHelper::termvector.size());

    result = reader->getFieldNames(IndexReader::FIELD_OPTION_INDEXED_NO_TERMVECTOR);
    BOOST_CHECK(result);
    BOOST_CHECK_EQUAL(result.size(), DocHelper::notermvector.size());
}

BOOST_AUTO_TEST_CASE(testTerms)
{
    TermEnumPtr terms = reader->terms();
    BOOST_CHECK(terms);
    while (terms->next())
    {
        TermPtr term = terms->term();
        BOOST_CHECK(term);
        String fieldValue = DocHelper::nameValues.get(term->field());
        BOOST_CHECK_NE(fieldValue.find(term->text()), -1);
    }

    TermDocsPtr termDocs = reader->termDocs();
    BOOST_CHECK(termDocs);
    termDocs->seek(newLucene<Term>(DocHelper::TEXT_FIELD_1_KEY, L"field"));
    BOOST_CHECK(termDocs->next());

    termDocs->seek(newLucene<Term>(DocHelper::NO_NORMS_KEY, DocHelper::NO_NORMS_TEXT));
    BOOST_CHECK(termDocs->next());

    TermPositionsPtr positions = reader->termPositions();
    positions->seek(newLucene<Term>(DocHelper::TEXT_FIELD_1_KEY, L"field"));
    BOOST_CHECK(positions);
    BOOST_CHECK_EQUAL(positions->doc(), 0);
    BOOST_CHECK(positions->nextPosition() >= 0);
}

BOOST_AUTO_TEST_CASE(testNorms)
{
    // test omit norms
    for (int32_t i = 0; i < DocHelper::fields.size(); ++i)
    {
        FieldPtr f = DocHelper::fields[i];
        if (f->isIndexed())
        {
            bool a = reader->hasNorms(f->name());
            bool b = !f->getOmitNorms();

            BOOST_CHECK_EQUAL(reader->hasNorms(f->name()), !f->getOmitNorms());
            BOOST_CHECK_EQUAL(reader->hasNorms(f->name()), !DocHelper::noNorms.contains(f->name()));
            
            if (!reader->hasNorms(f->name()))
            {
                // test for fake norms of 1.0 or null depending on the flag
                ByteArray norms = reader->norms(f->name());
                uint8_t norm1 = DefaultSimilarity::encodeNorm(1.0);
                BOOST_CHECK(!norms);
                norms.resize(reader->maxDoc());
                reader->norms(f->name(), norms, 0);
                for (int32_t j = 0; j < reader->maxDoc(); ++j)
                    BOOST_CHECK_EQUAL(norms[j], norm1);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testTermVectors)
{
    TermFreqVectorPtr result = reader->getTermFreqVector(0, DocHelper::TEXT_FIELD_2_KEY);
    BOOST_CHECK(result);
    Collection<String> terms = result->getTerms();
    Collection<int32_t> freqs = result->getTermFrequencies();
    BOOST_CHECK(terms);
    BOOST_CHECK_EQUAL(terms.size(), 3);
    BOOST_CHECK(freqs);
    BOOST_CHECK_EQUAL(freqs.size(), 3);
    for (int32_t i = 0; i < terms.size(); ++i)
    {
        String term = terms[i];
        int32_t freq = freqs[i];
        BOOST_CHECK_NE(String(DocHelper::FIELD_2_TEXT).find(term), -1);
        BOOST_CHECK(freq > 0);
    }

    Collection<TermFreqVectorPtr> results = reader->getTermFreqVectors(0);
    BOOST_CHECK(results);
    BOOST_CHECK_EQUAL(results.size(), 3);      
}

BOOST_AUTO_TEST_SUITE_END()
