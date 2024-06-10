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

#include "../singlewinner/gradual_c_b.h"

#include "../singlewinner/stats/cardinal.h"
#include "../singlewinner/positional/positional.h"
#include "../singlewinner/positional/simple_methods.h"

#include "../stats/stats.h"

// TODO, split these. Do that after improving pairwise and implementing tte,
// though.
#include "../singlewinner/pairwise/method.h"
#include "../singlewinner/pairwise/simple_methods.h"
#include "../singlewinner/pairwise/all.h"
#include "../singlewinner/pairwise/rpairs.h"
#include "../singlewinner/pairwise/kemeny.h"
#include "../singlewinner/pairwise/odm/odm.h"
#include "../singlewinner/pairwise/odm/hits.h"
#include "../singlewinner/pairwise/odm/odm_atan.h"
#include "../singlewinner/pairwise/sinkhorn.h"
//#include "../singlewinner/pairwise/tournament.cc"
#include "../singlewinner/pairwise/least_rev.h"
#include "../singlewinner/pairwise/random/randpair.h"
#include "../singlewinner/pairwise/dodgson_approxs.h"

#include "../singlewinner/experimental/3exp.h"
#include "../singlewinner/brute_force/brute.h"
#include "../singlewinner/brute_force/bruterpn.h"

#include "../singlewinner/sets/mdd.h"
#include "../singlewinner/sets/condorcet.h"
#include "../singlewinner/sets/max_elements/all.h"
#include "../singlewinner/sets/partition.h"
#include "../singlewinner/meta/comma.h"
#include "../singlewinner/meta/slash.h"

#include "../singlewinner/stats/var_median/vmedian.h"
//#include "../singlewinner/stats/median.cc"
#include "../singlewinner/stats/mode.h"

#include "../singlewinner/young.h"
#include "../bandit/lilucb.h"

#include "../tests/strategy/strategies.h"
#include "../tests/provider.h"

#include "../singlewinner/get_methods.h"

// Testing JGA's "strategic manipulation possible" concept. Perhaps it should
// be put into ttetest instead. "A method is vulnerable to burial if, when X
// wins, candidates who prefer other candidates to X can win by ranking X
// last"...
// But this isn't just burial. It's any sort of ordering. JGA uses what we may
// call "constructive strategy", i.e. assume there's a group who all prefer
// another candidate, Y, to X. If there is a ballot they can all vote that
// will move the winner from X to Y, then the method is vulnerable.
// One might also imagine setwise generalizations of this, e.g. a coalition
// prefers any in {set} to X, and if they can all vote one way so that a member
// of the set wins, then the method is vulnerable. Assume the exact member is
// found using backroom dealing or whatnot.

void test_with_bandits(
	std::vector<std::shared_ptr<election_method> > & to_test,
	std::shared_ptr<rng> randomizer,
	std::shared_ptr<pure_ballot_generator> ballotgen,
	bool find_most_susceptible) {

	if (find_most_susceptible) {
		throw std::runtime_error("Not implemented yet due to refactor!");
	}

	std::vector<std::shared_ptr<simulator> > sims;

	int numvoters = 997;
	int initial_numcands = 5, numcands = initial_numcands;

	std::cout << "Number of candidates = " << numcands << std::endl;

	size_t i;

	test_provider tests;

	for (i = 0; i < to_test.size(); ++i) {
		// Use lower values to more quickly identify the best method.
		// Use higher values to get more accurate strategy
		// susceptibility values (i.e. succumbs to strategy 10% of
		// the time).
		int tries_to_get_strat = 4096; // was 4096

		auto sim = std::make_shared<test_runner>(ballotgen,
				numvoters, numcands, numcands, randomizer, to_test[i],
				tries_to_get_strat);
		sims.push_back(sim);

		// We're testing the rate of failure of "strategy immunity" criteria,
		// i.e. the presence of opportunities for manipulation.
		sim->set_name("Strategy");

		// Add some strategies
		for (auto test: tests.get_tests_by_category("Strategy")) {
			sim->add_test(test);
		}
	}

	Lil_UCB lil_ucb;
	lil_ucb.load_bandits(sims);

	bool confident = false;

	// Confidence intervals have been disabled for now due to refactor.
	// FIX LATER.

	//double num_methods = sts.size();
	//double report_significance = 0.05;

	// Bonferroni correction
	//double corrected_significance = report_significance/num_methods;
	//confidence_int ci;

	time_t startpt = time(NULL);

	for (int j = 1; j < 1000000 && !confident; ++j) {
		// Bleh, why is min a macro?
		int num_tries = std::max(100, (int)sims.size());
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
			for (k = 0; k < sims.size(); ++k) {
				// TODO: Some kind of get C here...
				so_far.push_back(std::pair<double, int>(sims[k]->get_mean_score(), k));
			}
			sort(so_far.begin(), so_far.end());
			reverse(so_far.begin(), so_far.end());

			size_t how_many = 10;
			std::cout << "Interim report (by mean):" << std::endl;
			for (k = 0; k < std::min(how_many, sims.size()); ++k) {
				double mean = sims[so_far[k].second]->get_mean_score();

				// Confidence interval has been disabled. TODO: FIX LATER.

				/*int num_pulls = sims[so_far[k].second]->get_num_pulls();
				int num_successes = sims[so_far[k].second]->get_num_successes();

				std::pair<double, double> c_i = ci.bin_prop_interval(
						corrected_significance, num_successes, num_pulls);
				double lower = round((1 - c_i.second) * 1000)/1000.0;
				double middle = round((1 - mean) * 1000)/1000.0;
				double upper = round((1 - c_i.first) * 1000)/1000.0;

				std::cout << k+1 << ". " << sims[so_far[k].second]->name() << "(" <<
					lower << ", " << middle << ", " << upper << ")" << std::endl;*/

				double middle = round((1 - mean) * 1000)/1000.0;

				std::cout << k+1 << ". " << sims[so_far[k].second]->name()
					<< middle << std::endl;
			}

		}
	}

	std::shared_ptr<simulator> results = lil_ucb.get_best_bandit_so_far();

	std::cout << "Best so far is " << results->name() <<
		std::endl; //<< " with CB of " << results.second << std::endl;
	std::cout << "It has a mean of " << results->get_mean_score() << std::endl;

}

int main(int argc, const char ** argv) {
	std::vector<std::shared_ptr<election_method> > condorcets;
	std::vector<std::shared_ptr<election_method> > condorcetsrc;

	std::vector<std::shared_ptr<election_method> > methods =
		get_singlewinner_methods(false, false);

	condorcet_set xd;
	smith_set xe;
	schwartz_set xf;
	ifpp_method_x si;

	condorcets.push_back(std::make_shared<comma>(
			std::make_shared<smith_set>(),
			std::make_shared<ifpp_method_x>()));
	condorcets.push_back(std::make_shared<slash>(
			std::make_shared<smith_set>(),
			std::make_shared<ifpp_method_x>()));

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
