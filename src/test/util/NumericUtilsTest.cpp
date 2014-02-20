/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "NumericUtils.h"
#include "OpenBitSet.h"
#include "Random.h"
#include "MiscUtils.h"

using namespace Lucene;

typedef LuceneTestFixture NumericUtilsTest;

class CheckLongRangeBuilder : public LongRangeBuilder {
public:
    CheckLongRangeBuilder(int64_t lower, int64_t upper, OpenBitSetPtr bits,
                          Collection<int64_t>::iterator neededBoundsFirst,
                          Collection<int64_t>::iterator neededBoundsLast,
                          Collection<int32_t>::iterator neededShiftsFirst,
                          Collection<int32_t>::iterator neededShiftsLast) {
        this->lower = lower;
        this->upper = upper;
        this->bits = bits;
        this->neededBoundsFirst = neededBoundsFirst;
        this->neededBoundsLast = neededBoundsLast;
        this->neededShiftsFirst = neededShiftsFirst;
        this->neededShiftsLast = neededShiftsLast;
    }

    virtual ~CheckLongRangeBuilder() {
    }

protected:
    int64_t lower;
    int64_t upper;
    OpenBitSetPtr bits;
    Collection<int64_t>::iterator neededBoundsFirst;
    Collection<int64_t>::iterator neededBoundsLast;
    Collection<int32_t>::iterator neededShiftsFirst;
    Collection<int32_t>::iterator neededShiftsLast;

public:
    virtual void addRange(int64_t min, int64_t max, int32_t shift) {
        EXPECT_TRUE(min >= lower && min <= upper && max >= lower && max <= upper);
        if (bits) {
            for (int64_t l = min; l <= max; ++l) {
                if (bits->getAndSet((int64_t)(l - lower))) {
                    FAIL() << "getAndSet failure";
                }
                // extra exit condition to prevent overflow on MAX_VALUE
                if (l == max) {
                    break;
                }
            }
        }

        if (neededBoundsFirst == neededBoundsLast || neededShiftsFirst == neededShiftsLast) {
            return;
        }

        // make unsigned longs for easier display and understanding
        min ^= 0x8000000000000000LL;
        max ^= 0x8000000000000000LL;

        EXPECT_EQ(*neededShiftsFirst++, shift);
        EXPECT_EQ(*neededBoundsFirst++, MiscUtils::unsignedShift(min, (int64_t)shift)); // inner min bound
        EXPECT_EQ(*neededBoundsFirst++, MiscUtils::unsignedShift(max, (int64_t)shift)); // inner max bound
    }
};

static void checkLongRangeSplit(int64_t lower, int64_t upper, int32_t precisionStep, bool useBitSet, Collection<int64_t> neededBounds, Collection<int32_t> neededShifts) {
    OpenBitSetPtr bits = useBitSet ? newLucene<OpenBitSet>((int64_t)(upper - lower + 1)) : OpenBitSetPtr();
    NumericUtils::splitLongRange(newLucene<CheckLongRangeBuilder>(lower, upper, bits, neededBounds.begin(), neededBounds.end(), neededShifts.begin(), neededShifts.end()), precisionStep, lower, upper);

    if (useBitSet) {
        // after flipping all bits in the range, the cardinality should be zero
        bits->flip(0, (int64_t)(upper - lower + 1));
        EXPECT_TRUE(bits->isEmpty());
    }
}

class CheckIntRangeBuilder : public IntRangeBuilder {
public:
    CheckIntRangeBuilder(int32_t lower, int32_t upper, OpenBitSetPtr bits,
                         Collection<int32_t>::iterator neededBoundsFirst,
                         Collection<int32_t>::iterator neededBoundsLast,
                         Collection<int32_t>::iterator neededShiftsFirst,
                         Collection<int32_t>::iterator neededShiftsLast) {
        this->lower = lower;
        this->upper = upper;
        this->bits = bits;
        this->neededBoundsFirst = neededBoundsFirst;
        this->neededBoundsLast = neededBoundsLast;
        this->neededShiftsFirst = neededShiftsFirst;
        this->neededShiftsLast = neededShiftsLast;
    }

