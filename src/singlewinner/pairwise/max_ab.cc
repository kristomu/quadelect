
#include "max_ab.h"

#include "../positional/simple_methods.h"

#include <iostream>
#include <list>

// This seems awfully verbose for what is essentially a simple method.
// Is there any better way to do it?

std::pair<ordering, bool> max_ab::pair_elect(
	const abstract_condmat & input, const std::vector<bool> & hopefuls,
	cache_map * cache, bool winner_only) const {

	std::vector<std::pair<std::list<double>, size_t> > score_lists;
	std::vector<double> maxbeat(input.get_num_candidates(), 0);

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
			maxbeat[i] = std::max(maxbeat[i], AoverX);
			next_score_element.first.push_back(AoverX);
		}
		next_score_element.first.sort();
		next_score_element.first.reverse();
		score_lists.push_back(next_score_element);
	}

	std::sort(score_lists.begin(), score_lists.end());
	std::reverse(score_lists.begin(), score_lists.end());

	ordering out;
	int score = 0;

	for (i = 0; i < score_lists.size(); ++i) {
		if (!hopefuls[score_lists[i].second]) {
			continue;
		}
		if (i > 0 && score_lists[i-1].first > score_lists[i].first) {
			--score;
		}
		out.insert(candscore(score_lists[i].second, score));
	}

	return (std::pair<ordering, bool>(out, false));
}