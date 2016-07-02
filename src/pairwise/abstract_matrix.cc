
#include "../ballots.h"

#include "types.h"
#include "abstract_matrix.h"

#include <iterator>
#include <iostream>

using namespace std;

double abstract_condmat::get_magnitude(int candidate, int against,
		const vector<bool> & hopefuls) const {

	// size() is a bit expensive, thus I'm doing it this way.
	if (candidate < against)
		assert (candidate >= 0 && against <= hopefuls.size());
	else	assert (against >= 0 && candidate <= hopefuls.size());

	// Make sure num_voters is properly set.
	assert (get_num_voters() != -INFINITY);

	if (hopefuls[candidate] && hopefuls[against])
		return(get_magnitude(candidate, against));
	else	return(0);
}

bool abstract_condmat::set(int candidate, int against_candidate, double value) {
	return(set_internal(candidate, against_candidate, value));
}

