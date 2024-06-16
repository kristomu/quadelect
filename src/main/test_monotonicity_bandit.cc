#include <values.h>
#include <assert.h>
#include <errno.h>

#include <iterator>
#include <iostream>
#include <fstream>
#include <list>
#include <set>

#include "../tools/ballot_tools.h"
#include "../ballots.h"
#include "../tools/tools.h"

#include "../stats/confidence/as241.h"

#include "../generator/all.h"
#include "../bandit/lilucb.h"

#include "../tests/strategy/strategies.h"
#include "../tests/provider.h"

#include "../tests/quick_dirty/monotonicity.h"

#include "../singlewinner/get_methods.h"
#include "../stats/stats.h"

// Test for monotonicity failures by using multi-armed bandit.
// This is ripped from the *other* bandit tester and really needs
// a refactor. TODO.

void test_with_bandits(std::vector<std::shared_ptr<election_method> > &
	to_test, std::shared_ptr<rng> randomizer,
	std::shared_ptr<pure_ballot_generator> ballotgen,
	bool find_most_susceptible) {

	if (find_most_susceptible) {
		throw std::runtime_error("Not implemented yet due to refactor!");
	}

	std::vector<std::shared_ptr<bernoulli_simulator> > sims;

	int max_numvoters = 100, max_numcands = 4;

	std::cout << "Max number of candidates = " << max_numcands << std::endl;
	std::cout << "Max number of voters = " << max_numvoters << std::endl;

	size_t i;

	for (i = 0; i < to_test.size(); ++i) {
		sims.push_back(std::make_shared<monotone_check>(
				ballotgen, randomizer, to_test[i], max_numcands,
				max_numvoters));
	}

	Lil_UCB lil_ucb;
	lil_ucb.load_arms(sims);

	// The stuff below needs a cleanup! TODO
	// It's better now.

	bool confident = false;

	double num_methods = sims.size();
	double report_significance = 0.05;

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
			double lower = test->get_lower_confidence(
					report_significance, num_methods);
			double upper = test->get_upper_confidence(
					report_significance, num_methods);

			std::cout << k+1 << ". " << test->name() << "("
				<< round(lower, 4)	<<  ", " << round(mean, 4)
				<< ", " << round(upper, 4) <<  ")" << std::endl;
		}
	}
}

int main(int argc, const char ** argv) {
	std::vector<std::shared_ptr<election_method> > condorcets;
	std::vector<std::shared_ptr<election_method> > condorcetsrc;

	std::vector<std::shared_ptr<election_method> > methods =
		get_singlewinner_methods(true, false);

	int stepsize = 1, offset = 0;

	if (argc > 2) {

		int cand_stepsize = atoi(argv[2]);
		int cand_offset = atoi(argv[1]);

		if (cand_stepsize >= cand_offset && cand_stepsize > 1) {
			// This is an ugly hack and the confidence setting no longer
			// applies. Be sure you know what you're doing when using
			// this! (Ideally we should have a batched bandit.)

			stepsize = cand_stepsize;
			offset = cand_offset;

			std::cout << "Enabling poor man's multiprocessing mode: shard " <<
				offset << "/" << stepsize << std::endl;
		}
	}

	condorcet_set xd;
	smith_set xe;
	schwartz_set xf;
	ifpp_method_x si;

	auto rmr_ref = std::make_shared<rmr1>(RMR_TWO_WAY);
	auto rmrb_ref = std::make_shared<rmr1>(RMR_DEFEATED);
	auto rmrc_ref = std::make_shared<rmr1>(RMR_DEFEATING);

	int i = 0;

	for (auto & method: methods) {

		if (i++ % stepsize != offset) {
			continue;
		}

		std::string name = method->name();

		auto de = std::make_shared<disqelim>(method);
		condorcets.push_back(de);

		condorcets.push_back(std::make_shared<slash>(
				std::make_shared<smith_set>(),
				de));
		condorcets.push_back(std::make_shared<comma>(
				std::make_shared<smith_set>(),
				de));

		condorcets.push_back(std::make_shared<comma>(
				rmr_ref, method));
		condorcets.push_back(std::make_shared<comma>(
				rmrb_ref, method));
		condorcets.push_back(std::make_shared<comma>(
				rmrc_ref, method));
		condorcets.push_back(std::make_shared<comma>(
				std::make_shared<inner_burial_set>(), method));

	}

	std::cout << "There are " << condorcets.size() << " methods." << std::endl;

	std::shared_ptr<rng> randomizer;
	time_t startpt = time(NULL);    // randomize timer
	randomizer = std::make_shared<rng>(startpt);

	// !!! If you set the first parameter to true, then the monotonicity
	// checker explodes. Don't do that. TODO: Error handling...
	std::shared_ptr<pure_ballot_generator> ballotgen =
		std::make_shared<impartial>(false, false);


	test_with_bandits(condorcets, randomizer, ballotgen, false);

	return 0;
}
