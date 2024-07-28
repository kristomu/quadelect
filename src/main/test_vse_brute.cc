#include <iostream>
#include <list>
#include <memory>
#include <vector>

#include "../tests/quick_dirty/monotonicity.h"
#include "../simulator/all.h"

#include "../tools/ballot_tools.h"
#include "../ballots.h"

#include "../singlewinner/elimination/all.h"
#include "../singlewinner/experimental/all.h"
#include "../singlewinner/meta/all.h"
#include "../singlewinner/pairwise/all.h"
#include "../singlewinner/sets/all.h"
#include "../random/random.h"

#include "../generator/all.h"

#include "../stats/quasirandom/r_sequence.h"

int main() {

	int numvoters = 99, numcands = 4; // E.g.
	int dimensions = 4;

	// TODO get seed from an entropy source, see quadelect proper
	//std::shared_ptr<rng> rnd = std::make_shared<rng>(0);
	std::shared_ptr<coordinate_gen> rnd = std::make_shared<rng>(0);

	auto method_tested =
		std::make_shared<ext_minmax>(CM_WV, false);
	/*
	std::make_shared<slash>(std::make_shared<inner_burial_set>(),
		std::make_shared<schulze>(CM_WV));*/

	// Calculate E[optimal] - E[random].
	// This is ugly because the VSE sim is forced to use
	// Gaussians anyway... so I have to instantiate one just to
	// calculate the constant.

	gaussian_generator const_gen(false, false, dimensions, false);
	const_gen.set_dispersion(1);

	int iters = 32768000;

	std::cout << "Calculating E[optimal]-E[random]..." << std::flush;

	double optimal = const_gen.get_optimal_utility(
			*rnd, numvoters, numcands, iters);
	double random = const_gen.get_mean_utility(
			*rnd, numvoters, numcands, iters);

	double opt_less_random = optimal - random;

	std::cout << "done.\n";
	std::cout << "E[optimal] - E[random] ~= "
		<< opt_less_random << std::endl;

	std::cout << "Time to do VSE things to " << method_tested->name()
		<< ".\n\n\n";

	vse_sim checker(rnd, method_tested,
		numcands, numvoters, dimensions);
	checker.set_scale_factor(1/opt_less_random);

	checker.set_dispersion(1); // JGA stylee

	for (size_t i = 0;; ++i) {
		double quality = checker.simulate(true);

		if ((i & 8191) == 0) {
			std::cout << "\tResults: "
				<< "Exact: " << checker.get_mean_score() << "\t "
				<< "Approx: " << checker.get_adjusted_linear_value() << "\t"
				<< "Last: " << quality << std::endl;
		}
	}

	return 0;
}
