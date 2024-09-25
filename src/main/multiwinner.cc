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

#include <unordered_map>

// normsum, etc.

class VSE_limits {
	private:
		double worst, random, best;
		bool seen_worst, seen_best;
		size_t times_updated;

	public:
		void clear() {
			worst = 0;
			random = 0;
			best = 0;
			times_updated = 0;
			seen_worst = false;
			seen_best = false;
		}

		void update(double error) {
			if (!seen_worst || error > worst) {
				worst = error;
				seen_worst = true;
			}

			if (!seen_best || error < best) {
				best = error;
				seen_best = true;
			}

			random += error;
			++times_updated;
		}

		VSE_limits() {
			clear();
		}

		double get_worst() const {
			return worst;
		}
		double get_random() const {
			return random/(double)times_updated;
		}
		double get_best() const {
			return best;
		}
};

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
	std::iota(candidates.begin(), candidates.end(), 0);

	VSE_limits limits;

	auto evaluate = // TODO, const measure
		[&limits, &measure](std::vector<size_t>::const_iterator start,
	std::vector<size_t>::const_iterator end) -> bool {

		// TODO: measures: templating and iterators...
		limits.update(measure.get_error(
				std::list<size_t>(start, end)));

		// keep going
		return false;
	};

	if (log_combinations <= log(max_iters)) {
		for_each_combination(candidates.begin(),
			candidates.begin() + num_seats, candidates.end(),
			evaluate);
	} else {
		for (size_t i = 0; i < max_iters; ++i) {
			// make this possible later.
			//std::shuffle(candidates.begin(), candidates.end(), rnd);
			std::random_shuffle(candidates.begin(), candidates.end());
			evaluate(candidates.begin(), candidates.begin() + num_seats);
		}
	}

	//std::cout << "test: best " << limits.get_best() << " random: " << limits.get_random() << " worst: " << limits.get_worst() << "\n";

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

	std::cout << gauss.pdf(std::vector<double>(5, 0.1)) << "\n";

	cluster_proportionality test(num_clusters);
	normal_proportionality ntest(gauss);
	binary_proportionality btest;

	std::unordered_map<double, VSE> vse_per_delta;
	std::unordered_map<double, double> error_per_delta;

	for (size_t i = 0; i < maxiters; ++i) {

		bingen.bias_generator(num_issues, rnd);

		std::cerr << i << "/" << maxiters << "    \r" << std::flush;

		positions_election p_e = bingen.generate_election_result(
				num_voters, num_candidates, false, rnd);

		btest.prepare(p_e);

		size_t num_seats = 7;

		VSE_limits limits = get_proportionality_limits(btest,
				num_candidates, num_seats, rnd, 1024);

		for (double delta = 0.1; delta <= 1; delta += 0.1) {

			// Elect using, say, QPQ.

			std::list<size_t> qpq_council = QPQ(delta, true).get_council(
					num_seats, num_candidates, p_e.ballots);

			double disprop = btest.get_error(qpq_council);

			vse_per_delta[delta].add_result(limits.get_random(),
				disprop, limits.get_best());

			error_per_delta[delta] += disprop;
		}
	}


	for (double delta = 0.1; delta <= 1; delta += 0.1) {
		std::cerr << "\n";
		std::cout << delta << "\t" << error_per_delta[delta] << " (VSE: " <<
			vse_per_delta[delta].get() << ")" << std::endl;
	}
}
