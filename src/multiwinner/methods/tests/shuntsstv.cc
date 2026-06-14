// Schulze STV tests

#include <vector>

#include <gtest/gtest.h>

#include "multiwinner/methods/shuntsstv.h"
#include "interpreter/rank_order.h"

// TODO: Remember to run multiple elections, one after another, to account
// for mutable state.
// Could perhaps also make a fuzzer that does that...

// Helper function
std::vector<std::string> interpret_ballots_get_outcome(
	const multiwinner_method * method, size_t num_seats,
	const std::vector<std::string> & ballots) {

	rank_order_int interpreter;

	// Returns a pair: the first is a map from candidate
	// names to integers, and the second is an election (ballot set).

	names_and_election unanimity =
		interpreter.interpret_ballots(ballots, false);

	size_t num_candidates = unanimity.first.size();

	council_t outcome = method->get_council(num_seats,
			num_candidates, unanimity.second);

	std::vector<std::string> winners;

	for (auto i: outcome) {
		winners.push_back(unanimity.first[i]);
	}

	// Sort the winners in alphabetical order.
	std::sort(winners.begin(), winners.end());

	return winners;
}

TEST(SchulzeSTV, SingleWinnerUnanimity) {
	SchulzeSTV SSTV;

	std::vector<std::string> desired_outcome = {"A"};

	EXPECT_EQ(
		interpret_ballots_get_outcome(&SSTV, 1,
	{ "1: A>B>C"}),
	desired_outcome);
}

TEST(SchulzeSTV, A33Result) {
	// Tests the election given in Schulze's a33.dat against
	// the desired outcome specified in his Schulze STV paper,
	// Part 3 of 5, "Implementing the Schulze STV method".
	// https://sites.math.duke.edu/~bray/Courses/49s/Additional%20Reading/Schulze/Schulze3/schulze3.pdf

	// This test is a bit slow.

	SchulzeSTV SSTV;

	std::vector<std::string> election = {
		"1: O>I>E>F>H ",
		"1: R>G>P>A>L>N",
		"1: N>I>Q>M>O>E>B>L>P>H",
		"1: F>E>C>D>O>L>I>G>A>N>M>P>R>Q>H>B>J>K",
		"1: L>A>Q>I>F>E>B",
		"1: O>H>E>C",
		"1: H>O>B>C>E>D>A>I",
		"1: C>E>A>D>K>B>H",
		"1: Q>D>I>N>P>F>A>M>H>O>C>R>E>L>K>J>B>G"
	};

	std::vector<std::string> desired_outcome =
	{"E", "N", "O"};

	size_t num_seats = desired_outcome.size();

	EXPECT_EQ(
		interpret_ballots_get_outcome(&SSTV, num_seats,
			election),
		desired_outcome);
}

TEST(SchulzeSTV, A68ResultUncompressed) {

	SchulzeSTV SSTV;

	std::vector<std::string> election = {
		"A",
		"A",
		"A",
		"A",
		"A > B > C > D",
		"A > B > C > D",
		"A > C",
		"A > C",
		"A > C > B > D",
		"A > C > B > D",
		"A > C > B > D",
		"A > C > D",
		"A > C > D",
		"A > C > D",
		"A > C > D > B",
		"A > C > D > B",
		"A > C > D > B",
		"A > C > D > B",
		"A > C > D > B",
		"A > D > B",
		"A > D > C > B",
		"A > D > C > B",
		"A > D > C > B",
		"A > D > C > B",
		"B",
		"B > A",
		"B > C > A",
		"C",
		"C",
		"C > A",
		"C > A > B > D",
		"C > A > D",
		"C > A > D",
		"C > A > D",
		"C > A > D",
		"C > A > D",
		"C > A > D",
		"C > A > D > B",
		"C > B > D",
		"C > B > D > A",
		"C > B > D > A",
		"C > D > A",
		"C > D > A > B",
		"C > D > A > B",
		"C > D > B > A",
		"D > A",
		"D > A > C",
		"D > A > C",
		"D > A > C > B",
		"D > A > C > B",
	};

	std::vector<std::string> desired_outcome =
	{"A", "C", "D"};

	size_t num_seats = desired_outcome.size();

	EXPECT_EQ(
		interpret_ballots_get_outcome(&SSTV, num_seats,
			election),
		desired_outcome);
}

TEST(SchulzeSTV, A68ResultCompressed) {
	SchulzeSTV SSTV;

	std::vector<std::string> election = {
		"4: A",
		"2: A > B > C > D",
		"2: A > C",
		"3: A > C > B > D",
		"3: A > C > D",
		"5: A > C > D > B",
		"1: A > D > B",
		"4: A > D > C > B",
		"1: B",
		"1: B > A",
		"1: B > C > A",
		"2: C",
		"1: C > A",
		"1: C > A > B > D",
		"6: C > A > D",
		"1: C > A > D > B",
		"1: C > B > D",
		"2: C > B > D > A",
		"1: C > D > A",
		"2: C > D > A > B",
		"1: C > D > B > A",
		"1: D > A",
		"2: D > A > C",
		"2: D > A > C > B"
	};

	std::vector<std::string> desired_outcome =
	{"A", "C", "D"};

	size_t num_seats = desired_outcome.size();

	EXPECT_EQ(
		interpret_ballots_get_outcome(&SSTV, num_seats,
			election),
		desired_outcome);
}