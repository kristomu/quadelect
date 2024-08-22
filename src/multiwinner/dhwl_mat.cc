// D'Hondt without lists

#ifndef _VOTE_DHWL_MAT
#define _VOTE_DHWL_MAT

#include "../condorcet/matrix.cc"
#include <vector>

using namespace std;

class DHwLmatrix : public condmat {
	private:
		vector<bool> elected;
		double C;

		double downweight(int num_higher_elected_prefs) const;

		void count_ballots(const list<ballot_group> & scores,
			int num_candidates);

	public:
		void set_elected(const vector<bool> & source);
		void set_elected(int elected_idx);

		DHwLmatrix(const list<ballot_group> & scores,
			const vector<bool> & already_elected,
			int num_candidates, condorcet_type kind,
			bool tie_at_top, double C_in);
};

double DHwLmatrix::downweight(int num_higher_elected_prefs) const {
	return (C/(double)(C + num_higher_elected_prefs));
}

void DHwLmatrix::count_ballots(const list<ballot_group> & scores,
	int num_candidates) {

	// See condorcet/matrix.cc

	list<ballot_group>::const_iterator pri;
	list<candscore>::const_iterator checker, against;


	for (pri = scores.begin(); pri != scores.end(); ++pri) {
		// Reduce n^2 to n as set iterations are extremely slow.
		// KLUDGE.
		list<candscore> first_ch;
		copy(pri->contents.begin(), pri->contents.end(), inserter(
				first_ch, first_ch.begin()));

		list<candscore>::const_iterator first = first_ch.begin();

		int dimin = 0, level_dimin = 0;
		double last_level_score = -1;

		for (checker = first_ch.begin(); checker !=
			first_ch.end(); ++checker) {

			// Credit victories, with a twist. Any pairwise
			// preference stated below someone that's already been
			// elected gets downweighted by a function of the number
			// of candidates already elected also ranked higher.
			// Ties don't count.

			// --

			// If we're now at the next level, so that we know that
			// the current candidates are all both ranked below
			// any potential elected ones (and not just tied),
			// update the reweight number.
			if (last_level_score != checker->get_score()) {
				dimin = level_dimin;
			}

			double new_wt = downweight(dimin) * pri->weight;

			// If we happen upon an elected candidate, increment
			// the reweight number for the lower ranks (will get
			// updated in the if just above this one).
			if (elected[checker->get_candidate_num()]) {
				++level_dimin;
				last_level_score = checker->get_score();
			}

			for (against = inc(checker); against != first_ch.
				end(); ++against)
				if (checker->get_score() > against->get_score())
					matrix[checker->get_candidate_num()][
						against->get_candidate_num()] +=
							new_wt;

			// Tied at the top. See condorcet/matrix.cc
			// It's not possible for there to be a reweighting at
			// this stage (first preference), so copy directly.

			if (checker != first_ch.begin()) {
				continue;
			}

			for (against = inc(checker); against != first_ch.
				end() && against->get_score() !=
				checker->get_score(); ++against) {
				tied_at_top[checker->get_candidate_num()][
					against->get_candidate_num()] +=
						pri->weight;
				tied_at_top[against->get_candidate_num()][
					checker->get_candidate_num()] +=
						pri->weight;
			}
		}
	}
}

void DHwLmatrix::set_elected(const vector<bool> & source) {
	assert(source.size() == elected.size());

	elected = source;
}

void DHwLmatrix::set_elected(int elected_idx) {
	assert(elected_idx < elected.size());

	// Perhaps some stuff here about recounting being necessary? That's
	// probably the right thing to do with the CM in general..

	elected[elected_idx] = true;
}

DHwLmatrix::DHwLmatrix(const list<ballot_group> & scores, const
	vector<bool> & already_elected,	int num_candidates,
	condorcet_type kind, bool tie_at_top, double C_in) : condmat() {

	assert(0 <= C_in && C_in <= 1);
	C = C_in;

	init(num_candidates, kind, tie_at_top);
	elected = already_elected;

	// Count the sum weight (which we'll need for LV).
	for (list<ballot_group>::const_iterator pri = scores.begin(); pri !=
		scores.end(); ++pri) {
		num_voters += pri->weight;
	}

	count_ballots(scores, num_candidates);
}

#endif
