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

	std::vector<std::shared_ptr<bernoulli_simulator> > sims;

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
		int tries_to_get_strat = 512; // was 4096

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
	std::vector<std::shared_ptr<election_method> > methods_to_test;

	std::vector<std::shared_ptr<election_method> > methods =
		get_singlewinner_methods(false, false);

	condorcet_set xd;
	smith_set xe;
	schwartz_set xf;
	ifpp_method_x si;

	for (auto & m: methods) {
		methods_to_test.push_back(std::make_shared<comma>(
				std::make_shared<smith_set>(), m));
		methods_to_test.push_back(std::make_shared<slash>(
				std::make_shared<smith_set>(), m));
		methods_to_test.push_back(m);
	}

	std::cout << "There are " << methods_to_test.size()
		<< " methods." << std::endl;

	std::shared_ptr<rng> randomizer;
	time_t startpt = time(NULL);    // randomize timer
	randomizer = std::make_shared<rng>(startpt);

	// !!! If you set the first parameter to true, then the monotonicity
	// checker explodes. Don't do that. TODO: Error handling...
	std::shared_ptr<pure_ballot_generator> ballotgen =
		std::make_shared<impartial>(false, false);


	test_with_bandits(methods_to_test, randomizer,
		ballotgen, false);

	return 0;
}