    virtual ~CheckIntRangeBuilder() {
    }

protected:
    int32_t lower;
    int32_t upper;
    OpenBitSetPtr bits;
    Collection<int32_t>::iterator neededBoundsFirst;
    Collection<int32_t>::iterator neededBoundsLast;
    Collection<int32_t>::iterator neededShiftsFirst;
    Collection<int32_t>::iterator neededShiftsLast;

public:
    virtual void addRange(int32_t min, int32_t max, int32_t shift) {
        EXPECT_TRUE(min >= lower && min <= upper && max >= lower && max <= upper);
        if (bits) {
            for (int32_t l = min; l <= max; ++l) {
                if (bits->getAndSet((int32_t)(l - lower))) {
                    FAIL() << "getAndSet failure";
                }
                // extra exit condition to prevent overflow on MAX_VALUE
                if (l == max) {
                    break;
                }
            }
        }

        if (neededBoundsFirst == neededBoundsLast || neededShiftsFirst == neededShiftsLast) {
            return;
        }

        // make unsigned longs for easier display and understanding
        min ^= 0x80000000;
        max ^= 0x80000000;

        EXPECT_EQ(*neededShiftsFirst++, shift);
        EXPECT_EQ(*neededBoundsFirst++, MiscUtils::unsignedShift(min, shift)); // inner min bound
        EXPECT_EQ(*neededBoundsFirst++, MiscUtils::unsignedShift(max, shift)); // inner max bound
    }
};

static void checkIntRangeSplit(int32_t lower, int32_t upper, int32_t precisionStep, bool useBitSet, Collection<int32_t> neededBounds, Collection<int32_t> neededShifts) {
    OpenBitSetPtr bits = useBitSet ? newLucene<OpenBitSet>((int32_t)(upper - lower + 1)) : OpenBitSetPtr();
    NumericUtils::splitIntRange(newLucene<CheckIntRangeBuilder>(lower, upper, bits, neededBounds.begin(), neededBounds.end(), neededShifts.begin(), neededShifts.end()), precisionStep, lower, upper);

    if (useBitSet) {
        // after flipping all bits in the range, the cardinality should be zero
        bits->flip(0, (int32_t)(upper - lower + 1));
        EXPECT_TRUE(bits->isEmpty());
    }
}

TEST_F(NumericUtilsTest, testLongConversionAndOrdering) {
    // generate a series of encoded longs, each numerical one bigger than the one before
    String last;
    for (int64_t l = -100000; l < 100000; ++l) {
        String act = NumericUtils::longToPrefixCoded(l);
        if (!last.empty()) {
            // test if smaller
            if (last.compare(act) >= 0) {
                FAIL() << "compare failure";
            }
        }
        // test is back and forward conversion works
        EXPECT_EQ(l, NumericUtils::prefixCodedToLong(act));
        // next step
        last = act;
    }
}

TEST_F(NumericUtilsTest, testIntConversionAndOrdering) {
    // generate a series of encoded ints, each numerical one bigger than the one before
    String last;
    for (int32_t l = -100000; l < 100000; ++l) {
        String act = NumericUtils::intToPrefixCoded(l);
        if (!last.empty()) {
            // test if smaller
            if (last.compare(act) >= 0) {
                FAIL() << "compare failure";
            }
        }
        // test is back and forward conversion works
        EXPECT_EQ(l, NumericUtils::prefixCodedToInt(act));
        // next step
        last = act;
    }
}

