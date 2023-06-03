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
#include "../singlewinner/elimination.h"
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

#include "../bandit/bandit.h"
#include "../bandit/binomial.h"
#include "../bandit/lilucb.h"

#include "../bandit/tests/reverse.h"

#include "strat_test.h"

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

void test_with_bandits(std::vector<election_method *> & to_test,
	rng & randomizer, pure_ballot_generator * & ballotgen,
	pure_ballot_generator * strat_generator, bool find_most_susceptible) {

	std::vector<BinomialBandit> bandits;

	// sts: Tests that count how many times we find a strategy
	// reverse_sts: Tests that count how many times we fail.

	// Since bandits try to maximize the proportion (reward), using sts
	// will find the methods that resist strategy best, while using
	// reverse_sts will find those that are most susceptible.

	std::vector<strategy_test> sts;
	std::vector<ReverseTest> reverse_sts;

	int numvoters = 97;
	int initial_numcands = 3, numcands = initial_numcands;

	std::cout << "Number of candidates = " << numcands << std::endl;

	size_t i;

	for (i = 0; i < to_test.size(); ++i) {
		// Use lower values to more quickly identify the best method.
		// Use higher values to get more accurate strategy
		// susceptibility values (i.e. succumbs to strategy 10% of
		// the time).
		int tries_to_get_strat = 2053; // was 4096

		sts.push_back(strategy_test(ballotgen, strat_generator,
				numvoters, numcands, numcands, randomizer, to_test[i], 0,
				tries_to_get_strat));
	}

	// Needs to be done this way because inserting stuff into sts can
	// alter pointers.
	for (i = 0; i < sts.size(); ++i) {
		reverse_sts.push_back(ReverseTest(&sts[i]));
	}

	for (i = 0; i < to_test.size(); ++i) {
		if (find_most_susceptible) {
			bandits.push_back(BinomialBandit(&reverse_sts[i]));
		} else {
			bandits.push_back(BinomialBandit(&sts[i]));
		}
	}

	Lil_UCB lil_ucb;
	lil_ucb.load_bandits(bandits);

	bool confident = false;

	double num_methods = sts.size();
	double report_significance = 0.05;
	// Bonferroni correction
	double corrected_significance = report_significance/num_methods;
	//double zv = ppnd7(1-report_significance/num_methods);

	time_t startpt = time(NULL);
	confidence_int ci;

	for (int j = 1; j < 1000000 && !confident; ++j) {
		// Bleh, why is min a macro?
		int num_tries = std::max(100, (int)sts.size());
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
				so_far.push_back(std::pair<double, int>(bandits[k].get_mean(), k));
			}
			sort(so_far.begin(), so_far.end());
			reverse(so_far.begin(), so_far.end());

			size_t how_many = 10;
			std::cout << "Interim report (by mean):" << std::endl;
			for (k = 0; k < std::min(how_many, sts.size()); ++k) {
				double mean = bandits[so_far[k].second].get_mean();

				int num_pulls = bandits[so_far[k].second].get_num_pulls();
				int num_successes = bandits[so_far[k].second].get_num_successes();

				std::pair<double, double> c_i = ci.bin_prop_interval(
						corrected_significance, num_successes, num_pulls);
				double lower = round((1 - c_i.second) * 1000)/1000.0;
				double middle = round((1 - mean) * 1000)/1000.0;
				double upper = round((1 - c_i.first) * 1000)/1000.0;

				std::cout << k+1 << ". " << bandits[so_far[k].second].name() << "(" <<
					lower << ", " << middle << ", " << upper << ")" << std::endl;
			}

		}
	}

	const Bandit * results = lil_ucb.get_best_bandit_so_far();

	std::cout << "Best so far is " << results->name() <<
		std::endl; //<< " with CB of " << results.second << std::endl;
	std::cout << "It has a mean of " << results->get_mean() << std::endl;

}

int main(int argc, const char ** argv) {
	std::vector<election_method *> condorcets;
	std::vector<election_method *> condorcetsrc;

	int counter;

	int radix=9;

	for (int j = 0; j < pow(radix, 6); ++j) {
		cond_brute cbp(j, radix);

		if (cbp.is_monotone()
			&& cbp.passes_mat() /*&& cbp.reversal_symmetric()*/) {
			condorcetsrc.push_back(new cond_brute(j, radix));
			std::cout << "Adding " << j << std::endl;
		}
	}

	condorcet_set xd;

	for (counter = condorcetsrc.size()-1; counter >= 0; --counter) {
		condorcets.push_back(new comma(condorcetsrc[counter], &xd));
	}

	reverse(condorcets.begin(), condorcets.end());

	std::cout << "There are " << condorcets.size() << " methods." << std::endl;

	// Generate separate RNGs for each thread.
	const int numthreads = 16;
	std::vector<rng> randomizers;
	time_t startpt = time(NULL);    // randomize timer

	for (counter = 0; counter < numthreads; ++counter) {
		randomizers.push_back(rng(startpt + counter));
	}

	pure_ballot_generator * ballotgen = new impartial(true, false);
	// Something is wrong with this one. TODO: Check later.
	//ballotgens.push_back(new dirichlet(false));
	// And this one; it trips num_prefers_challenger - cumul. Determine why.
	// int dimensions = 4;
	//ballotgens.push_back(new gaussian_generator(true, false, dimensions,
	//		false));

	test_with_bandits(condorcets, randomizers[0], ballotgen, ballotgen,
		false);

	return (0);
}
