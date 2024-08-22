#ifndef _VOTE_DHWL
#define _VOTE_DHWL

#include "dhwl_mat.cc"
#include "methods.cc"
#include "../condorcet/methods.cc"
#include <list>

using namespace std;

class reweighted_condorcet : public multiwinner_method {
	private:
		condorcet_method * base;

	public:
		list<int> get_council(int council_size,
			int num_candidates,
			const list<ballot_group> & ballots) const;

		string name() const {
			return ("DHwL(" + base->name() + ")");
		}

		reweighted_condorcet(condorcet_method * base_in) {
			base = base_in;
		}

};

list<int> reweighted_condorcet::get_council(int council_size,
	int num_candidates, const list<ballot_group> & ballots) const {

	assert(council_size <= num_candidates);

	// Until the council has been filled: Run a DHwL_matrix election
	// with the given Condorcet method. Pick the first non-elected candidate
	// in the social order, add him to the council, and set that one as
	// elected.

	list<int> council;
	int council_count = 0;

	vector<bool> elected(num_candidates, false);

	while (council_count < council_size) {

		cerr << "[" << council_count << "]" << flush;

		bool tie_at_top = false; // TODO: Fix later.

		ordering social_order = base->cond_elect(DHwLmatrix(ballots,
					elected, num_candidates,
					base->get_type(), tie_at_top, 0.5));

		int next_candidate = -1;

		for (ordering::const_iterator pos = social_order.begin(); pos !=
			social_order.end() && next_candidate == -1;
			++pos)
			if (!elected[pos->get_candidate_num()]) {
				next_candidate = pos->get_candidate_num();
			}

		assert(next_candidate != -1);

		council.push_back(next_candidate);
		elected[next_candidate] = true;
		++council_count;
	}

	return (council);
}


#endif
