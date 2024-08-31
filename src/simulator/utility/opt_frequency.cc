#include "opt_frequency.h"
#include "tools/ballot_tools.h"

#include <math.h>
#include <numeric>
#include <iostream>

// Very heavily based off the VSE code. Copy-pasting it like this
// is easier than doing a bunch of abstractions.

double utility_freq_sim::do_simulation() {

	// Generate an election from the generator.
	// Determine mean utility for each candidate.
	// Call the method to get an outcome.
	// E[chosen] = mean over candidates coming in first.
	// E[random] = mean over the whole vector.

	election_t election = ballot_gen->generate_ballots(numvoters,
			numcands, *entropy_source);

	std::vector<double> candidate_scores(numcands, 0);

	for (const ballot_group & g: election) {
		for (const candscore & cs: g.contents) {
			assert(cs.get_candidate_num() < numcands);
			candidate_scores[cs.get_candidate_num()] +=
				g.get_weight() * cs.get_score() / (double)numvoters;
		}
	}

	ordering outcome = method->elect(election, numcands, true);

	std::vector<size_t> winners = ordering_tools::get_winners(outcome);

	double optimum_utility = *std::max_element(candidate_scores.begin(),
			candidate_scores.end());

	size_t optimal_winners = 0;

	for (int winner: winners) {
		if (candidate_scores[winner] == optimum_utility) {
			++optimal_winners;
		}
	}

	return optimal_winners/(double)winners.size();
}