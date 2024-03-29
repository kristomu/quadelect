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
	int num_voters,
	int numcands, bool do_truncate, coordinate_gen & coord_source) const {

	election_t toRet;

	// First generate the candidate positions.
	// Hm, perhaps this is where JGA's strategy test differs from mine.
	// Perhaps his sets candidates uniformly, then have a Gaussian dist.
	// for voters, whereas mine would have a Gaussian for both...

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

	size_t eff_dimensions = ceil(num_dimensions);

	std::vector<double> voter_pos, max_dist(eff_dimensions, 0),
		distances_to_cand(numcands, 0);

	std::vector<double> max_vector = max_dim_vector(num_dimensions);

	ballot_group our_entry;
	our_entry.set_weight(1);

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
		}

		// Report all distances within this area. If we're not going
		// to truncate, include everyone...
		double care_within = num_dimensions;

		// Determine the maximum possible distance that's still within
		// the space. It's used to adjust utilities if we're not using
		// Warren's utility model or truncation.
		double max_distance = INFINITY;

		if (do_truncate || !warren_utility) {
			// Find the closest point by this observation: for each
			// dimension, if we're closer to the zero-edge than the
			// other edge, the closest point is at the other edge,
			// otherwise it's at the zero edge. This can be done
			// because distance is decomposable.

			for (sec = 0; sec < eff_dimensions; ++sec) {
				if (voter_pos[sec] > max_vector[sec] * 0.5) {
					max_dist[sec] = 0;    // TODO, min?
				} else	{
					max_dist[sec] = max_vector[sec];
				}
			}

			max_distance = distance(voter_pos, max_dist);
		}

		if (do_truncate) {
			// If we're going to truncate, pick a random distance
			// between ourselves and the farthest corner. All
			// candidates more distant than that get truncated.
			care_within = drand48() * distance(voter_pos, max_dist);

			// If the distance is closer than the closest candidate,
			// expand so at least one candidate gets rated.
			if (care_within < mindist) {
				care_within = mindist;
			}
		}

		// Compatibility with Warren.
		double spacing = sqrt(0.6 * num_dimensions);

		// Expensive part ahead!
		// For all candidates...
		for (sec = 0; sec < (size_t)numcands; ++sec) {
			// Get distance to candidate.
			double our_dist = distances_to_cand[sec];

			if (our_dist > care_within) {
				continue;
			}

			// If warren_utility is true, use Warren's utility
			// model.
			if (warren_utility) {
				our_entry.contents.insert(candscore(sec, 1 /
						(spacing + our_dist)));
			}
			// ... otherwise use JGA's.
			else {
				our_entry.contents.insert(candscore(sec,
						max_distance-our_dist));
			}
		}

		our_entry.complete = (our_entry.contents.size() ==
				(size_t)numcands);
		our_entry.rated = true;

		toRet.push_back(our_entry);
	}

	return (toRet);
}

bool spatial_generator::set_params(size_t num_dimensions_in,
	bool warren_util_in) {

	if (num_dimensions_in < 1) {
		return (false);
	}

	warren_utility = warren_util_in;

	num_dimensions = num_dimensions_in;

	// If we've defined a mean, pad it with zeroes or cut as needed.
	if (center.size() != num_dimensions && !center.empty()) {
		center.resize(num_dimensions, 0);
	}

	return (true);
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
		return (false);    // Surely you jest!
	}

	std::vector<std::vector<double> > cand_positions;

	for (int counter = 0; counter < num_cands; ++counter)
		cand_positions.push_back(rnd_vector(num_dimensions,
				coord_source));

	return (fix_candidate_positions(num_cands, cand_positions));
}

std::vector<std::vector<double> >
spatial_generator::get_fixed_candidate_pos()
const {

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