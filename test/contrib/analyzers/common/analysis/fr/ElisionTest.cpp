/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
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

class ElisionFixture : public BaseTokenStreamFixture
{
public:
	virtual ~ElisionFixture()
	{
	}

public:
    Collection<String> addTerms(TokenFilterPtr filter)
    {
        Collection<String> terms = Collection<String>::newInstance();
        TermAttributePtr termAtt = filter->getAttribute<TermAttribute>();
        while (filter->incrementToken())
            terms.add(termAtt->term());
        return terms;
    }
};

BOOST_FIXTURE_TEST_SUITE(ElisionTest, ElisionFixture)

BOOST_AUTO_TEST_CASE(testElision)
{
    String test = L"Plop, juste pour voir l'embrouille avec O'brian. M'enfin.";
    TokenizerPtr tokenizer = newLucene<StandardTokenizer>(LuceneVersion::LUCENE_CURRENT, newLucene<StringReader>(test));
    HashSet<String> articles = HashSet<String>::newInstance();
    articles.add(L"l");
    articles.add(L"M");
    TokenFilterPtr filter = newLucene<ElisionFilter>(tokenizer, articles);
    Collection<String> terms = addTerms(filter);
    BOOST_CHECK_EQUAL(L"embrouille", terms[4]);
    BOOST_CHECK_EQUAL(L"O'brian", terms[6]);
    BOOST_CHECK_EQUAL(L"enfin", terms[7]);
}

BOOST_AUTO_TEST_SUITE_END()
