#include "vse.h"
#include "tools/ballot_tools.h"

#include <math.h>
#include <numeric>
#include <iostream>

double vse_sim::do_simulation() {

	// Generate an election from the generator.
	// Determine mean utility for each candidate.
	// Call the method to get an outcome.
	// E[chosen] = mean over candidates coming in first.
	// E[random] = mean over the whole vector.

	election_t election = ballot_gen.generate_ballots(numvoters,
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

	std::vector<int> winners = ordering_tools::get_winners(outcome);

	double chosen_utilities = 0, mean_utilities = 0;

	for (int winner: winners) {
		chosen_utilities += candidate_scores[winner] / (double)winners.size();
	}

	mean_utilities = std::accumulate(candidate_scores.begin(),
			candidate_scores.end(), 0.0) / (double)candidate_scores.size();

	exact_random += mean_utilities;
	exact_optimum += *std::max_element(candidate_scores.begin(),
			candidate_scores.end());

	return chosen_utilities - mean_utilities;

}

double vse_sim::get_exact_value(double linearized, bool total) const {
	double total_opt = exact_optimum;
	double total_utility = exact_random;

	if (!total) {
		linearized *= get_simulation_count();
	}

	return linearized / (total_opt - total_utility);
}

double vse_sim::variance_proxy() const {

	// This takes a bit of explaining.

	// The output reward has two components: the utility of the chosen
	// candidate, and the mean (random) utility across every candidate.

	// The expectation of the random utility is
	// 1/numcands sum i=1..numcands sum j=1...numvoters
	// ||location(i)-location(j)||
	// This has strictly lower variance than just
	// ||location(i)-location(j)||
	// which has a Nakagami distribution. See also
	// https://math.stackexchange.com/a/232527,
	// https://math.stackexchange.com/a/4657052.

	// The variance of one draw with new candidate and voter positions is
	// V[x] = omega * (1 - 1/m * (gamma(m+1/2)/gamma(m))^2),
	// with m = dimensions/2 and omega = 2*dimensions*sigma^2,
	// sigma being the variance of the underlying Gaussian.

	// Thus the variance of the random part is less than V[x].

	// The variance of the chosen candidate depends on the voting method,
	// which we can't model here. However, its variance is maximized if
	// the voting method just picks candidates at random, in which case
	// it'll be the same as the variance above, so we can upper bound the
	// variance this way.

	// So our variance bound is 2 V[x]. This is *very* loose and can be
	// improved by a lot in the limit of a large number of voters. Possible
	// ideas include the CLT or finding m and omega so that we get the proper
	// moments in the limit of number of voters going to infinity.

	// Consider e.g. ||X-Y||^2 --> ||X-mu||^2 as number of voters approaches
	// infinity. This would suggest omega --> dimensions * sigma^2;
	// however, the square root is done over individual pairs and not the
	// full sum.

	size_t dimensions = ballot_gen.get_num_dimensions();
	double omega = 2 * dimensions * sigma * sigma;
	double m = dimensions/2.0;

	double Vx = omega * (1 - 1.0/m * pow((tgamma(m+0.5)/tgamma(m)), 2));

	return 2 * Vx;
}