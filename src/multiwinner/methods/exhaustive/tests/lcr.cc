// Test a variety of council-optimizing methods against a
// Left-Center-Right example with Borda count values.

// These should all return Center when a single winner is requested,
// {Left, Right} with two seats, and Center and somebody else when
// using sequential voting.

#include <vector>
#include <iostream>

#include <gtest/gtest.h>

#include "multiwinner/methods/exhaustive/all.h"
#include "interpreter/rank_order.h"

// Helper function. TODO: Put into an appropriate tools file.

// quick and dirty for rated ballots. Using pairs makes it
// easier to specify ballots by nested braces. FIX LATER by making
// an appropriate interpreter/tokenizer for rated ballots.
ballot_group get_ballot_group(double weight,
	std::vector<std::pair<size_t, double> > candscores) {

	ballot_group out;
	out.set_weight(weight);

	for (auto cand_and_score: candscores) {
		out.contents.insert(candscore(cand_and_score.first,
				cand_and_score.second));
	}

	return out;
}

/*std::vector<std::string> interpret_ballots_get_outcome_x(
	const multiwinner_method * method, size_t num_seats,
	const std::vector<std::string> & ballots) {

	rank_order_int interpreter;

	// Returns a pair: the first is a map from candidate
	// names to integers, and the second is an election (ballot set).

	names_and_election interpreted_election =
		interpreter.interpret_ballots(ballots, false);

	size_t num_candidates = interpreted_election.first.size();

	council_t outcome = method->get_council(num_seats,
			num_candidates, interpreted_election.second);

	std::vector<std::string> winners;

	for (auto i: outcome) {
		winners.push_back(interpreted_election.first[i]);
	}

	// Sort the winners in alphabetical order.
	std::sort(winners.begin(), winners.end());

	return winners;
}*/

TEST(HarmonicVoting, SingleWinnerLCRElectsCenter) {
	harmonic_voting harmonic(1);

	// Candidate 0 is Center, 1 is Left, 2 is Right.

	std::vector<size_t> desired_outcome = {0};

	election_t lcr_election = {
		// Left > Center > Right
		get_ballot_group(42, { {1, 1.0}, {0, 0.5}, {2, 0} }),
		// Right > Center > Left
		get_ballot_group(38, { {2, 1.0}, {0, 0.5}, {1, 0} }),
		// Center > Right > Left
		get_ballot_group(10, { {0, 1.0}, {2, 0.5}, {1, 0} })
	};

	EXPECT_EQ(harmonic.get_council(1, 3, lcr_election),
		desired_outcome);

}

TEST(HarmonicVoting, TwoWinnerLCRElectsWings) {
	harmonic_voting harmonic(1);

	std::vector<size_t> desired_outcome = {1, 2};

	election_t lcr_election = {
		// Left > Center > Right
		get_ballot_group(42, { {1, 1.0}, {0, 0.5}, {2, 0} }),
		// Right > Center > Left
		get_ballot_group(38, { {2, 1.0}, {0, 0.5}, {1, 0} }),
		// Center > Right > Left
		get_ballot_group(10, { {0, 1.0}, {2, 0.5}, {1, 0} })
	};

	EXPECT_EQ(harmonic.get_council(2, 3, lcr_election),
		desired_outcome);
}

TEST(SequentialHarmonicVoting, TwoWinnerLCRHouseMonotone) {
	sequential_harmonic_voting s_harmonic(1);

	// We have to elect Center to satisfy house monotonicity.
	// With that given, the ballot set is symmetric in Left vs
	// Right except that Left has more support from voters not
	// already supporting Center, so we should expect the other
	// winner to be Left.
	std::vector<size_t> desired_outcome = {0, 1};

	election_t lcr_election = {
		// Left > Center > Right
		get_ballot_group(42, { {1, 1.0}, {0, 0.5}, {2, 0} }),
		// Right > Center > Left
		get_ballot_group(38, { {2, 1.0}, {0, 0.5}, {1, 0} }),
		// Center > Right > Left
		get_ballot_group(10, { {0, 1.0}, {2, 0.5}, {1, 0} })
	};

	EXPECT_EQ(s_harmonic.get_council(2, 3, lcr_election),
		desired_outcome);
}
