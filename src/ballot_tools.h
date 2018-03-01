#ifndef _VOTE_BTOOLS
#define _VOTE_BTOOLS

#include <iostream>
#include <list>
#include "ballots.h"
#include "tools.h"

using namespace std;

// Tools for managing ballots and orderings

class ordering_sorter {
	public:
		int compare (const ordering & a, const ordering & b) const;
		bool operator() (const ordering & a, const ordering & b) const;
		bool operator() (const ballot_group & a, const 
				ballot_group & b) const;
};

// Orderings:

class ordering_tools {

	public:
		ordering_sorter sorter;

		// Return a reverse ordering (winner becomes loser etc)
		ordering reverse(const ordering & in) const;
		// Return an ordering where all but the winners have been
		// removed.
		ordering winner_only(const ordering & in) const;
		// Turn the ordering into one with only rank data (winner has 0
		// pts, second place has -1, and so on).
		ordering scrub_scores(const ordering & in) const;
		// As above, but ties count, so A > B = C > D gives D -3 pts,
		// not -2.
		ordering scrub_scores_by_cand(const ordering & in) const;
		list<int> get_winners(const ordering & in) const;

		// Break ties in "tied" according to tiebreaker and return
		// result.
		ordering tiebreak(const ordering & tied, 
				const ordering & tiebreaker,
				int num_candidates) const;

		// Used for testing. Or should be, anyway.
		ordering ranked_tiebreak(const ordering & tied,
				const ordering & tiebreaker,
				int num_candidates) const;

		// Returns true if everybody who is ranked is also tied.
		bool has_equal_rank(const ordering & to_check) const;

		string ordering_to_text(const ordering & rank_ballot,
				const map<int, string> & reverse_cand_lookup,
				bool numeric) const;

		ordering direct_vector_to_ordering(const vector<double> & in,
				const vector<bool> & hopefuls) const;

		ordering indirect_vector_to_ordering(const vector<double> & in,
				const vector<int> & mapping) const;

};

///// Ballots.

class ballot_tools {
	private:
		ordering_tools otools;

	public:
		list<ballot_group> sort_ballots(const list<ballot_group> &
				to_sort) const;
		list<ballot_group> compress (const list<ballot_group> & 
				uncompressed) const;

		string ballot_to_text(const ballot_group & rank_ballot,
				const map<int, string> & reverse_cand_lookup,
				bool numeric) const;

		vector<string> ballots_to_text(string prefix, 
				const list<ballot_group> & rank_ballots,
				const map<int, string> & reverse_cand_lokoup,
				bool numeric) const;

		vector<string> ballots_to_text(const list<ballot_group> & 
				rank_ballots, const map<int, string> & 
				reverse_cand_lookup, bool numeric) const;

		void print_ranked_ballots(const list<ballot_group> & 
			rank_ballots) const;

};

#endif
