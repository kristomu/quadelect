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

TEST(SchulzeSTV, A34Result) {
	// Tests the election given in Schulze's A34.dat against
	// the desired outcome specified in his Schulze STV paper,
	// Part 3 of 5, "Implementing the Schulze STV method".
	// https://sites.math.duke.edu/~bray/Courses/49s/Additional%20Reading/Schulze/Schulze3/schulze3.pdf

	SchulzeSTV SSTV;

	std::vector<std::string> election = {
		"1: I > H > D > E",
		"1: I > L > A > N > D > J",
		"1: N > A > B > J > E > G > K > M > C > D > L",
		"1: B > G > J > F",
		"1: G > F",
		"1: G > K > J > A > E > F > N > B > H > D > L > C",
		"1: G > H > J > F > L > N > D > K",
		"1: K > D > J > C > M > H > N > B > L > G > F > A",
		"1: K > C > G > J > D > M > B > A > F > N > H > L",
		"1: K > A > M > L > I > J > F > E > D > C > B > G",
		"1: K > N > L > J > A > F > G > H > D",
		"1: N > C > D > E > L > M > F > I",
		"1: B > C > K > G > H > N > I > M",
		"1: C > B > G > K > F > E > N > A > L > H > D > J",
		"1: C > H > L > A > M > B > N > D > K > J > G > F",
		"1: C > K > D > E > H > L > J > M",
		"1: C > K > J > M > D > A > B > E > N > L > G > H > F",
		"1: E > B > A > H > L > J",
		"1: E > D > A > B > F > L > J > H > M > N > C > G",
		"1: E > H > F > G > J > L > A > D > K",
		"1: B > F > H > D > E > J > L > N > G > A > K > C",
		"1: B > F > N > J > D > A > H > M > E > L",
		"1: F > B > J > H > G > K > L > A > N > C",
		"1: F > N > H > D > E > L > A",
		"1: F > K > N > G > A > M > H > B > C > J > L > D > E > I",
		"1: E > J > M > L > B > F > D > K > G > A > H",
		"1: J > H > M > D > A > G > N > K > F > L > E > B > C",
		"1: M > H > L > E > N > D",
		"1: M > L > H > D",
		"1: M > J > F > D > N > H > K > C > A > G > B > L > E",
		"1: M > J > F > D > N > H > K > C > A > G > B > L > E",
		"1: N > L > M > J > E > D > F > B > K > H > G > A > C > I",
		"1: N > H > J > B > A > L > K > F > E > G > C > D",
		"1: N > L > B > E > H > J > A > K > F > M > D > C",
		"1: E > H > N > B > L > F > D",
		"1: E > L > F > B > A > D > G > J > K > I > H > C > N > M",
		"1: E > H > N > B > L > D > F",
		"1: J > L > H > M > C > G > F > E > D > K > N > A",
		"1: J > B > N > L > H > F > E > K > M > A > D > C > G > I",
		"1: B > E > L > M > N > H > A > G > J > K > C > F",
		"1: L > J > M > D > A > K > N > F",
		"1: L > J > F > M > K > C > B > A > D > N > H",
		"1: J > A > H > K > G > F > D > M > C > N > B > L > E > I",
		"1: J > B > H > A > D > E > K",
		"1: J > N > A > D > E > H > L > F > M > B > C > G > I > K",
		"1: B > J > A > E > N > D > G > I > L > C > F",
		"1: A > L > B > J > K > M > C > D > F > G > H > N",
		"1: A > G > F > J > K > B > I > D > H > L",
		"1: A > B > M > D > C > J > I > E > K > L > N > H > F > G",
		"1: E > D > L > J > B",
		"1: J > D > A > F > H > N > K > L > M > B > G > C",
		"1: J > D > A > B > N > E > F > M > H > L",
		"1: B > D > E > A > I > J > L > N",
		"1: B > D > J > M > N > A > E > K > L > G > C > H > F",
		"1: D > H > L > E > B > J",
		"1: D > J > M > I > E > L > H > B > A > N > K > G > F > C",
		"1: D > J > A > N > K > F > B > E > H > G > M > L",
		"1: H > J > E > A > F > G > K > D > N > M > C > L > B > I",
		"1: H > N > M > K > G > L > D > J > A > C > B > F",
		"1: H > J > N > D > A > K > F > E > L > G > B",
		"1: H > J > E > F > K > G > C > L > M > D",
		"1: H > F > G > E > K > M > C",
		"1: B"
	};

	std::vector<std::string> desired_outcome =
	{"A", "B", "C", "D", "E", "F", "G", "H", "J", "K", "M", "N"};

	size_t num_seats = desired_outcome.size();

	EXPECT_EQ(
		interpret_ballots_get_outcome(&SSTV, num_seats,
			election),
		desired_outcome);
}

