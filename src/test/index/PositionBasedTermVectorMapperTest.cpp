/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "TermVectorOffsetInfo.h"
#include "PositionBasedTermVectorMapper.h"
#include "BitSet.h"

using namespace Lucene;

typedef LuceneTestFixture PositionBasedTermVectorMapperTest;

TEST_F(PositionBasedTermVectorMapperTest, testPayload) {
    Collection<String> tokens = newCollection<String>(L"here", L"is", L"some", L"text", L"to", L"test", L"extra");
    Collection< Collection<int32_t> > thePositions = Collection< Collection<int32_t> >::newInstance(tokens.size());
    Collection< Collection<TermVectorOffsetInfoPtr> > offsets = Collection< Collection<TermVectorOffsetInfoPtr> >::newInstance(tokens.size());
    int32_t numPositions = 0;

    // save off the last one so we can add it with the same positions as some of the others, but in a predictable way
    for (int32_t i = 0; i < tokens.size() - 1; ++i) {
        thePositions[i] = Collection<int32_t>::newInstance(2 * i + 1); // give 'em all some positions
        for (int32_t j = 0; j < thePositions[i].size(); ++j) {
            thePositions[i][j] = numPositions++;
        }

        offsets[i] = Collection<TermVectorOffsetInfoPtr>::newInstance(thePositions[i].size());
        for (int32_t j = 0; j < offsets[i].size(); ++j) {
            offsets[i][j] = newLucene<TermVectorOffsetInfo>(j, j + 1);    // the actual value here doesn't much matter
        }
    }

    thePositions[tokens.size() - 1] = Collection<int32_t>::newInstance(1);
    thePositions[tokens.size() - 1][0] = 0; // put this at the same position as "here"
    offsets[tokens.size() - 1] = Collection<TermVectorOffsetInfoPtr>::newInstance(1);
    offsets[tokens.size() - 1][0] = newLucene<TermVectorOffsetInfo>(0, 1);

    PositionBasedTermVectorMapperPtr mapper = newLucene<PositionBasedTermVectorMapper>();
    mapper->setExpectations(L"test", tokens.size(), true, true);

    // Test single position
    for (int32_t i = 0; i < tokens.size(); ++i) {
        String token = tokens[i];
        mapper->map(token, 1, Collection<TermVectorOffsetInfoPtr>(), thePositions[i]);
    }

    MapStringMapIntTermVectorsPositionInfo map = mapper->getFieldToTerms();
    EXPECT_TRUE(map);
    EXPECT_EQ(map.size(), 1);
    MapIntTermVectorsPositionInfo positions = map.get(L"test");
    EXPECT_TRUE(positions);

    EXPECT_EQ(positions.size(), numPositions);
    BitSetPtr bits = newLucene<BitSet>(numPositions);
    for (MapIntTermVectorsPositionInfo::iterator entry = positions.begin(); entry != positions.end(); ++entry) {
        EXPECT_TRUE(entry->second);
        int32_t pos = entry->first;
        bits->set(pos);
        EXPECT_EQ(entry->second->getPosition(), pos);
        EXPECT_TRUE(entry->second->getOffsets());
        if (pos == 0) {
            EXPECT_EQ(entry->second->getTerms().size(), 2); // need a test for multiple terms at one pos
            EXPECT_EQ(entry->second->getOffsets().size(), 2);
        } else {
            EXPECT_EQ(entry->second->getTerms().size(), 1); // need a test for multiple terms at one pos
            EXPECT_EQ(entry->second->getOffsets().size(), 1);
        }
    }
    EXPECT_EQ(bits->cardinality(), numPositions);
}
