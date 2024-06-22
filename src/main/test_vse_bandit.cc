#include <iostream>
#include <list>
#include <memory>
#include <vector>

#include "../tests/quick_dirty/monotonicity.h"
#include "../simulator/vse/vse.h"

#include "../tools/ballot_tools.h"
#include "../ballots.h"

#include "../singlewinner/elimination/all.h"
#include "../singlewinner/experimental/all.h"
#include "../singlewinner/meta/all.h"
#include "../singlewinner/pairwise/simple_methods.h"
#include "../singlewinner/sets/all.h"
#include "../random/random.h"

#include "../generator/all.h"

#include "../stats/quasirandom/r_sequence.h"

#include "../bandit/lilucb.h"
#include "../singlewinner/get_methods.h"

void test_with_bandits(
	std::vector<std::shared_ptr<election_method> > & to_test,
	std::shared_ptr<coordinate_gen> randomizer,
	int numcands, int numvoters, double E_opt_random,
	int dimensions, double sigma) {

	std::vector<std::shared_ptr<simulator> > sims;

	std::cout << "Number of candidates = " << numcands << std::endl;

	size_t i;

	for (i = 0; i < to_test.size(); ++i) {
		auto sim = std::make_shared<vse_sim>(randomizer, to_test[i],
				numcands, numvoters, dimensions);

		sim->set_scale_factor(1/E_opt_random);
		sim->set_dispersion(sigma);

		sims.push_back(sim);
	}

	Lil_UCB lil_ucb;
	lil_ucb.load_arms(sims);

	// The stuff below needs a cleanup! TODO
	// It's better now.

	bool confident = false;

	time_t start_time = time(NULL);

	while (!confident) {
		double progress = lil_ucb.timed_pull_bandit_arms(10);
		if (progress == 1) {
			std::cout << "Managed in " << lil_ucb.get_total_num_pulls() <<
				" tries." << std::endl;
			std::cout << "That took " << time(NULL) - start_time << " seconds."
				<< std::endl;
			confident = true;
		}

		std::vector<std::pair<double, int> > so_far;

		// Warning: No guarantee that this will contain the best until
		// the method is finished; and no guarantee that it will contain
		// the best m even when the method is finished for m>1. (We
		// should really have it work for m>1 somehow. See "k-top"
		// bandits.)

		// NOTE: The array will be sorted in order of mean score, which
		// might not be the same thing as the bandit priority order since
		// the latter takes confidence into account.
		size_t k;
		for (k = 0; k < sims.size(); ++k) {
			double score = sims[k]->get_mean_score();
			// We're going to sort the list of sims so that the best come
			// on top. Since the sort is ascending, we need to negate the
			// score if higher is better, so that the sort puts the best
			// (highest scoring in that case) sims on top.
			if (sims[k]->higher_is_better()) {
				score = -score;
			}
			so_far.push_back(std::pair<double, int>(score, k));
		}
		sort(so_far.begin(), so_far.end());

		size_t how_many = 10;
		std::cout << "Current bandit testing progress = "
			<< progress << std::endl;
		std::cout << "Interim report (by mean):" << std::endl;
		for (k = 0; k < std::min(how_many, sims.size()); ++k) {
			double mean = sims[so_far[k].second]->get_mean_score();

			auto test = sims[so_far[k].second];
			/*			double lower = test->get_lower_confidence(
								report_significance, num_methods);
						double upper = test->get_upper_confidence(
								report_significance, num_methods);*/

			std::cout << k+1 << ". " << test->name() << "\t"
				<< round(mean, 4) << std::endl;
		}
	}
}

int main() {
	int numvoters = 99, numcands = 4; // E.g.
	int dimensions = 4;
	double sigma = 1;

	// TODO get seed from an entropy source, see quadelect proper
	//std::shared_ptr<rng> rnd = std::make_shared<rng>(0);
	std::shared_ptr<coordinate_gen> rnd = std::make_shared<rng>(0);

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

	// Bandit time

	std::vector<std::shared_ptr<election_method> > methods =
		get_singlewinner_methods(false, false);

	std::vector<std::shared_ptr<election_method> > methods_to_test;

	condorcet_set xd;
	std::shared_ptr<smith_set> smith = std::make_shared<smith_set>();
	schwartz_set xf;
	std::shared_ptr<inner_burial_set> ibs =
		std::make_shared<inner_burial_set>();

	for (auto & m: methods) {
		methods_to_test.push_back(m);
		if (m->name().find("Smith") == std::string::npos) {
			methods_to_test.push_back(std::make_shared<comma>(
					smith, m));
			methods_to_test.push_back(std::make_shared<slash>(
					smith, m));
		}
		methods_to_test.push_back(std::make_shared<slash>(
				ibs, m));
		/*methods_to_test.push_back(std::make_shared<slash>(
				std::make_shared<inner_burial_set>(), m));*/
		methods_to_test.push_back(m);
	}

	std::cout << "That's " << methods_to_test.size()
		<< " methods." << std::endl;

	test_with_bandits(methods_to_test, rnd,
		numcands, numvoters, opt_less_random,
		dimensions, sigma);

	return 0;
}
