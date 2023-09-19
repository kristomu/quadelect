// "IFPP Method X". Each candidate's score is his value A>B after eliminating
// all other candidates but B in a number of rounds. A will try to structure
// the elimination so as to maximize this value.

// The elimination rules are:
//		- In each round, a single candidate may be eliminated as long as
//			this candidate's first preferences do not exceed 1/n of
//			the total number of first preferences, where n is the
//			number of remaining candidates.
//		- If every candidate but A is eliminated this way, then A's
//			score is the number of non-exhausted voters.
//		- If all but one other is eliminated, then A's score is A>B.
//		- If A is forced to be eliminated, then A's score is zero.

// The candidate with the maximum score through this process wins.
// The method is very slow and may be better set as experimental...

#pragma once

#include "../method.h"
#include "../positional/simple_methods.h"

class ifpp_method_x : public election_method {

	private:
		plurality plurality_method;

		void find_eliminated_plurality_counts(
			std::map<std::vector<bool>, std::vector<double> > & first_pref_counts,
			const election_t & election,
			std::vector<bool> & remaining_candidates,
			int num_remaining) const;

		std::map<std::vector<bool>, std::vector<double> >
		get_eliminated_plurality_counts(
			const election_t & election,
			std::vector<bool> hopefuls) const;

		std::vector<double> find_candidate_scores(
			const std::map<std::vector<bool>, std::vector<double> > &
			first_pref_counts,
			std::map<std::vector<bool>, std::vector<double> > & score_cache,
			std::vector<bool> & remaining_candidates) const;

		std::vector<double> get_candidate_scores(
			const election_t & election,
			std::vector<bool> hopefuls, int num_candidates) const;

	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		virtual std::string name() const {
			return "IFPP Method X";
		}

		// Since we manually count the total number of first
		// preferences each time to determine the number of non-
		// exhaused ballots, any setting should work here: whole or
		// fractional. I haven't tested the method with equal rank,
		// though.
		ifpp_method_x() : plurality_method(PT_WHOLE) {}
};
