
#include "set_webster.h"

#include "coalitions/coalitions.h"
#include "tools/combinations.h"

double set_webster::get_min_disallowed_factor(
	combo::it outcome_start, combo::it outcome_end,
	const std::vector<coalition_data> & solid_coalitions) const {

	// The algorithm works as follows: We start with a max
	// permissible factor of infinity. For each solid coalition,
	// we determine its intersection with our proposed outcome.
	// If this is the whole solid coalition, then the coalition
	// isn't binding (due to the min |S| factor) and we skip.

	// Otherwise, let the number of candidates in the
	// intersection be C. Then there exists a maximal factor
	// x_max so that round(support * x_max) = C.
	// This occurs at support * x_max = (C + 0.5 - epsilon), i.e.
	// x_max = (C + 0.5 - epsilon) / support, so that the
	// minimum disallowed factor is (C + 0.5) / support.

	// If the current disallowed minimum is greater than this,
	// we reduce it.

	std::set<size_t> outcome(outcome_start, outcome_end);

	double min_disallowed = std::numeric_limits<double>::infinity();

	for (auto cc_pos = solid_coalitions.begin();
		cc_pos != solid_coalitions.end(); ++cc_pos) {

		std::set<size_t> proposed_in_coalition;

		std::set_intersection(
			outcome.begin(), outcome.end(),
			cc_pos->coalition.begin(), cc_pos->coalition.end(),
			std::inserter(proposed_in_coalition,
				proposed_in_coalition.begin()));

		if (proposed_in_coalition.size() == cc_pos->coalition.size()) {
			continue; // Not binding
		}

		double min_disallowed_here = (proposed_in_coalition.size()
				+ 0.5) / cc_pos->support;

		min_disallowed = std::min(min_disallowed,
				min_disallowed_here);
	}

	return min_disallowed;
}

council_t set_webster::get_council(size_t council_size,
	size_t num_candidates, const election_t & ballots) const {

	// Get the solid coalitions.
	std::vector<coalition_data> solid_coalitions =
		get_solid_coalitions(ballots,
			std::vector<bool>(num_candidates, true),
			num_candidates);

	// Determine the council/s with the greatest minimum
	// infeasible factor (and thus, unless there are ties,
	// the greatest maximum feasible factor).

	double record_minimum_factor =
		-std::numeric_limits<double>::infinity();
	std::vector<std::vector<size_t> > best_outcomes;

	combination_generator combo(num_candidates, council_size);
	do {
		double proposed_minimum = get_min_disallowed_factor(
				combo.begin(), combo.end(), solid_coalitions);
		if (proposed_minimum > record_minimum_factor) {
			best_outcomes.clear();
		}
		if (proposed_minimum >= record_minimum_factor) {
			best_outcomes.push_back(std::vector<size_t>(
					combo.begin(), combo.end()));
			record_minimum_factor = proposed_minimum;
		}
	} while (combo.next());

	// TODO: This only picks "a" solution. We don't know if it's unique.
	// Do something about it. And about this ugly conversion.
	// Perhaps some kind of tiebreak?

	std::vector<size_t> om = best_outcomes[0];

	council_t output;
	std::copy(om.begin(), om.end(), std::back_inserter(output));

	return output;
}
