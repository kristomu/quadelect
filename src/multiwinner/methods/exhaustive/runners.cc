#include "runners.h"

#include "tools/combinations.h"

#include "multiwinner/score_aggregation/sum_scores.h"

#include "optima.h"

#include <iostream>
#include <iterator>

council_t global_exhaustive_runner::get_council(
	size_t council_size, size_t num_candidates,
	const election_t & ballots) const {

	std::vector<size_t> eligible(num_candidates);
	std::iota(eligible.begin(), eligible.end(), 0);

	std::vector<size_t> v = eligible;

	// Note that this probably alters state in the exhaustive method
	// object because we're using a const pointer, not a pointer to
	// a const. We want the method state to be mutable, though.
	method_ref->process_ballots(ballots, num_candidates);

	combination_generator combo(num_candidates, council_size);
	exhaustive_optima optimum;

	do {
		sum_scores objective_value;
		method_ref->add_method_scores(combo.begin(), combo.end(),
			objective_value);
		optimum.update(combo.begin(), combo.end(),
			objective_value);
	} while (combo.next());

	council_t out;

	for (size_t i: optimum.get_maximum_solution()) {
		out.push_back(i);
	}

	return out;
}

council_t sequential_exhaustive_runner::get_council(
	size_t council_size, size_t num_candidates,
	const election_t & ballots) const {

	std::vector<size_t> chosen;
	std::vector<bool> is_eligible(num_candidates, true);

	method_ref->process_ballots(ballots, num_candidates);

	for (size_t seat = 0; seat < council_size; ++seat) {

		exhaustive_optima optimum;

		std::vector<size_t> proposed_extension = chosen;
		proposed_extension.push_back(0);

		for (proposed_extension[seat] = 0;
			proposed_extension[seat] < num_candidates;
			++proposed_extension[seat]) {

			if (!is_eligible[proposed_extension[seat]]) {
				continue;
			}

			sum_scores objective_value;
			method_ref->add_method_scores(
				proposed_extension.begin(),
				proposed_extension.end(),
				objective_value);
			optimum.update(proposed_extension.begin(),
				proposed_extension.end(),
				objective_value);
		}

		chosen = optimum.get_maximum_solution();
		is_eligible[*chosen.rbegin()] = false;
	}

	return chosen;
}
