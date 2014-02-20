/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "StandardTokenizer.h"
#include "StringReader.h"
#include "ElisionFilter.h"
#include "TermAttribute.h"

using namespace Lucene;

class ElisionTest : public BaseTokenStreamFixture {
public:
    virtual ~ElisionTest() {
    }

public:
    Collection<String> addTerms(const TokenFilterPtr& filter) {
        Collection<String> terms = Collection<String>::newInstance();
        TermAttributePtr termAtt = filter->getAttribute<TermAttribute>();
        while (filter->incrementToken()) {
            terms.add(termAtt->term());
        }
        return terms;
    }
};

TEST_F(ElisionTest, testElision) {
    String test = L"Plop, juste pour voir l'embrouille avec O'brian. M'enfin.";
    TokenizerPtr tokenizer = newLucene<StandardTokenizer>(LuceneVersion::LUCENE_CURRENT, newLucene<StringReader>(test));
    HashSet<String> articles = HashSet<String>::newInstance();
    articles.add(L"l");
    articles.add(L"M");
    TokenFilterPtr filter = newLucene<ElisionFilter>(tokenizer, articles);
    Collection<String> terms = addTerms(filter);
    EXPECT_EQ(L"embrouille", terms[4]);
    EXPECT_EQ(L"O'brian", terms[6]);
    EXPECT_EQ(L"enfin", terms[7]);
}
