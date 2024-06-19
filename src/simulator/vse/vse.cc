#include "vse.h"
#include "../../tools/ballot_tools.h"

#include <numeric>
#include <iostream>

double vse_sim::do_simulation() {

	// Generate an election from the generator.
	// Determine mean utility for each candidate.
	// Call the method to get an outcome.
	// E[chosen] = mean over candidates coming in first.
	// E[random] = mean over the whole vector.


	// For this rough prototype.
	size_t numcands = 5, numvoters = 99;

	election_t ballots = ballot_gen->generate_ballots(numvoters,
			numcands, *entropy_source);

	std::vector<double> total_utility(numcands, 0);

	for (const ballot_group & g: ballots) {
		for (const candscore & cs: g.contents) {
			total_utility[cs.get_candidate_num()] +=
				g.get_weight() * cs.get_score();
		}
	}

	ordering outcome = method->elect(ballots, numcands, true);

	std::vector<int> winners = ordering_tools::get_winners(outcome);

	double chosen_utilities = 0, mean_utilities = 0;

	for (int winner: winners) {
		chosen_utilities += total_utility[winner] / (double)numvoters;
	}

	mean_utilities = std::accumulate(total_utility.begin(),
			total_utility.end(), 0.0) / (double)(numvoters *
			total_utility.size());

	return chosen_utilities - mean_utilities;
}
