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

#include "../singlewinner/get_methods.h"

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

// TODO?? Multiple election method support? I wouldn't have to generate a bunch
// of elections over and over per method in that case. But then the test runner
// won't work very well as a multi-armed bandit test... Ow, design can be hard
// sometimes.

// num_methods here is used for the Bonferroni correction for the confidence
// intervals.

void test_strategy(std::shared_ptr<election_method> to_test,
	rng & randomizer,
	int num_methods, std::shared_ptr<pure_ballot_generator> ballot_gen) {

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
	impartial iic(true, false);
	//impartial ballot_gen(true, false);

	// A bunch of times, generate ballots and clear the cache. Then try
	// these ballots against numerous Condorcet methods. If we have
	// cached the Condorcet data, that should be faster than if we haven't,
	// but one probably needs Valgrind to see the difference.

	std::cerr << "Now trying " << to_test->name() << std::endl;

	//cache_map cache;		// TODO: Support for this please

	// TODO: check if that actually works.
	int numvoters = 97; // was 29
	int initial_numcands = 5, numcands = initial_numcands;

	// --- //

	std::vector<std::shared_ptr<pure_ballot_generator> > ballotgens;
	ballotgens.push_back(ballot_gen);

	int tests_per_ballot = 32768;//512;
	test_provider tests;
	test_runner st(ballot_gen, numvoters, numcands, numcands,
		randomizer, to_test, tests_per_ballot);

	// We're testing the rate of failure of "strategy immunity" criteria,
	// i.e. the presence of opportunities for manipulation.
	st.set_name("Strategy");

	for (auto test: tests.get_tests_by_category("Strategy")) {
		st.add_test(test);
	}

	int worked = 0, f;
	int fmax = 2000; //50000;
	int total_generation_attempts = 0;
	for (f = 0; f < fmax; ++f) {

		if ((f & 63) == 63) {
			std::cerr << "." << std::flush;
		}
		if ((f & 4095) == 4095) {
			std::cerr << f/(double)fmax << std::flush;
		}

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
		to_test->name() << " ties: " << total_generation_attempts-f << " ("
		<< tiefreq << ")" << std::endl;
}

void get_itemized_stats(
	std::shared_ptr<election_method> to_test, rng & randomizer,
	int num_methods, std::shared_ptr<pure_ballot_generator> ballot_gen) {

	std::map<std::string, std::vector<bool> > results;

	impartial iic(true, false);

	std::cerr << "Itemized stats: Now trying " << to_test->name() << std::endl;

	int numvoters = 97;
	int initial_numcands = 5, numcands = initial_numcands;

	int tests_per_ballot = 32768;//512;
	test_provider tests;
	test_runner st(ballot_gen, numvoters, numcands, numcands,
		randomizer, to_test, tests_per_ballot);

	st.set_name("Strategy");
	for (auto test: tests.get_tests_by_category("Strategy")) {
		st.add_test(test);
	}

	size_t f, fmax = 50000, num_non_ties = 0;
	int total_generation_attempts = 0;
	for (f = 0; f < fmax; ++f) {

		if ((f & 63) == 63) {
			std::cerr << "." << std::flush;
		}
		if ((f & 4095) == 4095) {
			std::cerr << f/(double)fmax << std::flush;
		}

		std::map<std::string, bool> failures_this_election =
			st.calculate_failure_pattern();

		// If we got a tie, just skip.
		if (failures_this_election.empty()) {
			continue;
		}

		++num_non_ties;

		// Incorporate the failures into the results map.
		for (auto & failure_status: failures_this_election) {
			results[failure_status.first].push_back(
				failure_status.second);
		}
	}

	// We're interested in:
	// Burial alone
	// Compromise alone
	// Either of the two above
	// Neither of these but two-sided (Two-sided and Two-sided reverse)
	// Neither of the above but coalitional.

	size_t burial_only = 0,
		   compromise_only = 0,
		   burial_and_compromise = 0,
		   two_sided = 0,
		   coalitional = 0,
		   any_strategy = 0;

	for (f = 0; f < num_non_ties; ++f) {
		bool is_burial = results["Burial immunity"][f],
			 is_compromise = results["Compromising immunity"][f],
			 is_twosided = results["Two-sided immunity"][f]
				 || results["Two-sided reverse immunity"][f],
				 is_coalitional = results["Coalitional strategy immunity"][f];

		if (is_burial && !is_compromise) {
			++burial_only;
		}
		if (is_compromise && !is_burial) {
			++compromise_only;
		}
		if (is_compromise && is_burial) {
			++burial_and_compromise;
		}
		if (is_twosided && !is_compromise && !is_burial) {
			++two_sided;
		}
		if (is_coalitional && !is_twosided && !is_compromise && !is_burial) {
			++coalitional;
		}
		if (is_coalitional || is_twosided || is_compromise || is_burial) {
			++any_strategy;
		}
	}

	std::cout << "\nTies: " << 1 - (num_non_ties/(double)fmax) << " (" << fmax
		-num_non_ties << ") " << std::endl;
	std::cout << "Of the non-ties: " << std::endl;
	std::cout << "\nBurial, no compromise:\t" << burial_only << "\t"
		<< burial_only /(double) num_non_ties << std::endl;
	std::cout << "Compromise, no burial:\t" << compromise_only << "\t"
		<< compromise_only / (double) num_non_ties << std::endl;
	std::cout << "Burial and compromise:\t" << burial_and_compromise
		<< "\t" << burial_and_compromise / (double) num_non_ties
		<< std::endl;
	std::cout << "Two-sided:\t\t" << two_sided << "\t"
		<< two_sided / (double) num_non_ties << std::endl;
	std::cout << "Other coalition strats:\t" << coalitional << "\t"
		<< coalitional / (double) num_non_ties << std::endl;
	std::cout << "==========================================\n";
	std::cout << "Manipulable elections:\t" << any_strategy
		<< "\t" << any_strategy / (double) num_non_ties << std::endl;
	std::cout << "Quick and dirty: Other then manip " <<
		coalitional / (double) num_non_ties << " " <<
		any_strategy / (double) num_non_ties << "\n";

	total_generation_attempts = st.get_total_generation_attempts();

	// Do a simple proportions test
	double prop = any_strategy / (double) num_non_ties;

	double significance = 0.05;
	double zv = ppnd7(1-significance/num_methods);

	std::pair<double, double> c_i = confidence_interval(f, prop, zv);

	double lower_bound = c_i.first;
	double upper_bound = c_i.second;

	lower_bound = round(lower_bound*10000)/10000.0;
	upper_bound = round(upper_bound*10000)/10000.0;

	double tiefreq = 1 - (num_non_ties/(double)fmax);
	tiefreq = round(tiefreq*10000)/1000.0;

	std::cout << "Worked in " << any_strategy << " ("<< lower_bound << ", " <<
		upper_bound
		<< ") out of " << f << " for " <<
		to_test->name() << " ties: " << total_generation_attempts-f << " ("
		<< tiefreq << ")" << std::endl;



	std::cout << std::endl;
}

int main(int argc, const char ** argv) {
	std::vector<std::shared_ptr<election_method> > chosen_methods;
	std::vector<std::shared_ptr<election_method> > to_be_condorcified;

	size_t counter;

	std::vector<pairwise_ident> types;
	types.push_back(CM_WV);

	/*for (election_method * method: lots) {
		to_be_condorcified.push_back(
			new loser_elimination(
			method, false, false));
		to_be_condorcified.push_back(
			new loser_elimination(
			method, false, true));
		to_be_condorcified.push_back(
			new loser_elimination(
			method, true, false));
		to_be_condorcified.push_back(
			new loser_elimination(
			method, true, true));
	}*/

	condorcet_set xc;
	smith_set xsm;
	schwartz_set xs;
	landau_set xl;
	fpa_max_fpc fmf;
	donated_contingent_vote dcv;
	contingent_vote cv;
	ifpp_method_x si;
	std::shared_ptr<ifpp_method_x> strat_ifpp =
		std::make_shared<ifpp_method_x>();
	std::shared_ptr<smith_set> smith = std::make_shared<smith_set>();

	to_be_condorcified.push_back(
		std::make_shared<instant_runoff_voting>(PT_WHOLE, true));
	to_be_condorcified.push_back(std::make_shared<no_elimination_irv>());
	to_be_condorcified.push_back(std::make_shared<ifpp_method_x>());
	to_be_condorcified.push_back(std::make_shared<contingent_vote>());
	to_be_condorcified.push_back(std::make_shared<donated_contingent_vote>());
	to_be_condorcified.push_back(std::make_shared<contingent_vote>());
	to_be_condorcified.push_back(std::make_shared<ifpp_like_fpa_fpc>());
	to_be_condorcified.push_back(std::make_shared<fpa_sum_fpc>());
	to_be_condorcified.push_back(std::make_shared<fpa_max_fpc>());
	to_be_condorcified.push_back(std::make_shared<quick_runoff>());
	to_be_condorcified.push_back(std::make_shared<plurality>(PT_WHOLE));

	std::cout << "Test time!" << std::endl;

	for (counter = 0; counter < to_be_condorcified.size(); ++counter) {
		//chosen_methods.push_back(new comma(new slash(to_be_condorcified[counter], &xs), &xl));
		chosen_methods.push_back(std::make_shared<comma>(
				smith, to_be_condorcified[counter]));
		chosen_methods.push_back(std::make_shared<slash>(
				smith, to_be_condorcified[counter]));
	}

	//std::copy(lots.begin(), lots.end(), std::back_inserter(chosen_methods));

	//chosen_methods.push_back(new antiplurality(PT_WHOLE));

	std::cout << "There are " << chosen_methods.size() << " methods." <<
		std::endl;

	std::vector<rng> randomizers;
	randomizers.push_back(rng(1));

	std::vector<std::shared_ptr<pure_ballot_generator> > ballotgens;
	//int dimensions = 4;
	ballotgens.push_back(std::make_shared<impartial>(true, false));
	// Something is wrong with this one. Check later.
	/*ballotgens.push_back(new gaussian_generator(true, false, dimensions,
			false));*/
	//ballotgens.push_back(new dirichlet(false));

	int stepsize = atoi(argv[2]);
	int offset = atoi(argv[1]);

	for (size_t bg = 0; bg < ballotgens.size(); ++bg) {
		std::cout << "Using ballot domain " << ballotgens[bg]->name() << std::endl;
		std::cout << "Stepsize " << stepsize << ", offset " << offset << std::endl;
		for (counter = offset; counter < chosen_methods.size();
			counter += stepsize) {
			std::cout << "\t" << counter << ": " << std::flush;
			get_itemized_stats(chosen_methods[counter], randomizers[0],
				chosen_methods.size(), ballotgens[bg]);
		}
	}
}
