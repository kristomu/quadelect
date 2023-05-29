
#include "../ballots.h"

#include "types.h"
#include "abstract_matrix.h"

#include <iterator>
#include <iostream>
#include <stdexcept>

using namespace std;

double abstract_condmat::get_magnitude(size_t candidate, size_t against,
		const vector<bool> & hopefuls) const {

	// size() is a bit expensive, thus I'm doing it this way.
	if (std::max(candidate, against) >= hopefuls.size()) {
		throw std::out_of_range(
			"get_magnitude: candidate number too large!");
	}

	// Make sure num_voters is properly set.
	assert (get_num_voters() != -INFINITY);

	if (hopefuls[candidate] && hopefuls[against])
		return(get_magnitude(candidate, against));
	else	return(0);
}

bool abstract_condmat::set(size_t candidate, size_t against_candidate, double value) {
	return(set_internal(candidate, against_candidate, value));
}

