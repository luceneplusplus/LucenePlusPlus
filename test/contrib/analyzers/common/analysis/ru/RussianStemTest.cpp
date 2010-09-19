/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TestUtils.h"
#include "BaseTokenStreamFixture.h"
#include "RussianStemmer.h"
#include "FileReader.h"
#include "BufferedReader.h"
#include "InputStreamReader.h"

using namespace Lucene;

class RussianStemmerFixture : public BaseTokenStreamFixture
{
public:
    RussianStemmerFixture()
    {
        words = Collection<String>::newInstance();
        stems = Collection<String>::newInstance();
        
        String wordsFile(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"russian"), L"wordsUTF8.txt"));
        String stemsFile(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"russian"), L"stemsUTF8.txt"));
        
        BufferedReaderPtr inWords = newLucene<BufferedReader>(newLucene<InputStreamReader>(newLucene<FileReader>(wordsFile)));
        String word;
        while (inWords->readLine(word))
            words.add(word);
        inWords->close();
        
        BufferedReaderPtr inStems = newLucene<BufferedReader>(newLucene<InputStreamReader>(newLucene<FileReader>(stemsFile)));
        String stem;
        while (inStems->readLine(stem))
            stems.add(stem);
        inStems->close();
    }
    
	virtual ~RussianStemmerFixture()
	{
	}

protected:
    Collection<String> words;
    Collection<String> stems;
};

BOOST_FIXTURE_TEST_SUITE(RussianStemmerTest, RussianStemmerFixture)

BOOST_AUTO_TEST_CASE(testStem)
{
    BOOST_CHECK_EQUAL(words.size(), stems.size());
    for (int32_t i = 0; i < words.size(); ++i)
    {
        String realStem = RussianStemmer::stemWord(words[i]);
        BOOST_CHECK_EQUAL(stems[i], realStem);
    }
}

BOOST_AUTO_TEST_SUITE_END()
