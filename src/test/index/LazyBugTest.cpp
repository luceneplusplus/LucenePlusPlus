/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "FieldSelector.h"
#include "RAMDirectory.h"
#include "IndexReader.h"
#include "SimpleAnalyzer.h"
#include "IndexWriter.h"
#include "Document.h"
#include "Field.h"
#include "Random.h"

using namespace Lucene;

class LazyBugTest : public LuceneTestFixture {
public:
    virtual ~LazyBugTest() {
    }

public:
    static const int32_t NUM_DOCS;
    static const int32_t NUM_FIELDS;

public:
    Collection<String> data() {
        static Collection<String> _data;
        if (!_data) {
            _data = Collection<String>::newInstance();
            _data.add(L"now");
            _data.add(L"is the time");
            _data.add(L"for all good men");
            _data.add(L"to come to the aid");
            _data.add(L"of their country!");
            _data.add(L"this string contains big chars:{\u0111 \u0222 \u0333 \u1111 \u2222 \u3333}");
            _data.add(L"this string is a bigger string, mary had a little lamb, little lamb, little lamb!");
        }
        return _data;
    }

    HashSet<String> dataset() {
        static HashSet<String> _dataset;
        if (!_dataset) {
            Collection<String> _data = data();
            _dataset = HashSet<String>::newInstance(_data.begin(), _data.end());
        }
        return _dataset;
    }

    String MAGIC_FIELD() {
        return L"f" + StringUtils::toString((double)NUM_FIELDS / 3);
    }

    DECLARE_SHARED_PTR(LazyBugSelector)

    class LazyBugSelector : public FieldSelector {
    public:
        LazyBugSelector(const String& magicField) {
            this->magicField = magicField;
        }

        virtual ~LazyBugSelector() {
        }

        LUCENE_CLASS(LazyBugSelector);

    protected:
        String magicField;

    public:
        virtual FieldSelectorResult accept(const String& fieldName) {
            if (fieldName == magicField) {
                return FieldSelector::SELECTOR_LOAD;
            } else {
                return FieldSelector::SELECTOR_LAZY_LOAD;
            }
        }
    };

    FieldSelectorPtr SELECTOR() {
        return newLucene<LazyBugSelector>(MAGIC_FIELD());
    }

    DirectoryPtr makeIndex() {
        DirectoryPtr dir = newLucene<RAMDirectory>();
        RandomPtr rand = newLucene<Random>();

        try {
            AnalyzerPtr analyzer = newLucene<SimpleAnalyzer>();
            IndexWriterPtr writer = newLucene<IndexWriter>(dir, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);

            writer->setUseCompoundFile(false);
            Collection<String> _data = data();

            for (int32_t d = 1; d <= NUM_DOCS; ++d) {
                DocumentPtr doc = newLucene<Document>();
                for (int32_t f = 1; f <= NUM_FIELDS; ++f) {
                    doc->add(newLucene<Field>(L"f" + StringUtils::toString(f), _data[f % _data.size()] + L"#" + _data[rand->nextInt(_data.size())], Field::STORE_YES, Field::INDEX_ANALYZED));
                }
                writer->addDocument(doc);
            }
            writer->close();
        } catch (LuceneException& e) {
            boost::throw_exception(RuntimeException(L"Unexpected exception: " + e.getError()));
        }
        return dir;
    }

    void doTest(Collection<int32_t> docs) {
        DirectoryPtr dir = makeIndex();
        IndexReaderPtr reader = IndexReader::open(dir, true);
        HashSet<String> _dataset = dataset();

        for (int32_t i = 0; i < docs.size(); ++i) {
            DocumentPtr d = reader->document(docs[i], SELECTOR());
            d->get(MAGIC_FIELD());

            Collection<FieldablePtr> fields = d->getFields();
            for (Collection<FieldablePtr>::iterator field = fields.begin(); field != fields.end(); ++field) {
                try {
                    String fname = (*field)->name();
                    String fval = (*field)->stringValue();
                    EXPECT_TRUE(!fval.empty());

                    Collection<String> vals = StringUtils::split(fval, L"#");
                    EXPECT_EQ(vals.size(), 2);
                    if (!_dataset.contains(vals[0]) || !_dataset.contains(vals[1])) {
                        FAIL() << "FIELD:" << fname << ",VAL:" << fval;
                    }
                } catch (LuceneException& e) {
                    FAIL() << "Unexpected exception: " << e.getError();
                }
            }
        }
        reader->close();
    }
};

const int32_t LazyBugTest::NUM_DOCS = 500;
const int32_t LazyBugTest::NUM_FIELDS = 100;

/// Test demonstrating EOF bug on the last field of the last doc if other docs have already been accessed.

TEST_F(LazyBugTest, testLazyWorks) {
    doTest(newCollection<int32_t>(399));
}

TEST_F(LazyBugTest, testLazyAlsoWorks) {
    doTest(newCollection<int32_t>(399, 150));
}

TEST_F(LazyBugTest, testLazyBroken) {
    doTest(newCollection<int32_t>(150, 399));
}
