// Hard Cardinal-type voting methods: Birational voting and LPV.
// These do an exhaustive search over the candidate space, and so can take a
// VERY long time for many candidates.

// Perhaps include minimax.

#include "hard_card.h"

#include <numeric>

// Auxiliary functions. TESTS thus KLUDGY. Fix later

double gini_one(const std::vector<double> & a) {

	double total = 0;
	double r_gini_count = 0;

	for (size_t counter = 0; counter < a.size(); ++counter) {
		for (size_t sec = 0; sec < a.size(); ++sec) {
			r_gini_count += fabs(a[counter] - a[sec]);
		}
		total += a[counter];
	}

	double mean = total / (double)a.size();

	double gini = r_gini_count / (2 * a.size() * a.size() * mean);

	return gini;
}

// Sen's welfare function
double welfare(const std::vector<double> & a) {

	double total = 0;

	for (size_t counter = 0; counter < a.size(); ++counter) {
		total += a[counter];
	}

	return (1-gini_one(a)) * (total / (double)(a.size()));
}

/////////////////////////////////////

std::vector<scored_ballot> hardcard::make_cardinal_array(const
	election_t & ballots, int numcand) const {

	// This is used to calculate birational and LPV results quickly, as
	// those methods have terms like "voter X's rating of candidate Y".
	// With the usual ballot format, that would take at least linear
	// (perhaps quadratic) time, whereas this takes constant.

	// We use NAN for "no opinion" Range-style values.

	std::vector<scored_ballot> results(ballots.size());

	int counter = 0;
	for (election_t::const_iterator pos = ballots.begin();
		pos != ballots.end(); ++pos) {

		results[counter].weight = pos->get_weight();
		results[counter].scores.resize(numcand, NAN);

		for (ordering::const_iterator opos = pos->contents.begin();
			opos != pos->contents.end(); ++opos) {
			results[counter].scores[opos->get_candidate_num()] =
				opos->get_score();
		}
		++counter;
	}

	return results;
}

double hardcard::birational(const std::vector<bool> & W,
	const scored_ballot & this_ballot) const {

	//                                     x_w
	// L(w) = SUM      SUM      SUM      -------
	// 	  vote     w in W   s in W   1 + x_s
	// 	  vectors
	// 	  x->

	// We don't handle Range-style ballots yet - they get set to 0.
	// Possible later TODO, change 1 + x_s so that the D'Hondt
	// generalization of PAV turns into Sainte-Laguë instead.

	double total = 0;

	// Minimax doesn't do much better either.

	for (size_t w = 0; w < W.size(); ++w) {
		if (!W[w]) {
			continue;
		}

		for (size_t s = 0; s < W.size(); ++s) {
			if (!W[s]) {
				continue;
			}
			double atw = this_ballot.scores[w],
				   ats = this_ballot.scores[s];

			if (!isfinite(atw)) {
				atw = 0;
			}
			if (!isfinite(ats)) {
				ats = 0;
			}

			total += atw / (1 + ats);
		}
	}

	return total * this_ballot.weight;
}

double hardcard::birational(const std::vector<bool> & W,
	const std::vector<scored_ballot> & ballots) const {

	double total = 0;

	for (std::vector<scored_ballot>::const_iterator pos = ballots.begin();
		pos != ballots.end(); ++pos) {
		total += birational(W, *pos);
	}

	return total;
}


// For merging recursion values.
hardcard_extrema hardcard::merge_extrema(const hardcard_extrema a,
	const hardcard_extrema b) const {

	hardcard_extrema toRet = a;

	/*std::vector<bool> W_minimum_set, W_maximum_set;
	double W_minimum, W_maximum;*/

	if (b.W_minimum < toRet.W_minimum || isnan(toRet.W_minimum)) {
		toRet.W_minimum = b.W_minimum;
		toRet.W_minimum_set = b.W_minimum_set;
	}

	if (b.W_maximum > toRet.W_maximum || isnan(toRet.W_maximum)) {
		toRet.W_maximum = b.W_maximum;
		toRet.W_maximum_set = b.W_maximum_set;
	}

	return toRet;
}

