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
#include "../bandit/lilucb.h"

#include "../tests/strategy/strategies.h"
#include "../tests/runner.h"
#include "../tests/provider.h"

// default values for number of processors and current processor
// use -I compiler options to get multiproc support. TODO: make this an
// input parameter.

#ifndef __NUMPROCS__
#define __NUMPROCS__ 1
#endif

#ifndef __THISPROC__
#define __THISPROC__ 0
#endif

// Determine the the z binomial confidence interval.
// Agresti-Coull. http://www.graphpad.com/guides/prism/6/statistics/index.htm?how_to_compute_the_95_ci_of_a_proportion.htm
// TODO: use as241.c to calculate the Z directly. Make class for Bonferroni
// correction. Place everything in an object so that we don't have to bother
// about it again. Ideally we should use something more ANOVA-like than
// Bonferroni, but eh.
std::pair<double, double> confidence_interval(int n, double p_mean,
	double z) {

	double p_mark = (p_mean * n + 0.5 * z * z) / (n + z * z);

	double W = sqrt(p_mark * (1 - p_mark) / (n + z * z));

	return (std::pair<double, double>(p_mark - W, p_mark + W));
}

void test_strategy(election_method * to_test, rng & randomizer,
	int num_methods, pure_ballot_generator * ballot_gen) {

	// Generate a random ballot set.
	// First should be true: compress the ballots. Second, that's
	// whether truncation is permitted.
	// TODO later: fix last-rank problem with gradual-cond-borda.
	// Also fix memory-hogging loop of doom in positional if this is set on.
	//impartial ballot_gen(/*true, true*/true, true);
	//spatial_generator ballot_gen(true, false, 4, false);

	//int dimensions = 4;

	//gaussian_generator ballot_gen(true, false, dimensions, false);
	//spatial_generator spatial(true, false);
	impartial true_iic(true, false);
	impartial iic(true, false);
	//impartial ballot_gen(true, false);

	std::list<ballot_group> ballots;

	// A bunch of times, generate ballots and clear the cache. Then try
	// these ballots against numerous Condorcet methods. If we have
	// cached the Condorcet data, that should be faster than if we haven't,
	// but one probably needs Valgrind to see the difference.

	int counter;

	std::vector<election_method *> condorcets;

	condorcets.push_back(to_test);
	std::cerr << "Now trying " << to_test->name() << std::endl;

	cache_map cache;

	// TODO: check if that actually works.
	int numvoters = 97; // was 29
	int initial_numcands = 5, numcands = initial_numcands;

	std::map<int, std::string> rcl;
	for (counter = 0; counter < 26; ++counter) {
		std::string foo = "A";
		foo[0] = 'A' + counter;
		rcl[counter] = foo;
	}

	// --- //

	std::vector<pure_ballot_generator *> ballotgens;
	ballotgens.push_back(ballot_gen);
	//ballotgens.push_back(&iic);
	///ballotgens.push_back(new gaussian_generator(true, false, dimensions, false));
	/* ballotgens.push_back(new dirichlet(true));*/

	int tests_per_ballot = 256;
	test_provider tests;
	test_runner st(ballot_gen, &iic, numvoters, numcands, numcands,
		randomizer, to_test, 0, tests_per_ballot);

	// We're testing the rate of failure of "strategy immunity" criteria,
	// i.e. the presence of opportunities for manipulation.
	st.set_name("Strategy");

	for (auto test: tests.get_tests_by_category("Strategy")) {
		st.add_test(test);
	}

	/*	st.add_test(std::make_shared<burial>());
		st.add_test(std::make_shared<compromising>());
		st.add_test(std::make_shared<two_sided_strat>());
		st.add_test(std::make_shared<two_sided_reverse>());
		st.add_test(std::make_shared<two_sided_reverse>());
		st.add_test(std::make_shared<coalitional_strategy>());*/

	int worked = 0, f;
	int fmax = 500; //50000;
	int total_generation_attempts = 0;
	for (f = 0; f < fmax; ++f) {

		if ((f & 63) == 63) {
			std::cerr << "." << std::flush;
		}
		if ((f & 4095) == 4095) {
			std::cerr << f/(double)fmax << std::flush;
		}

		/*if (test_once(ballots, ballot_gen, numvoters, numcands,
			randomizer, total_generation_attempts, condorcets)) {
			++worked;
		}*/

		if (st.perform_test() == 0) {
			++worked;
		}
	}

	total_generation_attempts = st.get_total_generation_attempts();

	// Do a simple proportions test
	double prop = worked/(double)f;

	double significance = 0.05;
	double zv = ppnd7(1-significance/num_methods);

	std::pair<double, double> c_i = confidence_interval(f, prop, zv);

	double lower_bound = c_i.first;
	double upper_bound = c_i.second;

	lower_bound = round(lower_bound*10000)/10000.0;
	upper_bound = round(upper_bound*10000)/10000.0;

	double tiefreq = (total_generation_attempts-f)/(double)f;
	tiefreq = round(tiefreq*10000)/1000.0;

	std::cout << "Worked in " << worked << " ("<< lower_bound << ", " <<
		upper_bound
		<< ") out of " << f << " for " <<
		condorcets[0]->name() << " ties: " << total_generation_attempts-f << " ("
		<< tiefreq << ")" << std::endl;
}

int main(int argc, const char ** argv) {
	std::vector<election_method *> condorcets; // Although they aren't.
	std::vector<election_method *> condorcetsrc;

	size_t counter;

	std::vector<pairwise_ident> types;
	types.push_back(CM_WV);

	condorcetsrc.push_back(new loser_elimination(
			new plurality(PT_WHOLE), false, true));
	condorcetsrc.push_back(new plurality(PT_WHOLE));
	/*condorcetsrc.push_back(new antiplurality(PT_WHOLE));*/

	std::cout << "Test time!" << std::endl;

	smith_set xd;

	// TODO: Really fix comma. DONE, kinda.
	for (counter = 0; counter < condorcetsrc.size(); ++counter) {
//    	condorcets.push_back(new slash(condorcetsrc[counter], &xd));
		condorcets.push_back(new comma(condorcetsrc[counter], &xd));
	}

	//condorcets.push_back(new ext_minmax(CM_WV, false));
	//condorcets.push_back(new schulze(CM_WV));
	//condorcets.push_back(new ranked_pairs(CM_WV, false));
	//condorcets.push_back(new ranked_pairs(CM_WV, true));

	std::cout << "There are " << condorcets.size() << " methods." << std::endl;

	std::vector<rng> randomizers;
	randomizers.push_back(rng(1));

	std::vector<pure_ballot_generator *> ballotgens;
	int dimensions = 4;
	ballotgens.push_back(new impartial(true, false));
	// Something is wrong with this one. Check later.
	ballotgens.push_back(new gaussian_generator(true, false, dimensions,
			false));
	//ballotgens.push_back(new dirichlet(false));

	for (size_t bg = 0; bg < ballotgens.size(); ++bg) {
		std::cout << "Using ballot domain " << ballotgens[bg]->name() << std::endl;
		for (counter = 0; counter < condorcets.size(); counter ++) {
			std::cout << "\t" << counter << ": " << std::flush;
			test_strategy(condorcets[counter], randomizers[0],
				condorcets.size(), ballotgens[bg]);
		}
	}
}
