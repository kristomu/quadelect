#include "iac.h"
#include "tools/tools.h"

// To generate an election according to IAC, we use the sorted random uniform
// algorithm: create (n-1) random points on 0..|V|, sort them in ascending order,
// add 0 to the beginning and |V| to the end, and then take the differences to get
// the counts per ballot.

election_t iac::generate_ballots_int(
	int num_voters, int numcands, bool do_truncate,
	coordinate_gen & coord_source) const {

	if (do_truncate) {
		throw std::runtime_error("Impartial anon. culture does not support truncation.");
	}

	size_t num_permutations = factorial(numcands);
	std::vector<int> interval_source(num_permutations-1);

	size_t i;
	for (i = 0; i < num_permutations-1; ++i) {
		interval_source[i] = coord_source.next_int(num_voters+1);
	}

	interval_source.push_back(0);
	interval_source.push_back(num_voters);
	std::sort(interval_source.begin(), interval_source.end());

	election_t ballots;

	// Create a ballot order that we'll iterate through.
	std::vector<int> candidate_order(numcands, 0);
	std::iota(candidate_order.begin(), candidate_order.end(), 0);

	// And iterate through them.
	i = -1; // Pump priming

	do {
		++i;

		int weight = interval_source[i+1] - interval_source[i];
		if (weight == 0) {
			continue;
		}

		ordering cur_ordering;
		for (int j = 0; j < numcands; ++j) {
			cur_ordering.insert(candscore(candidate_order[j], numcands-j));
		}

		ballots.push_back(ballot_group(weight, cur_ordering, true, false));
	} while (std::next_permutation(candidate_order.begin(),
			candidate_order.end()));

	return ballots;
};