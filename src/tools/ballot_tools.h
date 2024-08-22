#pragma once

#include <iostream>
#include <list>
#include "common/ballots.h"
#include "tools.h"

// Tools for managing ballots and orderings

class ordering_sorter {
	public:
		int compare(const ordering & a, const ordering & b) const;
		bool operator()(const ordering & a, const ordering & b) const;
		bool operator()(const ballot_group & a, const
			ballot_group & b) const;
};

// Orderings:

class ordering_tools {

	public:
		ordering_sorter sorter;

		// Return a reverse ordering (winner becomes loser etc)
		static ordering reverse(const ordering & in);
		// Return an ordering where all but the winners have been
		// removed.
		ordering winner_only(const ordering & in) const;
		// Turn the ordering into one with only rank data (winner has 0
		// pts, second place has -1, and so on).
		ordering scrub_scores(const ordering & in) const;
		// As above, but ties count, so A > B = C > D gives D -3 pts,
		// not -2.
		ordering scrub_scores_by_cand(const ordering & in) const;

		static bool has_multiple_winners(const ordering & in);
		static std::vector<int> get_winners(const ordering & in);
		static bool is_winner(const ordering & in, int candidate_num);

		// Checks if the ordering has any explicitly equal-ranked
		// candidates. NOTE: Does not check truncation! TODO?
		static bool has_some_equal_rank(const ordering & in);

		static std::list<candscore> get_loser_candscores(const ordering & in);

		// Break ties in "tied" according to tiebreaker and return
		// result.
		ordering tiebreak(const ordering & tied,
			const ordering & tiebreaker,
			size_t num_candidates) const;

		// Used for testing. Or should be, anyway.
		static ordering ranked_tiebreak(const ordering & tied,
			const ordering & tiebreaker,
			size_t num_candidates);

		// Returns true if everybody who is ranked is also tied.
		bool all_ranked_equal(const ordering & to_check) const;

		static std::string ordering_to_text(const ordering & rank_ballot,
			const std::map<size_t, std::string> & reverse_cand_lookup,
			bool numeric);

		static std::string ordering_to_text(const ordering & rank_ballot,
			bool numeric);

		ordering direct_vector_to_ordering(const std::vector<double> & in,
			const std::vector<bool> & hopefuls) const;

		ordering indirect_vector_to_ordering(const std::vector<double> & in,
			const std::vector<int> & mapping) const;

};

///// Ballots and elections.

class ballot_tools {
	private:
		ordering_tools otools;

		void print_ballots(const election_t & ballots,
			bool rated) const;

	public:
		// Truncates an ordering so that everybody ranked below
		// the given candidate num will be omitted from the ordering.
		// This is used by Adjusted Condorcet Plurality and for
		// experimentation.
		static ballot_group truncate_after(const ballot_group & in,
			size_t truncate_after_candidate_num);

		static election_t truncate_after(
			const election_t & ballots,
			size_t truncate_after_candidate_num);

		static election_t reverse(const election_t & ballots);

		election_t sort_ballots(const election_t &
			to_sort) const;
		election_t compress(const election_t &
			uncompressed) const;

		std::string ballot_to_text(const ballot_group & rank_ballot,
			const std::map<size_t, std::string> & reverse_cand_lookup,
			bool numeric) const;

		std::vector<std::string> ballots_to_text(std::string prefix,
			const election_t & rank_ballots,
			const std::map<size_t, std::string> & reverse_cand_lokoup,
			bool numeric) const;

		std::vector<std::string> ballots_to_text(const election_t &
			rank_ballots, const std::map<size_t, std::string> &
			reverse_cand_lookup, bool numeric) const;

		void print_ranked_ballots(const election_t &
			rank_ballots) const;

		void print_rated_ballots(const election_t &
			rated_ballots) const;

		static double get_num_voters(const election_t &
			rank_ballots);

		// Multiply all the weights by factor. Used for covering
		// methods more thoroughly when doing monotonicity checks etc.
		election_t rescale(const election_t & ballots,
			double factor) const;

};

// These templated functions find the first iterator that refers to
// a hopeful candidate. The templating lets us deal with iterators,
// reverse iterators, const iterators, etc., but comes with the cost
// that the functions must be defined outside of a class.

template<typename T> T first_hopeful(T cur, T end,
	const std::vector<bool> & hopefuls) {

	while (cur != end && !hopefuls[cur->get_candidate_num()]) {
		if (++cur == end) {
			return (cur);
		}
	}

	return cur;
}

template<typename T> T next_hopeful(T cur, T end,
	const std::vector<bool> & hopefuls) {
	++cur;

	return first_hopeful(cur, end, hopefuls);
}