TEST_F(NumericUtilsTest, testLongSpecialValues) {
    static const int64_t vals[] = {LLONG_MIN, LLONG_MIN + 1, LLONG_MIN + 2, -5003400000000LL, -4000LL, -3000LL,
                                   -2000LL, -1000LL, -1LL, 0LL, 1LL, 10LL, 300LL, 50006789999999999LL,
                                   LLONG_MAX - 2, LLONG_MAX - 1, LLONG_MAX
                                  };
    int32_t length = SIZEOF_ARRAY(vals);
    Collection<String> prefixVals = Collection<String>::newInstance(length);
    for (int32_t i = 0; i < length; ++i) {
        prefixVals[i] = NumericUtils::longToPrefixCoded(vals[i]);

        // check forward and back conversion
        EXPECT_EQ(vals[i], NumericUtils::prefixCodedToLong(prefixVals[i]));

        // test if decoding values as long fails correctly
        try {
            NumericUtils::prefixCodedToInt(prefixVals[i]);
        } catch (NumberFormatException& e) {
            EXPECT_TRUE(check_exception(LuceneException::NumberFormat)(e));
        }
    }

    // check sort order (prefixVals should be ascending)
    for (int32_t i = 1; i < prefixVals.size(); ++i) {
        EXPECT_TRUE(prefixVals[i - 1].compare(prefixVals[i]) < 0);
    }

    // check the prefix encoding, lower precision should have the difference to original
    // value equal to the lower removed bits
    for (int32_t i = 0; i < length; ++i) {
        for (int32_t j = 0; j < 32; ++j) {
            int64_t prefixVal = NumericUtils::prefixCodedToLong(NumericUtils::longToPrefixCoded(vals[i], j));
            int64_t mask = ((int64_t)1 << j) - 1;
            EXPECT_EQ(vals[i] & mask, vals[i] - prefixVal);
        }
    }
}

TEST_F(NumericUtilsTest, testIntSpecialValues) {
    static const int32_t vals[] = {INT_MIN, INT_MIN + 1, INT_MIN + 2, -64765767, -4000, -3000, -2000,
                                   -1000, -1, 0, 1, 10, 300, 765878989, INT_MAX - 2, INT_MAX- 1, INT_MAX
                                  };
    int32_t length = SIZEOF_ARRAY(vals);
    Collection<String> prefixVals = Collection<String>::newInstance(length);
    for (int32_t i = 0; i < length; ++i) {
        prefixVals[i] = NumericUtils::intToPrefixCoded(vals[i]);

        // check forward and back conversion
        EXPECT_EQ(vals[i], NumericUtils::prefixCodedToInt(prefixVals[i]));

        // test if decoding values as long fails correctly
        try {
            NumericUtils::prefixCodedToLong(prefixVals[i]);
        } catch (NumberFormatException& e) {
            EXPECT_TRUE(check_exception(LuceneException::NumberFormat)(e));
        }
    }

    // check sort order (prefixVals should be ascending)
    for (int32_t i = 1; i < prefixVals.size(); ++i) {
        EXPECT_TRUE(prefixVals[i - 1].compare(prefixVals[i]) < 0);
    }

    // check the prefix encoding, lower precision should have the difference to original
    // value equal to the lower removed bits
    for (int32_t i = 0; i < length; ++i) {
        for (int32_t j = 0; j < 32; ++j) {
            int32_t prefixVal = NumericUtils::prefixCodedToInt(NumericUtils::intToPrefixCoded(vals[i], j));
            int32_t mask = ((int32_t)1 << j) - 1;
            EXPECT_EQ(vals[i] & mask, vals[i] - prefixVal);
        }
    }
}

TEST_F(NumericUtilsTest, testDoubles) {
    static const double vals[] = {-std::numeric_limits<double>::infinity(), -2.3E25, -1.0E15, -1.0,
                                  -1.0E-1, -1.0E-2, -0.0, +0.0, 1.0E-2, 1.0E-1, 1.0, 1.0E15, 2.3E25,
                                  std::numeric_limits<double>::infinity()
                                 };
    int32_t length = SIZEOF_ARRAY(vals);
    Collection<int64_t> longVals = Collection<int64_t>::newInstance(length);

    // check forward and back conversion
    for (int32_t i = 0; i < length; ++i) {
        longVals[i] = NumericUtils::doubleToSortableLong(vals[i]);
        EXPECT_EQ(vals[i], NumericUtils::sortableLongToDouble(longVals[i]));
    }

    // check sort order (longVals should be ascending)
    for (int32_t i = 1; i < longVals.size(); ++i) {
        EXPECT_TRUE(longVals[i - 1] < longVals[i]);
    }
}

