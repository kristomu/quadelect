
#include "common/ballots.h"

#include "types.h"
#include "abstract_matrix.h"

#include <iterator>
#include <iostream>
#include <stdexcept>


double abstract_condmat::get_magnitude(size_t candidate, size_t against,
	const std::vector<bool> & hopefuls) const {

	// size() is a bit expensive, thus I'm doing it this way.
	if (std::max(candidate, against) >= hopefuls.size()) {
		throw std::out_of_range(
			"get_magnitude: candidate number too large!");
	}

	// Make sure num_voters is properly set.
	if (!finite(get_num_voters())) {
		throw std::out_of_range(
			"get_magnitude: number of voters is not finite!");
	}

	if (hopefuls[candidate] && hopefuls[against]) {
		return get_magnitude(candidate, against);
	} else {
		return 0;
	}
}

bool abstract_condmat::set(size_t candidate, size_t against_candidate,
	double value) {

	return set_internal(candidate, against_candidate, value);
}

void abstract_condmat::debug_print_raw_values() const {
	for (size_t row_cand = 0; row_cand < get_num_candidates(); ++row_cand) {
		for (size_t col_cand = 0; col_cand < get_num_candidates(); ++col_cand) {
			if (row_cand == col_cand) {
				std::cout << "--- ";
			} else {
				std::cout << get_internal(row_cand, col_cand, true) << " ";
			}
		}
		std::cout << "\n";
	}
}