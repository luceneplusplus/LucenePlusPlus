/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "TermVectorOffsetInfo.h"
#include "PositionBasedTermVectorMapper.h"
#include "BitSet.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(PositionBasedTermVectorMapperTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testPayload)
{
    Collection<String> tokens = newCollection<String>(L"here", L"is", L"some", L"text", L"to", L"test", L"extra");
    Collection< Collection<int32_t> > thePositions = Collection< Collection<int32_t> >::newInstance(tokens.size());
    Collection< Collection<TermVectorOffsetInfoPtr> > offsets = Collection< Collection<TermVectorOffsetInfoPtr> >::newInstance(tokens.size());
    int32_t numPositions = 0;
    
    // save off the last one so we can add it with the same positions as some of the others, but in a predictable way
    for (int32_t i = 0; i < tokens.size() - 1; ++i)
    {
        thePositions[i] = Collection<int32_t>::newInstance(2 * i + 1); // give 'em all some positions
        for (int32_t j = 0; j < thePositions[i].size(); ++j)
            thePositions[i][j] = numPositions++;
        
        offsets[i] = Collection<TermVectorOffsetInfoPtr>::newInstance(thePositions[i].size());
        for (int32_t j = 0; j < offsets[i].size(); ++j)
            offsets[i][j] = newLucene<TermVectorOffsetInfo>(j, j + 1); // the actual value here doesn't much matter
    }

    thePositions[tokens.size() - 1] = Collection<int32_t>::newInstance(1);
    thePositions[tokens.size() - 1][0] = 0; // put this at the same position as "here"
    offsets[tokens.size() - 1] = Collection<TermVectorOffsetInfoPtr>::newInstance(1);
    offsets[tokens.size() - 1][0] = newLucene<TermVectorOffsetInfo>(0, 1);
    
    PositionBasedTermVectorMapperPtr mapper = newLucene<PositionBasedTermVectorMapper>();
    mapper->setExpectations(L"test", tokens.size(), true, true);
    
    // Test single position
    for (int32_t i = 0; i < tokens.size(); ++i)
    {
        String token = tokens[i];
        mapper->map(token, 1, Collection<TermVectorOffsetInfoPtr>(), thePositions[i]);
    }

    MapStringMapIntTermVectorsPositionInfo map = mapper->getFieldToTerms();
    BOOST_CHECK(map);
    BOOST_CHECK_EQUAL(map.size(), 1);
    MapIntTermVectorsPositionInfo positions = map.get(L"test");
    BOOST_CHECK(positions);

    BOOST_CHECK_EQUAL(positions.size(), numPositions);
    BitSetPtr bits = newLucene<BitSet>(numPositions);
    for (MapIntTermVectorsPositionInfo::iterator entry = positions.begin(); entry != positions.end(); ++entry)
    {
        BOOST_CHECK(entry->second);
        int32_t pos = entry->first;
        bits->set(pos);
        BOOST_CHECK_EQUAL(entry->second->getPosition(), pos);
        BOOST_CHECK(entry->second->getOffsets());
        if (pos == 0)
        {
            BOOST_CHECK_EQUAL(entry->second->getTerms().size(), 2); // need a test for multiple terms at one pos
            BOOST_CHECK_EQUAL(entry->second->getOffsets().size(), 2);
        }
        else
        {
            BOOST_CHECK_EQUAL(entry->second->getTerms().size(), 1); // need a test for multiple terms at one pos
            BOOST_CHECK_EQUAL(entry->second->getOffsets().size(), 1);
        }
    }
    BOOST_CHECK_EQUAL(bits->cardinality(), numPositions);
}

BOOST_AUTO_TEST_SUITE_END()