/// NumericRangeQuery errors with endpoints near long min and max values
TEST_F(NumericUtilsTest, testLongExtremeValues) {
    // upper end extremes
    checkLongRangeSplit(LLONG_MAX, LLONG_MAX, 1, true,
                        newCollection<int64_t>(0xffffffffffffffffLL, 0xffffffffffffffffLL),
                        newCollection<int32_t>(0)
                       );
    checkLongRangeSplit(LLONG_MAX, LLONG_MAX, 2, true,
                        newCollection<int64_t>(0xffffffffffffffffLL, 0xffffffffffffffffLL),
                        newCollection<int32_t>(0)
                       );
    checkLongRangeSplit(LLONG_MAX, LLONG_MAX, 4, true,
                        newCollection<int64_t>(0xffffffffffffffffLL, 0xffffffffffffffffLL),
                        newCollection<int32_t>(0)
                       );
    checkLongRangeSplit(LLONG_MAX, LLONG_MAX, 6, true,
                        newCollection<int64_t>(0xffffffffffffffffLL, 0xffffffffffffffffLL),
                        newCollection<int32_t>(0)
                       );
    checkLongRangeSplit(LLONG_MAX, LLONG_MAX, 8, true,
                        newCollection<int64_t>(0xffffffffffffffffLL ,0xffffffffffffffffLL),
                        newCollection<int32_t>(0)
                       );
    checkLongRangeSplit(LLONG_MAX, LLONG_MAX, 64, true,
                        newCollection<int64_t>(0xffffffffffffffffLL, 0xffffffffffffffffLL),
                        newCollection<int32_t>(0)
                       );

    checkLongRangeSplit(LLONG_MAX - 0xfLL, LLONG_MAX, 4, true,
                        newCollection<int64_t>(0xfffffffffffffffLL, 0xfffffffffffffffLL),
                        newCollection<int32_t>(4)
                       );
    checkLongRangeSplit(LLONG_MAX - 0x10LL, LLONG_MAX, 4, true,
                        newCollection<int64_t>(0xffffffffffffffefLL, 0xffffffffffffffefLL, 0xfffffffffffffffLL, 0xfffffffffffffffLL),
                        newCollection<int32_t>(0, 4)
                       );

    // lower end extremes
    checkLongRangeSplit(LLONG_MIN, LLONG_MIN, 1, true,
                        newCollection<int64_t>(0x0000000000000000LL,0x0000000000000000LL),
                        newCollection<int32_t>(0)
                       );
    checkLongRangeSplit(LLONG_MIN, LLONG_MIN, 2, true,
                        newCollection<int64_t>(0x0000000000000000LL, 0x0000000000000000LL),
                        newCollection<int32_t>(0)
                       );
    checkLongRangeSplit(LLONG_MIN, LLONG_MIN, 4, true,
                        newCollection<int64_t>(0x0000000000000000LL, 0x0000000000000000LL),
                        newCollection<int32_t>(0)
                       );
    checkLongRangeSplit(LLONG_MIN, LLONG_MIN, 6, true,
                        newCollection<int64_t>(0x0000000000000000LL, 0x0000000000000000LL),
                        newCollection<int32_t>(0)
                       );
    checkLongRangeSplit(LLONG_MIN, LLONG_MIN, 8, true,
                        newCollection<int64_t>(0x0000000000000000LL, 0x0000000000000000LL),
                        newCollection<int32_t>(0)
                       );
    checkLongRangeSplit(LLONG_MIN, LLONG_MIN, 64, true,
                        newCollection<int64_t>(0x0000000000000000LL, 0x0000000000000000LL),
                        newCollection<int32_t>(0)
                       );

    checkLongRangeSplit(LLONG_MIN, LLONG_MIN + 0xfLL, 4, true,
                        newCollection<int64_t>(0x000000000000000LL, 0x000000000000000LL),
                        newCollection<int32_t>(4)
                       );
    checkLongRangeSplit(LLONG_MIN, LLONG_MIN + 0x10LL, 4, true,
                        newCollection<int64_t>(0x0000000000000010LL, 0x0000000000000010LL, 0x000000000000000LL, 0x000000000000000LL),
                        newCollection<int32_t>(0, 4)
                       );
}

