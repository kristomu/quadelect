// Generic class for spatial models. It's hardwired in many ways that should
// be made user-specifiable.

#include "spatial.h"
#include <iostream>


double spatial_generator::distance(const std::vector<double> & a,
	const std::vector<double> & b) const {

	double sum = 0;

	assert(a.size() == b.size());

	for (size_t counter = 0; counter < a.size(); ++counter) {
		sum += (a[counter]-b[counter])*(a[counter]-b[counter]);
	}

	return (sqrt(sum));
}

std::vector<double> spatial_generator::max_dim_vector(
	size_t dimensions) const {

	// Same as above, only generate the max vector. Slow? Perhaps.

	std::vector<double> toRet(ceil(dimensions), 1);
	return (toRet);
}

election_t spatial_generator::generate_ballots_int(
	int num_voters, int numcands, bool do_truncate,
	coordinate_gen & coord_source) const {

	election_t toRet;

	// I'm ripping out the truncation logic because
	// it's so bad anyway. FIX LATER.
	assert(!do_truncate);


	// First generate the candidate positions.

	// Optimize: could make this part of the class, and mutable.
	// Optimize: could also make this take a list<..>& as input and then
	// have an auxiliary generate_ballots_int create election_t,
	// run this on it, and return it. Probably not worth too much, given
	// that we have to compress anyway.
	std::vector<std::vector<double> > cand_positions;

	// If they're fixed and the number of candidates are right, then
	// defer to the fixed position.

	size_t counter, sec;

	if (fixed && (size_t)numcands == fixed_cand_positions.size()) {
		cand_positions = fixed_cand_positions;
	} else {
		for (counter = 0; counter < (size_t)numcands; ++counter)
			cand_positions.push_back(rnd_vector(num_dimensions,
					coord_source));
	}

	// Then, for each candidate, determine his position. If we need to
	// truncate, find a radius beyond which we don't care, otherwise
	// rank candidates in order of distance.

	// TODO: Move to another function.

	std::vector<double> voter_pos, distances_to_cand(numcands, 0);

	ballot_group our_entry;
	our_entry.set_weight(1);

	double max_distance = 0;

	for (counter = 0; counter < (size_t)num_voters; ++counter) {
		our_entry.contents.clear();

		// Get our location.
		voter_pos = rnd_vector(num_dimensions, coord_source);

		// Dump distances to all the candidates and get the minimum
		// distance (which may be needed for truncation).
		double mindist = INFINITY;
		for (sec = 0; sec < (size_t)numcands; ++sec) {
			distances_to_cand[sec] = distance(voter_pos,
					cand_positions[sec]);

			mindist = std::min(mindist, distances_to_cand[sec]);
			max_distance = std::max(max_distance, distances_to_cand[sec]);
		}

		// Compatibility with Warren.
		double spacing = sqrt(0.6 * num_dimensions);

		// For all candidates...
		for (sec = 0; sec < (size_t)numcands; ++sec) {
			// Get distance to candidate.
			double our_dist = distances_to_cand[sec];

			// If warren_utility is true, use Warren's utility
			// model.
			if (warren_utility) {
				our_entry.contents.insert(candscore(sec, 1 /
						(spacing + our_dist)));
			}
			// ... otherwise use JGA's.
			else {
				assert(our_dist <= max_distance);
				our_entry.contents.insert(candscore(sec,
						-our_dist));
			}
		}

		our_entry.complete = (our_entry.contents.size() ==
				(size_t)numcands);
		our_entry.rated = true;

		toRet.push_back(our_entry);
	}

	return toRet;
}

bool spatial_generator::set_params(size_t num_dimensions_in,
	bool warren_util_in) {

	if (num_dimensions_in < 1) {
		return false;
	}

	warren_utility = warren_util_in;

	num_dimensions = num_dimensions_in;

	// If we've defined a mean, pad it with zeroes or cut as needed.
	if (center.size() != num_dimensions && !center.empty()) {
		center.resize(num_dimensions, 0);
	}

	return true;
}

double spatial_generator::get_score_quantile(
	coordinate_gen & coord_source, double p, size_t iterations) const {

	// Very slow way of doing this: I ought to refactor the ballot
	// generation instead so both it and this can call on the same
	// logic. Then the ballot generation wouldn't need to be protected.

	std::vector<double> scores;

	for (int i = 0; i < 1; ++i) {

		election_t election = generate_ballots_int(
				iterations, 1, false, coord_source);

		for (const ballot_group & g: election) {

			// Quick and dirty check because we don't support
			// non-unit weights yet. There are ways to handle
			// weighted votes without a blowup in the length
			// of the scores array. Do that later if needed.
			assert(g.get_weight() < 1.1 && g.get_weight() > 0.99);

			for (const candscore & cs: g.contents) {
				scores.push_back(cs.get_score());
			}
		}
	}

	std::sort(scores.begin(), scores.end());

	return scores[(size_t)round((scores.size()-1) * p)];
}

