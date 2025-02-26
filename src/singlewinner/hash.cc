#include "hash.h"

std::pair<ordering, bool> hash_random_cand::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls, int num_candidates,
	cache_map * cache, bool winner_only) const {

	// Dump the ballot data to a double vector that
	// we'll later read into the hash function.
	std::vector<double> ballot_byte_rep;
	// Reserve a spot for the candidate number.
	ballot_byte_rep.push_back(-1);

	for (const ballot_group & bg: papers) {
		ballot_byte_rep.push_back(bg.get_weight());
		for (const candscore & cs: bg.contents) {
			ballot_byte_rep.push_back(cs.get_score());
			ballot_byte_rep.push_back(cs.get_candidate_num());
		}
	}

	// Get the "score" of each candidate by setting the first
	// element to the candidate number. Highest score wins.

	ordering out;

	for (size_t candidate = 0; candidate < (size_t)num_candidates;
		++candidate) {

		ballot_byte_rep[0] = candidate;

		size_t score = hasher.Hash64(
				ballot_byte_rep.data(),
				sizeof(double) * ballot_byte_rep.size(),
				0);

		out.insert(candscore(candidate, score));
	}

	return (std::pair<ordering, bool>(out, false));
}