static int64_t randomLong(const RandomPtr& random) {
    int64_t val;
    switch (random->nextInt(4)) {
    case 0:
        val = 1LL << (int64_t)random->nextInt(63); //  patterns like 0x000000100000 (-1 yields patterns like 0x0000fff)
        break;
    case 1:
        val = -1LL << (int64_t)random->nextInt(63); // patterns like 0xfffff00000
        break;
    default:
        val = (int64_t)random->nextInt();
    }

    val += random->nextInt(5) - 2;

    if (random->nextInt() % 2 == 1) {
        if (random->nextInt() % 2 == 1) {
            val += random->nextInt(100) - 50;
        }
        if (random->nextInt() % 2 == 1) {
            val = ~val;
        }
        if (random->nextInt() % 2 == 1) {
            val = val << 1;
        }
        if (random->nextInt() % 2 == 1) {
            val = MiscUtils::unsignedShift(val, (int64_t)1);
        }
    }

    return val;
}

static void executeOneRandomSplit(const RandomPtr& random) {
    int64_t lower = randomLong(random);
    int64_t len = (int64_t)random->nextInt(16384 * 1024); // not too large bitsets, else OOME!
    while (lower + len < lower) { // overflow
        lower >>= 1;
    }
    checkLongRangeSplit(lower, lower + len, random->nextInt(64) + 1, true, Collection<int64_t>::newInstance(), Collection<int32_t>::newInstance());
}

TEST_F(NumericUtilsTest, testRandomSplit) {
    RandomPtr random = newLucene<Random>(123);
    for (int32_t i = 0; i < 20; ++i) {
        executeOneRandomSplit(random);
    }
}

TEST_F(NumericUtilsTest, testSplitLongRange) {
    Collection<int64_t> neededBounds = Collection<int64_t>::newInstance(14);
    neededBounds[0] = 0x7fffffffffffec78LL;
    neededBounds[1] = 0x7fffffffffffec7fLL;
    neededBounds[2] = 0x8000000000002510LL;
    neededBounds[3] = 0x800000000000251cLL;
    neededBounds[4] = 0x7fffffffffffec8LL;
    neededBounds[5] = 0x7fffffffffffecfLL;
    neededBounds[6] = 0x800000000000250LL;
    neededBounds[7] = 0x800000000000250LL;
    neededBounds[8] = 0x7fffffffffffedLL;
    neededBounds[9] = 0x7fffffffffffefLL;
    neededBounds[10] = 0x80000000000020LL;
    neededBounds[11] = 0x80000000000024LL;
    neededBounds[12] = 0x7ffffffffffffLL;
    neededBounds[13] = 0x8000000000001LL;

    // a hard-coded "standard" range
    checkLongRangeSplit(-5000, 9500, 4, true, neededBounds, newCollection<int32_t>(0, 0, 4, 4, 8, 8, 12));

    // the same with no range splitting
    checkLongRangeSplit(-5000, 9500, 64, true,
                        newCollection<int64_t>(0x7fffffffffffec78LL, 0x800000000000251cLL),
                        newCollection<int32_t>(0)
                       );

    // this tests optimized range splitting, if one of the inner bounds
    // is also the bound of the next lower precision, it should be used completely
    checkLongRangeSplit(0, 1024 + 63, 4, true,
                        newCollection<int64_t>(0x800000000000040LL, 0x800000000000043LL, 0x80000000000000LL, 0x80000000000003LL),
                        newCollection<int32_t>(4, 8)
                       );

    // the full long range should only consist of a lowest precision range;
    // no bitset testing here, as too much memory needed
    checkLongRangeSplit(LLONG_MIN, LLONG_MAX, 8, false,
                        newCollection<int64_t>(0x00LL, 0xffLL),
                        newCollection<int32_t>(56)
                       );

    // the same with precisionStep=4
    checkLongRangeSplit(LLONG_MIN, LLONG_MAX, 4, false,
                        newCollection<int64_t>(0x00LL, 0xfLL),
                        newCollection<int32_t>(60)
                       );

    // the same with precisionStep=2
    checkLongRangeSplit(LLONG_MIN, LLONG_MAX, 2, false,
                        newCollection<int64_t>(0x00LL, 0x3LL),
                        newCollection<int32_t>(62)
                       );

    // the same with precisionStep=1
    checkLongRangeSplit(LLONG_MIN, LLONG_MAX, 1, false,
                        newCollection<int64_t>(0x00LL, 0x1LL),
                        newCollection<int32_t>(63)
                       );

    // a inverse range should produce no sub-ranges
    checkLongRangeSplit(9500, -5000, 4, false,  Collection<int64_t>::newInstance(), Collection<int32_t>::newInstance());

    // a 0-length range should reproduce the range itself
    checkLongRangeSplit(9500, 9500, 4, false,
                        newCollection<int64_t>(0x800000000000251cLL, 0x800000000000251cLL),
                        newCollection<int32_t>(0)
                       );
}

