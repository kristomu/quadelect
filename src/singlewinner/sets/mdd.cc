#include "mdd.h"

std::pair<ordering, bool> mdd_set::pair_elect(
	const abstract_condmat & input,
	const std::vector<bool> & hopefuls, cache_map * cache,
	bool winner_only) const {

	std::vector<int> num_defeats(input.get_num_candidates(), 0);

	size_t candidate, challenger;

	for (candidate = 0; candidate < input.get_num_candidates(); ++candidate) {
		if (!hopefuls[candidate]) {
			continue;
		}
		for (challenger = 0; challenger < input.get_num_candidates();
			++challenger) {

			if (candidate == challenger) {
				continue;
			}
			if (!hopefuls[challenger]) {
				continue;
			}

			if (input.get_magnitude(candidate, challenger, hopefuls) >
				input.get_num_voters() * 0.5) {
				++num_defeats[challenger];
			}
		}
	}

	ordering toRet;

	for (candidate = 0; candidate < input.get_num_candidates(); ++candidate) {
		if (!hopefuls[candidate]) {
			continue;
		}

		int score;
		if (sum_defeats) {
			score = -num_defeats[candidate];
		} else {
			if (num_defeats[candidate] > 0) {
				score = -1;
			} else	{
				score = 0;
			}
		}

		toRet.insert(candscore(candidate, score));
	}

	return std::pair<ordering, bool>(toRet, false);
}

mdd_set::mdd_set(bool sum_defeats_in) : pairwise_method(CM_WV) {
	sum_defeats = sum_defeats_in;
	update_name();
}

std::string mdd_set::pw_name() const {
	std::string base = "MDD";
	if (sum_defeats) {
		base += "(sum)";
	} else	{
		base += "(direct)";
	}

	return (base);
}

