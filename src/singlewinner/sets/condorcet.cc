
#include "condorcet.h"

int condorcet_set::get_CW(const abstract_condmat & input,
	const std::vector<bool> & hopefuls) const {

	// First, we go through every candidate, where the winner of a one-on
	// one between the first and second in line wins. If there's a CW, we'll
	// end up with the CW.

	int curcand = 0;
	int counter;

	// Find the first hopeful candidate
	for (curcand = 0; curcand < input.get_num_candidates() &&
		!hopefuls[curcand]; ++curcand) {
	}

	// If none, get outta here.
	if (curcand == input.get_num_candidates()) {
		return -1;
	}

	for (counter = curcand+1; counter < input.get_num_candidates(); ++counter)
		if (hopefuls[counter] &&
			input.get_magnitude(counter, curcand,
				hopefuls) >
			input.get_magnitude(curcand, counter,
				hopefuls)) {
			curcand = counter;
		}

	// Then, we check that candidate's results against everybody else.
	// If he wins against all of them, he's the CW, otherwise there's a
	// cycle and we should return -1.

	for (counter = 0; counter < input.get_num_candidates(); ++counter)
		if (hopefuls[counter] && counter != curcand)
			if (input.get_magnitude(counter, curcand, hopefuls) >=
				input.get_magnitude(curcand, counter,
					hopefuls)) {
				return (-1);
			}

	return (curcand);
}

std::pair<ordering, bool> condorcet_set::pair_elect(const abstract_condmat &
	input,
	const std::vector<bool> & hopefuls, cache_map * cache,
	bool winner_only) const {

	// We start with the hopefuls we have. Find the CW, insert into the
	// output ordering below what's already there, mark as no longer
	// hopeful, and repeat. Stop when we get a -1 or all candidates have
	// been added.

	std::vector<bool> iter_hopefuls = hopefuls;
	std::vector<bool> cws(input.get_num_candidates(), false);
	ordering to_ret;
	int rank = 0, cw;
	size_t counter = 0;

	do {
		cw = get_CW(input, iter_hopefuls);

		if (cw != -1) {
			to_ret.insert(candscore(cw, rank--));
			iter_hopefuls[cw] = false;
			cws[cw] = true;
			++counter;
		}

	} while (cw != -1 && !winner_only &&
		counter < input.get_num_candidates());

	// Add all the other candidates below the iterated CWs.

	for (counter = 0; counter < cws.size(); ++counter)
		if (iter_hopefuls[counter] && !cws[counter]) {
			to_ret.insert(candscore(counter, rank));
		}

	return (std::pair<ordering, bool>(to_ret, winner_only));
}