TEST_F(NumericUtilsTest, testSplitIntRange) {
    Collection<int32_t> neededBounds = Collection<int32_t>::newInstance(14);
    neededBounds[0] = 0x7fffec78;
    neededBounds[1] = 0x7fffec7f;
    neededBounds[2] = 0x80002510;
    neededBounds[3] = 0x8000251c;
    neededBounds[4] = 0x7fffec8;
    neededBounds[5] = 0x7fffecf;
    neededBounds[6] = 0x8000250;
    neededBounds[7] = 0x8000250;
    neededBounds[8] = 0x7fffed;
    neededBounds[9] = 0x7fffef;
    neededBounds[10] = 0x800020;
    neededBounds[11] = 0x800024;
    neededBounds[12] = 0x7ffff;
    neededBounds[13] = 0x80001;

    // a hard-coded "standard" range
    checkIntRangeSplit(-5000, 9500, 4, true, neededBounds, newCollection<int32_t>(0, 0, 4, 4, 8, 8, 12));

    // the same with no range splitting
    checkIntRangeSplit(-5000, 9500, 32, true,
                       newCollection<int32_t>(0x7fffec78, 0x8000251c),
                       newCollection<int32_t>(0)
                      );

    // this tests optimized range splitting, if one of the inner bounds
    // is also the bound of the next lower precision, it should be used completely
    checkIntRangeSplit(0, 1024 + 63, 4, true,
                       newCollection<int32_t>(0x8000040, 0x8000043, 0x800000, 0x800003),
                       newCollection<int32_t>(4, 8)
                      );

    // the full int range should only consist of a lowest precision range;
    // no bitset testing here, as too much memory needed
    checkIntRangeSplit(INT_MIN, INT_MAX, 8, false,
                       newCollection<int32_t>(0x00, 0xff),
                       newCollection<int32_t>(24)
                      );

    // the same with precisionStep=4
    checkIntRangeSplit(INT_MIN, INT_MAX, 4, false,
                       newCollection<int32_t>(0x00, 0xf),
                       newCollection<int32_t>(28)
                      );

    // the same with precisionStep=2
    checkIntRangeSplit(INT_MIN, INT_MAX, 2, false,
                       newCollection<int32_t>(0x00, 0x3),
                       newCollection<int32_t>(30)
                      );

    // the same with precisionStep=1
    checkIntRangeSplit(INT_MIN, INT_MAX, 1, false,
                       newCollection<int32_t>(0x00, 0x1),
                       newCollection<int32_t>(31)
                      );

    // a inverse range should produce no sub-ranges
    checkIntRangeSplit(9500, -5000, 4, false, Collection<int32_t>::newInstance(), Collection<int32_t>::newInstance());

    // a 0-length range should reproduce the range itself
    checkIntRangeSplit(9500, 9500, 4, false,
                       newCollection<int32_t>(0x8000251c, 0x8000251c),
                       newCollection<int32_t>(0)
                      );
}
