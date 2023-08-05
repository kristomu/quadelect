#pragma once

// Kevin Venzke's No elimination IRV methods, referenced on
// https://www.votingmethods.net/misc/

// In each round, for each voter, go down the ballots until
// we encounter a candidate who has not been marked "eliminated" yet.
// Count every candidate with a score no less than this candidate as
// approved.

// If, in a round, any candidate has an approval from more than half
// the voters, then that candidate is elected. Otherwise, the loser
// among non-eliminated candidates is marked eliminated and the next
// round begins.

// There are two exceptions for type 1, and one for type 2. If every
// candidate is eliminated, then the highest count candidate wins
// (tie for first if there are more than one). This is common to both.
// If only one uneliminated candidate remains, then that candidate is
// the sole winner (type 1 only).

// Note that type 2 will produce a full n-way tie if everybody uses
// full ballots.

// Currently only type 1. FIX LATER

// TODO?? "scored_method" subclass that just asks for get_scores
// and does the translation into an ordering automatically? Even more
// relevant if I implement is_scored() so that quadelect's interpreter
// mode can print scores... or if I want to do some kind of gradient
// Yee thing.

#include "method.h"

enum nirv_type { NE_IRV_TYPE_ONE, NE_IRV_TYPE_TWO };

class no_elimination_irv : public election_method {

	private:
		std::vector<double> get_scores(
			const std::list<ballot_group> & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates) const;

	public:
		std::pair<ordering, bool> elect_inner(
			const std::list<ballot_group> & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string name() const {
			return ("No-elimination IRV");
		}
};