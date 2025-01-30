// Dodgson approximations: "Dodgson Quick". This is really just conddlr where
// every opposition score has been halved and rounded up. Properly defined,
// Dodgson Quick is on margins.

// MCCABE-DANSTED, John C.; PRITCHARD, Geoffrey; SLINKO, Arkadii.
// Approximability of Dodgson’s rule. Social Choice and Welfare, 2008,
// 31.2: 311-330.

#include "dodgson_approxs.h"

std::pair<ordering, bool> dquick::pair_elect(const abstract_condmat &
	input,
	const std::vector<bool> & hopefuls, cache_map * cache,
	bool winner_only) const {

	ordering out;

	for (size_t incumbent = 0; incumbent < input.get_num_candidates();
		++incumbent) {

		if (!hopefuls[incumbent]) {
			continue;
		}
		double sum = 0;

		for (size_t challenger = 0; challenger < input.get_num_candidates();
			++challenger) {

			if (incumbent == challenger || !hopefuls[challenger]) {
				continue;
			}

			sum += ceil(input.get_magnitude(challenger, incumbent,
						hopefuls)/2.0);
		}

		out.insert(candscore(incumbent, -sum));
	}

	return (std::pair<ordering, bool>(out, false));
}
