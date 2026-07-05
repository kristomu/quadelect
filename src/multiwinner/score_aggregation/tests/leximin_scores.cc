// Leximin score tests

#include <vector>

#include <gtest/gtest.h>

#include "multiwinner/score_aggregation/leximin_scores.h"

TEST(LeximinScores, NonemptyGreaterThanEmpty) {
	leximin_scores a, b;

	a.insert(1, 3.2);

	EXPECT_GT(a, b);
}

TEST(LeximinScores, EqualToSelf) {
	leximin_scores a, b;

	a.insert(1, 3.2);

	EXPECT_EQ(a, a);
	EXPECT_EQ(b, b);
	EXPECT_NE(a, b);
}

TEST(LeximinScores, EqualWithSameEntries) {
	leximin_scores a, b;

	a.insert(1, 3.2);
	b.insert(1, 3.2);

	EXPECT_EQ(a, b);
}

TEST(LeximinScores, SameWeightDifferentScoreGreater) {
	leximin_scores a, b;

	a.insert(1, 3.2);
	b.insert(1, 1.4);

	EXPECT_GT(a, b);
	EXPECT_LE(b, a);
}

TEST(LeximinScores, LargerShareGreaterScoreIsGreater) {
	leximin_scores a, b;

	a.insert(2, 3.2);
	a.insert(1, 4.4);

	b.insert(1, 3.2);
	b.insert(2, 4.4);

	EXPECT_GT(b, a);
}

TEST(LeximinScores, TwoStageInsertEqualsOneStage) {
	leximin_scores a, b;

	a.insert(2, 3.2);

	b.insert(1, 3.2);
	b.insert(1, 3.2);

	EXPECT_EQ(a, b);
}

TEST(LeximinScores, GreaterWeightGreaterAtEnd) {

	leximin_scores a, b;

	a.insert(100, 3.2);
	b.insert(1, 3.2);

	EXPECT_GT(a, b);
}

TEST(LeximinScores, TestAllComparisonOperators) {
	// Assuming the previous tests are OK, check that every
	// comparison operator does the right thing.

	leximin_scores a, b;

	a.insert(1, 3.2);

	EXPECT_LT(b, a);

	EXPECT_LE(b, a);
	EXPECT_LE(a, a);

	EXPECT_GT(a, b);

	EXPECT_GE(a, b);
	EXPECT_GE(a, a);

	EXPECT_EQ(a, a);
	EXPECT_EQ(b, b);

	EXPECT_NE(a, b);
	EXPECT_NE(b, a);
}