double spatial_generator::get_optimal_utility(
	coordinate_gen & coord_source, size_t num_voters,
	size_t numcands, size_t iterations) const {

	double total_opt_score = 0;
	std::vector<double> candidate_scores(numcands);

	size_t num_elections = iterations/(numcands * num_voters);

	for (size_t i = 0; i < num_elections; ++i) {

		election_t election = generate_ballots_int(
				num_voters, numcands, false, coord_source);

		std::fill(candidate_scores.begin(), candidate_scores.end(), 0);

		for (const ballot_group & g: election) {
			for (const candscore & cs: g.contents) {
				assert(cs.get_candidate_num() < numcands);
				candidate_scores[cs.get_candidate_num()] +=
					g.get_weight() * cs.get_score() / (double)num_voters;
			}
		}

		total_opt_score += *std::max_element(candidate_scores.begin(),
				candidate_scores.end());
	}

	return total_opt_score/num_elections;
}

double spatial_generator::get_mean_utility(
	coordinate_gen & coord_source, size_t num_voters,
	size_t numcands, size_t iterations) const {

	// Determine the random utility of a voter-candidate pair.
	// Experiments with Gaussians seem to indicate that there's no
	// distinction between candidates and voters, i.e. we converge on
	// the same result regardless of whether we draw candidates a few
	// times and voters lots of times, or vice versa; but I have yet to
	// prove this.

	// For now we evenly divide candidates and voters. Simplify
	// once I have a better theoretical backing for the apparent
	// result. TODO.

	double total_score = 0, total_weight = 0;
	size_t num_elections = iterations/(numcands * num_voters);

	for (size_t i = 0; i < num_elections; ++i) {

		election_t election = generate_ballots_int(
				num_voters, numcands, false, coord_source);

		for (const ballot_group & g: election) {
			for (const candscore & cs: g.contents) {
				total_score += g.get_weight() * cs.get_score();
				total_weight += g.get_weight();
			}
		}
	}

	return total_score/total_weight;

}

bool spatial_generator::fix_candidate_positions(int num_cands,
	const std::vector<std::vector<double> > cand_positions) {
	// If we have the wrong number of candidates or the wrong number of
	// dimensions, no go.

	if ((int)cand_positions.size() != num_cands) {
		return (false);
	}
	if (!cand_positions.empty() && cand_positions[0].size() !=
		ceil(num_dimensions)) {
		return (false);
	}

	// TODO: Check bounds more carefully by max vector. If we're dealing
	// with 2.5 dimensions and the other position array is 3D, we might
	// get a too high value in the 3rd element of the vector. Things like
	// that.

	fixed = true;
	fixed_cand_positions = cand_positions;
	return (true);
}

bool spatial_generator::fix_candidate_positions(int num_cands,
	coordinate_gen & coord_source) {
	if (num_cands < 1) {
		return false;    // Surely you jest!
	}

	std::vector<std::vector<double> > cand_positions;

	for (int counter = 0; counter < num_cands; ++counter)
		cand_positions.push_back(rnd_vector(num_dimensions,
				coord_source));

	return (fix_candidate_positions(num_cands, cand_positions));
}

std::vector<std::vector<double> >
spatial_generator::get_fixed_candidate_pos() const {

	// If no positions have been fixed, there's nothing we can do!
	if (!fixed) {
		return (std::vector<std::vector<double> >());
	}

	return (fixed_cand_positions);
}

bool spatial_generator::set_center(const std::vector<double> center_in) {
	if (!uses_center) {
		return (false);
	}

	// If it's not the right number of dimensions, return false too.
	if (center_in.size() != ceil(num_dimensions)) {
		return (false);
	}

	// Otherwise, all OK. (Again, should check against min and max.)
	center = center_in;
	return (true);
}

bool spatial_generator::set_dispersion(const std::vector<double>
	dispersion_in) {
	if (!uses_dispersion) {
		return (false);
	}

	// If it's not the right number of dimensions, return false too.
	if (dispersion_in.size() != ceil(num_dimensions)) {
		return (false);
	}

	// If not all values are positive, return false.
	for (size_t i = 0; i < dispersion_in.size(); ++i)
		if (dispersion_in[i] <= 0) {
			return (false);
		}

	// Otherwise, all OK. (Again, should check against min and max.)
	dispersion = dispersion_in;
	return (true);
}