TEST(SchulzeSTV, A68ResultUncompressed) {
	// I may replace this when/if I make tests for the
	// rank-order interpreter and the uncompress tool,
	// although this verbose uncompressed style is still
	// useful for detecting bugs in how my Schulze STV
	// shunt *uses* ballot decompression.

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

TEST(SchulzeSTV, FRVMPRSTVp38Result) {
	// https://sites.math.duke.edu/%7Ebray/Courses/49s/Additional%20Reading/Schulze/schulze2.pdf p. 38

	SchulzeSTV SSTV;

	std::vector<std::string> election = {
		"60: A > B > C > D > E",
		"45: A > C > E > B > D",
		"30: A > D > B > E > C",
		"15: A > E > D > C > B",
		"12: B > A > E > D > C",
		"48: B > C > D > E > A",
		"39: B > D > A > C > E",
		"21: B > E > C > A > D",
		"27: C > A > D > B > E",
		" 9: C > B > A > E > D",
		"51: C > D > E > A > B",
		"33: C > E > B > D > A",
		"42: D > A > C > E > B",
		"18: D > B > E > C > A",
		" 6: D > C > B > A > E",
		"54: D > E > A > B > C",
		"57: E > A > B > C > D",
		"36: E > B > D > A > C",
		"24: E > C > A > D > B",
		" 3: E > D > C > B > A"
	};

	std::vector<std::string> desired_outcome =
	{"A", "D", "E"};

	size_t num_seats = desired_outcome.size();

	EXPECT_EQ(
		interpret_ballots_get_outcome(&SSTV, num_seats,
			election),
		desired_outcome);
}

TEST(SchulzeSTV, A04Result) {
	// The A04.dat election.

	SchulzeSTV SSTV;

	std::vector<std::string> election = {
		"1: A > B",
		"1: A > B > F",
		"1: A > B > I",
		"1: A > C > N",
		"1: A > H > B",
		"1: A > I > B > H",
		"1: A > I > L > M",
		"2: B > A",
		"1: B > E > I > A > J > K",
		"1: C > F > I",
		"1: C > F > M > I",
		"1: C > I > F > K",
		"1: C > I > K",
		"1: C > I > K > J > N > G > H > M",
		"1: D > E > C > K > I",
		"1: D > F > I > K > C",
		"1: D > H > F > M > N > K > A > B > C > E > I > J > L > G",
		"1: E > F > B > M",
		"1: E > I > N > K > H > C > L > J > A > B > G > F > M > D",
		"1: E > J > I > N > F",
		"1: F > A > I > E > C > K",
		"1: F > I > K",
		"1: F > I > K > C > B > A",
		"1: G > I > H",
		"1: G > I > J",
		"1: G > K > I > F > E > H > A > C > L > D > M > B > N > J",
		"1: H > M > K",
		"3: I > K",
		"1: I > K > F",
		"1: I > K > F > E > A",
		"1: I > K > F > E > C",
		"1: I > K > H > E > A > B > F",
		"1: I > M > F > N > J > B",
		"1: J > E > I",
		"1: K > H > E > I > L",
		"1: K > I > A > G > J > C > F > H > M > B > E > N > L > D",
		"1: K > I > F > A > M > N > L > J > H > G > E > D > C > B",
		"1: K > N > E",
		"1: L > N > E",
		"1: M > I > B > F > J > E > H"
	};

	std::vector<std::string> desired_outcome =
	{"A", "I"};

	size_t num_seats = desired_outcome.size();

	EXPECT_EQ(
		interpret_ballots_get_outcome(&SSTV, num_seats,
			election),
		desired_outcome);
}