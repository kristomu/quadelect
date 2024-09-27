// More multiwinner hacks/clustering tests.

#include "multiwinner/methods/all.h"
#include "generator/spatial/all.h"

#include "random/random.h"
#include "tools/tools.h"

#include "multiwinner/helper/errors.h"

#include "multiwinner/pr_measures/binary.h"
#include "multiwinner/pr_measures/clustering.h"
#include "multiwinner/pr_measures/normal_fit.h"

#include "stats/multiwinner/vse.h"
#include "stats/multiwinner/vse_limits.h"

#include <unordered_map>

// return the natural log of x!
double lfac(size_t x) {
	return lgamma(x+1);
}

VSE_limits get_proportionality_limits(
	proportionality_measure & measure,
	size_t num_candidates, size_t num_seats,
	rng & rnd, size_t max_iters) {

	// Check if an exhaustive search would exceed our iteration budget.

	double log_combinations = lfac(num_candidates) - (
			lfac(num_seats) + lfac(num_candidates-num_seats));

	std::vector<size_t> candidates(num_candidates, 0);
	std::vector<size_t> council(num_seats, 0);
	std::iota(candidates.begin(), candidates.end(), 0);

	VSE_limits limits(MINIMIZE); // higher error is worse

	auto evaluate = // TODO, const measure
		[&limits, &measure, &council](
			std::vector<size_t>::const_iterator start,
	std::vector<size_t>::const_iterator end) -> bool {

		std::copy(start, end, council.begin());

		// TODO: measures: templating and iterators...
		limits.update(measure.get_error(council));

		// keep going
		return false;
	};

	if (log_combinations <= log(max_iters)) {
		for_each_combination(candidates.begin(),
			candidates.begin() + num_seats, candidates.end(),
			evaluate);
	} else {
		for (size_t i = 0; i < max_iters; ++i) {
			std::shuffle(candidates.begin(), candidates.end(), rnd);
			evaluate(candidates.begin(), candidates.begin() + num_seats);
		}
	}

	return limits;
}

int main() {
	rng rnd(1);
	gaussian_generator gauss;
	gauss.set_params(5, false); // 5D spatial, say.
	std::cout << gauss.get_num_dimensions() << std::endl;
	gauss.set_dispersion(1);

	// Or for binary testing...
	size_t num_issues = 5;
	bernoulli_generator bingen;

	size_t num_voters = 4096;
	size_t num_candidates = 10;

	size_t num_clusters = 2; // say

	size_t maxiters = 50000;
	size_t delta_spacing = 10;

	cluster_proportionality test(num_clusters);
	normal_proportionality ntest(gauss);
	binary_proportionality btest;

	std::unordered_map<double, VSE> vse_per_delta;
	std::unordered_map<double, double> error_per_delta;

	for (size_t i = 0; i < maxiters; ++i) {

		bingen.bias_generator(num_issues, rnd);

		if ((i & 15) == 0) {
			std::cerr << i << "/" << maxiters << "    \r" << std::flush;
		}

		positions_election p_e = bingen.generate_election_result(
				num_voters, num_candidates, false, rnd);

		btest.prepare(p_e);

		size_t num_seats = 7;

		VSE_limits limits = get_proportionality_limits(btest,
				num_candidates, num_seats, rnd, 1024);

		for (size_t j = 1; j <= delta_spacing; ++j) {
			double delta = j / (double)delta_spacing;

			// Elect using, say, QPQ.

			std::vector<size_t> qpq_council = QPQ(delta, true).get_council(
					num_seats, num_candidates, p_e.ballots);

			double disprop = btest.get_error(qpq_council);

			vse_per_delta[delta].add_result(limits.get_random(),
				disprop, limits.get_best());

			error_per_delta[delta] += disprop;
		}
	}


	std::cerr << "\n";
	for (size_t j = 1; j <= delta_spacing; ++j) {
		double delta = j / (double)delta_spacing;
		std::cout << delta << "\t" << error_per_delta[delta] << " (VSE: " <<
			vse_per_delta[delta].get() << ")" << std::endl;
	}
}