// Ain't recursion nifty? Here we recurse to find the best of all possible
// council sets.
hardcard_extrema hardcard::all_birational(const std::vector<scored_ballot> &
	ballots, const std::vector<bool> & cur_W, std::vector<bool>::iterator
	begin, std::vector<bool>::iterator end, int marks_left) const {

	// All marked!
	if (marks_left == 0) {
		hardcard_extrema result;
		result.W_minimum_set = cur_W;
		result.W_minimum = birational(cur_W, ballots);

		result.W_maximum_set = result.W_minimum_set;
		result.W_maximum = result.W_minimum;

		return result;
	}

	// Not an admissible council since marks_left != 0
	if (begin == end) {
		hardcard_extrema result;
		result.W_minimum = NAN;
		result.W_maximum = NAN;
		return result;
	}

	// Not at the end, but marks_left isn't 0 either! So recurse twice,
	// one towards the end with cur_W not set, another towards the end with
	// cur_W set and marks_left--.

	*begin = true;
	hardcard_extrema a = all_birational(ballots, cur_W, begin+1, end,
			marks_left - 1);
	*begin = false;
	// TODO later: early break, if even if all later were true, marks
	// left would be above zero.
	hardcard_extrema b = all_birational(ballots, cur_W, begin+1, end,
			marks_left);

	return merge_extrema(a, b);
}

double hardcard::LPV(const std::vector<bool> & W, int council_size,
	const scored_ballot & this_ballot, double k) const {

	/*
	                                        /     K + |W|    \
	   L_k(W) = SUM     SUM       x_j *   ln| -------------- |
	            vote    j in C              | K + SUM    x_s |
	            vector                      \     s in W     /
	*/

	// That can be optimized by calculating the logarithm just once,
	// which we do.

	// Get denominator
	double log_denom = k;

	size_t counter = 0;

	for (counter = 0; counter < std::min(W.size(),
			this_ballot.scores.size()); ++counter) {
		if (W[counter]) {
			log_denom += this_ballot.scores[counter];
		}
	}

	double logarithm = log((k + council_size)/(log_denom));

	if (!isfinite(logarithm)) {
		return logarithm;
	}

	double total = 0;

	for (counter = 0; counter < this_ballot.scores.size(); ++counter) {
		total += (this_ballot.scores[counter] * logarithm);
	}

	return this_ballot.weight * total;
}


double hardcard::LPV(const std::vector<bool> & W, int council_size,
	const std::vector<scored_ballot> & ballots, double k) const {

	double total = 0;

	for (size_t counter = 0; counter < ballots.size() && isfinite(total);
		++counter) {
		total += LPV(W, council_size, ballots[counter], k);
	}

	return total;
}

hardcard_extrema hardcard::all_LPV(const std::vector<scored_ballot> &
	ballots,
	const std::vector<bool> & cur_W, double k, int council_size,
	std::vector<bool>::iterator begin,
	std::vector<bool>::iterator end, int marks_left) const {

	// See birational.
	hardcard_extrema result;

	if (marks_left == 0) {
		result.W_minimum_set = cur_W;
		result.W_minimum = LPV(cur_W, council_size, ballots, k);

		result.W_maximum_set = result.W_minimum_set;
		result.W_maximum = result.W_minimum;
		return result;
	}

	if (begin == end) {
		result.W_minimum = NAN;
		result.W_maximum = NAN;
		return result;
	}

	*begin = true;
	hardcard_extrema a = all_LPV(ballots, cur_W, k, council_size, begin+1,
			end, marks_left - 1);
	*begin = false;
	hardcard_extrema b = all_LPV(ballots, cur_W, k, council_size, begin+1,
			end, marks_left);

	return merge_extrema(a, b);
}

std::list<int> hardcard::get_council(int council_size, int num_candidates,
	const election_t & ballots) const {

	std::vector<scored_ballot> cballots = make_cardinal_array(ballots,
			num_candidates);
	std::vector<bool> W(num_candidates, false);

	// Paper over a BUG. Handle this in a more principled manner,
	// probably with a "next_combination" analog, later.
	// TODO. Who knew I could write such buggy code :-P
	if (council_size == num_candidates) {
		std::list<int> out(num_candidates, 0);
		std::iota(out.begin(), out.end(), 0);
		return out;
	}

	std::list<int> council;
	size_t counter;

	if (type == HC_BIRATIONAL) {
		hardcard_extrema birat = all_birational(cballots, W, W.begin(),
				W.end(), council_size);

		// Turn into council
		std::vector<bool> winner_b = birat.W_maximum_set;

		for (counter = 0; counter < winner_b.size(); ++counter)
			if (winner_b[counter]) {
				council.push_back(counter);
			}
	}

	if (type == HC_LPV) {
		double k = 1e-13; // ideally 0, but that brings infinities

		hardcard_extrema result = all_LPV(cballots, W, k, council_size,
				W.begin(), W.end(), council_size);

		for (counter = 0; counter < result.W_minimum_set.size();
			++counter)
			if (result.W_minimum_set[counter]) {
				council.push_back(counter);
			}
	}

	return council;
}

std::string hardcard::name() const {
	switch (type) {
		case HC_LPV: return ("Cardinal: LPV");
		case HC_BIRATIONAL: return ("Cardinal: Birational");
		default:
			throw std::logic_error("Hardcard: Unsupported type!");
	}
}
