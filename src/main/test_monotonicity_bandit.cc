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

#include "../singlewinner/positional/as241.h"

#include "../generator/all.h"


#include "../bandit/bandit.h"
#include "../bandit/binomial.h"
#include "../bandit/lilucb.h"

#include "../bandit/tests/reverse.h"

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
	std::shared_ptr<pure_ballot_generator> ballotgen) {

	std::vector<std::shared_ptr<monotone_check> > tests;
	std::vector<std::shared_ptr<BinomialBandit> > bandits;

	int max_numvoters = 100, max_numcands = 4;

	std::cout << "Max number of candidates = " << max_numcands << std::endl;
	std::cout << "Max number of voters = " << max_numvoters << std::endl;

	size_t i;

	for (i = 0; i < to_test.size(); ++i) {
		tests.push_back(std::make_shared<monotone_check>(
				ballotgen, randomizer, to_test[i], max_numcands,
				max_numvoters));
	}

	for (i = 0; i < to_test.size(); ++i) {
		std::shared_ptr<Test> test_to_add = std::shared_ptr<Test>(tests[i]);

		bandits.push_back(std::make_shared<BinomialBandit>(test_to_add));
	}

	Lil_UCB lil_ucb;
	lil_ucb.load_bandits(bandits);

	// The stuff below needs a cleanup! TODO

	bool confident = false;

	double num_methods = tests.size();
	double report_significance = 0.05;
	// Bonferroni correction
	double corrected_significance = report_significance/num_methods;
	//double zv = ppnd7(1-report_significance/num_methods);

	time_t startpt = time(NULL);
	confidence_int ci;

	for (int j = 1; j < 1000000 && !confident; ++j) {
		// Bleh, why is min a macro?
		int num_tries = std::max(100, (int)tests.size());
		// Don't run more than 20k at a time because otherwise
		// feedback is too slow.
		//num_tries = std::min(40000, num_tries);
		double progress = lil_ucb.pull_bandit_arms(num_tries);
		if (progress == 1) {
			std::cout << "Managed in fewer than " << j * num_tries <<
				" tries." << std::endl;
			confident = true;
		} else {
			if (time(NULL) - startpt < 2) {
				continue;
			}
			std::cout << time(NULL) - startpt << "s." << std::endl;
			std::cout << "After " << j * num_tries << ": " << progress << std::endl;
			std::vector<std::pair<double, int> > so_far;
			if (time(NULL) - startpt < 5) {
				continue;
			}
			startpt = time(NULL);
			// Warning: No guarantee that this will contain the best until
			// the method is finished; and no guarantee that it will contain
			// the best m even when the method is finished for m>1. (We
			// should really have it work for m>1 somehow.)
			size_t k;
			for (k = 0; k < bandits.size(); ++k) {
				// TODO: Some kind of get C here...
				so_far.push_back(std::pair<double, int>(bandits[k]->get_mean(), k));
			}
			sort(so_far.begin(), so_far.end());
			reverse(so_far.begin(), so_far.end());

			size_t how_many = 10;
			std::cout << "Interim report (by mean):" << std::endl;
			for (k = 0; k < std::min(how_many, tests.size()); ++k) {
				double mean = bandits[so_far[k].second]->get_mean();

				int num_pulls = bandits[so_far[k].second]->get_num_pulls();
				int num_successes = bandits[so_far[k].second]->get_num_successes();

				std::pair<double, double> c_i = ci.bin_prop_interval(
						corrected_significance, num_successes, num_pulls);
				double lower = round((c_i.first) * 1000)/1000.0;
				double middle = round((mean) * 1000)/1000.0;
				double upper = round((c_i.second) * 1000)/1000.0;

				std::cout << k+1 << ". " << bandits[so_far[k].second]->name() << "(" <<
					lower << ", " << middle << ", " << upper << ")" << std::endl;
			}

		}
	}

	std::shared_ptr<Bandit> results = lil_ucb.get_best_bandit_so_far();

	std::cout << "Best so far is " << results->name() <<
		std::endl; //<< " with CB of " << results.second << std::endl;
	std::cout << "It has a mean of " << results->get_mean() << std::endl;

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


	test_with_bandits(condorcets, randomizer, ballotgen);

	return 0;
}
