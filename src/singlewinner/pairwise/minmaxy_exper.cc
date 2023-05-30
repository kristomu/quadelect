
#include "minmaxy_exper.h"

#include "../positional/simple_methods.h"

#include <iostream>
#include <list>

std::pair<ordering, bool> minmaxy_experimental::pair_elect(
	const abstract_condmat & input, const std::vector<bool> & hopefuls,
	cache_map * cache, bool winner_only) const {

	// Each candidate A has a leximax ordering over all other candidates X
	// std::min(X>A, A>X) - X>A. This is then sorted in order of smallest to
	// greatest, and the maximum wins. Note that a CW has no negative
	// components and so wins outright.

	// Intended for pairwise opposition. WV would suck.

	std::vector<std::pair<std::list<double>, size_t> > score_lists;

	size_t i, j;

	for (i = 0; i < input.get_num_candidates(); ++i) {
		if (!hopefuls[i]) {
			continue;
		}
		std::pair<std::list<double>, size_t> next_score_element(
			std::list<double>(), i);
		for (j = 0; j < input.get_num_candidates(); ++j) {
			if (!hopefuls[j]) {
				continue;
			}
			double AoverX = input.get_magnitude(i, j, hopefuls);
			double XoverA = input.get_magnitude(j, i, hopefuls);
			next_score_element.first.push_back(
				std::min(AoverX, XoverA)-XoverA);
		}
		next_score_element.first.sort();
		score_lists.push_back(next_score_element);
	}

	std::sort(score_lists.begin(), score_lists.end());

	ordering out;
	int score = 0;

	for (i = 0; i < score_lists.size(); ++i) {
		if (!hopefuls[score_lists[i].second]) {
			continue;
		}
		if (i > 0 && score_lists[i].first > score_lists[i-1].first) {
			++score;
		}
		out.insert(candscore(score_lists[i].second, score));
	}

	return (std::pair<ordering, bool>(out, false));
}

