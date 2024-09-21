// More multiwinner hacks/clustering tests.

#include "multiwinner/methods/all.h"
#include "generator/spatial/all.h"

#include "random/random.h"
#include "tools/tools.h"

#include "multiwinner/helper/errors.h"

#include "multiwinner/pr_measures/clustering.h"
#include "multiwinner/pr_measures/normal_fit.h"

// normsum, etc.

int main() {
	rng rnd(1);
	gaussian_generator gauss;
	gauss.set_params(5, false); // 5D spatial, say.
	std::cout << gauss.get_num_dimensions() << std::endl;
	gauss.set_dispersion(1);

	size_t num_voters = 4096;
	size_t num_candidates = 50;

	size_t num_clusters = 2; // say

	size_t maxiters = 5000;

	std::cout << gauss.pdf(std::vector<double>(5, 0.1)) << "\n";

	cluster_proportionality test(num_clusters);
	normal_proportionality ntest(gauss);

	for (double delta = 0.1; delta <= 1; delta += 0.1) {

		double error = 0;
		for (int i = 0; i < maxiters; ++i) {

			std::cerr << i << "/" << maxiters << "    \r" << std::flush;

			positions_election p_e = gauss.generate_election_result(
					num_voters, num_candidates, false, rnd);

			ntest.prepare(p_e);

			// Elect using, say, QPQ.
			size_t num_seats = 7;

			std::list<size_t> qpq_council = QPQ(delta, true).get_council(
					num_seats, num_candidates, p_e.ballots);

			error += ntest.get_error(qpq_council);
		}

		std::cerr << "\n";
		std::cout << delta << "\t" << error << std::endl;
	}
}