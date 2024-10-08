// D'Hondt without lists

#include "dhwl_mat.h"

double DHwLmatrix::deweight(int num_higher_elected_prefs) const {
	return (C/(double)(C + num_higher_elected_prefs));
}

void DHwLmatrix::add(size_t candidate, size_t against,
	double value) {

	set_internal(candidate, against,
		get_internal(candidate, against, true) + value);
}

void DHwLmatrix::count_ballots(const election_t & scores,
	size_t num_candidates) {

	// See pairwise/matrix.cc

	election_t::const_iterator pri;
	std::list<candscore>::const_iterator checker, against;

	// First set the total number of voters (voting weight),
	// which the abstract matrix class requires.

	double total_weight = 0;

	for (pri = scores.begin(); pri != scores.end(); ++pri) {
		total_weight += pri->get_weight();
	}

	set_num_voters(total_weight);

	// Then set the pairwise contests.

	for (pri = scores.begin(); pri != scores.end(); ++pri) {
		// Reduce n^2 to n as set iterations are extremely slow.
		// KLUDGE.
		std::list<candscore> first_ch;
		std::copy(pri->contents.begin(), pri->contents.end(),
			std::inserter(first_ch, first_ch.begin()));

		int dimin = 0, level_dimin = 0;
		double last_level_score = -1;

		for (checker = first_ch.begin(); checker !=
			first_ch.end(); ++checker) {

			// Credit victories, with a twist. Any pairwise
			// preference stated below someone that's already been
			// elected gets deweighted by a function of the number
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

			double new_wt = deweight(dimin) * pri->get_weight();

			// If we happen upon an elected candidate, increment
			// the reweight number for the lower ranks (will get
			// updated in the if just above this one).
			if (elected[checker->get_candidate_num()]) {
				++level_dimin;
				last_level_score = checker->get_score();
			}

			for (against = inc(checker); against != first_ch.
				end(); ++against) {
				if (checker->get_score() > against->get_score()) {
					add(checker->get_candidate_num(),
						against->get_candidate_num(),
						new_wt);
				}
			}

			// There used to be a tied at the top thing here.
			// Is that something that I should port over to condmat?
			// XXX???

			// Would some sort of inversion of control help with the
			// duplicate code here? Yes, but at what cost?
		}
	}
}

void DHwLmatrix::set_elected(const std::vector<bool> & source) {
	if (source.size() != elected.size()) {
		throw std::invalid_argument("Proposed elected set is of the wrong size!");
	}

	elected = source;
}

void DHwLmatrix::set_elected(size_t elected_idx) {
	if (elected_idx >= elected.size()) {
		throw std::invalid_argument(
			"Can't set a candidate as elected who's not in the election.");
	}

	// Perhaps some stuff here about recounting being necessary? That's
	// probably the right thing to do with the CM in general..

	elected[elected_idx] = true;
}

DHwLmatrix::DHwLmatrix(const election_t & scores, const
	std::vector<bool> & already_elected, size_t num_candidates,
	pairwise_type type_in, bool tie_at_top, double C_in) : condmat(
			type_in) {

	if (num_candidates == 0) {
		throw std::invalid_argument("DHwLmatrix: Must have at least "
			"one candidate");
	}

	if (C_in <= 0 || C_in > 1) {
		throw std::invalid_argument(
			"DHwLmatrix: reweighting factor out of range");
	}

	C = C_in;

	elected = already_elected;

	zeroize(num_candidates);
	count_ballots(scores, num_candidates);
}
