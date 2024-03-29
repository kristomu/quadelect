#include "mdd.h"

std::pair<ordering, bool> mdd_set::pair_elect(const abstract_condmat &
	input,
	const std::vector<bool> & hopefuls, cache_map * cache,
	bool winner_only) const {

	std::vector<int> num_defeats(input.get_num_candidates(), 0);

	int counter, sec;

	for (counter = 0; counter < input.get_num_candidates(); ++counter) {
		if (!hopefuls[counter]) {
			continue;
		}
		for (sec = 0; sec < input.get_num_candidates(); ++sec) {
			if (counter == sec) {
				continue;
			}
			if (!hopefuls[sec]) {
				continue;
			}

			if (input.get_magnitude(counter, sec, hopefuls) >
				input.get_num_voters() * 0.5) {
				++num_defeats[sec];
			}
		}
	}

	ordering toRet;

	for (counter = 0; counter < input.get_num_candidates(); ++counter) {
		if (!hopefuls[counter]) {
			continue;
		}

		int score;
		if (sum_defeats) {
			score = -num_defeats[counter];
		} else {
			if (num_defeats[counter] > 0) {
				score = -1;
			} else	{
				score = 0;
			}
		}

		toRet.insert(candscore(counter, score));
	}

	return (std::pair<ordering, bool>(toRet, false));
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

