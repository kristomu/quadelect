#include <assert.h>
#include <errno.h>

#include <iterator>
#include <iostream>
#include <fstream>
#include <list>
#include <set>

#include "../singlewinner/stats/median.h"

#include "../tools/ballot_tools.h"
#include "../ballots.h"
#include "../tools/tools.h"

#include "../stats/confidence/as241.h"
#include "../singlewinner/get_methods.h"

#include "../generator/all.h"

#include "../singlewinner/gradual_c_b.h"

#include "../singlewinner/stats/cardinal.h"
#include "../singlewinner/elimination/all.h"
#include "../singlewinner/brute_force/all.h"
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

#include "../singlewinner/experimental/all.h"
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

#include "../tests/strategy/strategies.h"
#include "../tests/provider.h"

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

void test_strategy(std::shared_ptr<const election_method> to_test,
	std::shared_ptr<rng> randomizer, int num_methods,
	std::shared_ptr<pure_ballot_generator> ballot_gen,
	int numvoters, int numcands, int num_iterations,
	int num_strategy_attempts_per_iter) {

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
	impartial ic(true, false);
	//impartial ballot_gen(true, false);

	election_t ballots;

	// A bunch of times, generate ballots and clear the cache. Then try
	// these ballots against numerous Condorcet methods. If we have
	// cached the Condorcet data, that should be faster than if we haven't,
	// but one probably needs Valgrind to see the difference.

	int counter;

	std::cerr << "Now trying " << to_test->name() << std::endl;

	cache_map cache;

	std::map<int, std::string> rcl;
	for (counter = 0; counter < 26; ++counter) {
		std::string foo = "A";
		foo[0] = 'A' + counter;
		rcl[counter] = foo;
	}

	// --- //

	test_provider tests;
	test_runner st(ballot_gen, numvoters, numcands,
		randomizer, to_test, num_strategy_attempts_per_iter);

	// We're testing the rate of failure of "strategy immunity" criteria,
	// i.e. the presence of opportunities for manipulation.
	st.set_name("Strategy");

	for (auto test: tests.get_tests_by_category("Strategy")) {
		st.add_test(test);
	}


	int strategy_worked = 0, strategy_failed = 0;
	int i;
	int total_test_attempts = 0;

	for (i = 0; i < num_iterations; ++i) {
		// Perform_test may do more than one test if there's a tie, so that
		// such ties do not make the method look artificially susceptible
		// or resistant to strategy.

		// The total number of tests actually performed is retrieved later.
		if (st.simulate(true) == 0) {
			++strategy_worked;
		} else {
			++strategy_failed;
		}

		if ((i & 63) == 63) {
			std::cerr << "." << std::flush;
			if ((i & 4095) == 4095) {
				std::cerr << i/(double)num_iterations << std::flush;
			}
		}
	}

	total_test_attempts = st.get_total_generation_attempts();
	int ties = total_test_attempts - num_iterations;

	// Do a simple proportions test
	double prop = strategy_worked/(double)num_iterations;

	double significance = 0.05;
	double zv = ppnd7(1-significance/num_methods);

	std::pair<double, double> c_i = confidence_interval(num_iterations, prop,
			zv);

	double lower_bound = c_i.first;
	double upper_bound = c_i.second;

	lower_bound = round(lower_bound*10000)/10000.0;
	upper_bound = round(upper_bound*10000)/10000.0;

	double tiefreq = ties/(double)total_test_attempts;
	tiefreq = round(tiefreq*1000)/1000.0;

	//#pragma omp critical
	{
		std::cout << "Worked in " << strategy_worked << " ("<< lower_bound
			<< ", " << upper_bound << ") out of " << num_iterations
			<< " for " << to_test->name() << " ties: "
			<< ties << " (" << tiefreq << ")" << std::endl;
		std::cout << "strat;" << numvoters << ";" << numcands << ";"
			<< ballot_gen->name() << ";" << to_test->name() << ";"
			<< lower_bound << ";" << prop << ";" << upper_bound
			<< ";" << tiefreq << std::endl;
	}
}

int main(int argc, const char ** argv) {
	std::vector<std::shared_ptr<election_method> > chosen_methods;
	std::vector<std::shared_ptr<election_method> > condorcet_src;

	size_t counter;

	std::vector<pairwise_ident> types;
	types.push_back(CM_WV);

	// The great cosmic vote-off
	// We want to test these methods with four candidates, 29 voters,
	// impartial culture. Strictly speaking, though, I would really
	// want to have a spatial model.

	int numvoters, numcands;

	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " [num voters]"
			<< " [num candidates]" << std::endl;
		std::cerr << "Assuming 29 voters, 4 candidates.\n";
		numvoters = 29;
		numcands = 4;
	} else {
		numvoters = atoi(argv[1]);
		numcands = atoi(argv[2]);
	}

	// Smith//Score				don't have this
	// Approval, MR				don't have this
	// Smith//Approval			don't have this
	// Double Defeat Hare		don't have this
	// Majority Judgement		don't have this
	// Max Strength Tr BP		don't have this
	// STAR						don't have this
	// Approval					which one? don't have this
	// Margins-Sorted			don't have these
	// RCIPE					don't have this

	// Woodall					Smith//IRV
	// Schwartz-Woodall			Schwartz//IRV
	// Benham					Benham

	// Ranked Robin				Copeland//Borda
	// Minmax(wv)				Minmax(wv)

	// Plurality				Plurality
	// IRV						IRV

	// Schulze					Schulze
	// Baldwin					Eliminate-[Borda]
	// Black					Condorcet//Borda

	// Raynaud(Gross Loser)		Eliminate-[Minmax(whatever)] since no truncation
	//							I should implement Gross Loser, though
	//							later.

	// Smith//DAC				Smith//DAC

	// RP(wv)					RP(wv)

	//string name = argv[1];
	//ifstream tests_in(argv[1]);
	std::vector<unsigned long long> tests;

	auto cond_ptr = std::make_shared<condorcet_set>();
	auto smith_ptr = std::make_shared<smith_set>();
	auto schwartz_ptr = std::make_shared<schwartz_set>();
	auto landau_ptr = std::make_shared<landau_set>();
	auto copeland_ptr = std::make_shared<copeland>(CM_WV);

	auto irv = std::make_shared<instant_runoff_voting>(PT_WHOLE, true);
	auto m_borda = std::make_shared<borda>(PT_WHOLE);
	auto m_minmax = std::make_shared<ord_minmax>(CM_WV, false);
	auto m_plur = std::make_shared<plurality>(PT_WHOLE);

	auto m_range = std::make_shared<cardinal_ratings>(0, 10, false);
	auto normed_range = std::make_shared<cardinal_ratings>(0, 10, true);

	// Approval, MR				don't have this
	// Smith//Approval			don't have this
	// Double Defeat Hare		don't have this
	// Majority Judgement		don't have this
	// Max Strength Tr BP		don't have this
	// Approval					which one? don't have this
	// Margins-Sorted			don't have these

	// This isn't admissible, but if it were, would give an indication
	// of where MJ falls.
	//chosen_methods.push_back(std::make_shared<median_ratings>(0, 10, false, true));

	// First Borda for reference (checking against JGA's published figures)
	// And BTR-IRV to show Robert Bristow-Johnson.
	/*chosen_methods.push_back(std::make_shared<loser_elimination>(m_plur,
			false, true, true));*/

	// STAR, with Range for reference.
	chosen_methods.push_back(m_range);
	chosen_methods.push_back(std::make_shared<auto_runoff>(m_range));
	chosen_methods.push_back(normed_range);
	chosen_methods.push_back(std::make_shared<auto_runoff>(normed_range));

	// Smith//Score (but beware, raw Range gives me much higher results than JGA
	// gets).
	chosen_methods.push_back(std::make_shared<slash>(smith_ptr, m_range));
	chosen_methods.push_back(std::make_shared<slash>(smith_ptr,
			std::make_shared<cardinal_ratings>(0, 10, true)));
	chosen_methods.push_back(m_borda);

	// RCIPE					Eliminate-[[Cond. nonloser], Plurality]
	chosen_methods.push_back(std::make_shared<loser_elimination>(
			std::make_shared<comma>(
				std::make_shared<condorcet_nonloser_set>(),
				m_plur),
			false, true));

	// Woodall					Smith//IRV
	// Schwartz-Woodall			Schwartz//IRV
	// Benham					Benham

	chosen_methods.push_back(std::make_shared<slash>(smith_ptr, irv));
	chosen_methods.push_back(std::make_shared<slash>(schwartz_ptr, irv));
	chosen_methods.push_back(std::make_shared<benham_meta>(irv));

	// Ranked Robin				Copeland//Borda
	// Minmax(wv)				Minmax(wv)

	chosen_methods.push_back(std::make_shared<slash>(copeland_ptr, m_borda));
	chosen_methods.push_back(m_minmax);

	// Plurality				Plurality
	// IRV						IRV

	chosen_methods.push_back(m_plur);
	chosen_methods.push_back(irv);

	// Schulze					Schulze
	// Baldwin					Eliminate-[Borda]
	// Black					Condorcet//Borda

	chosen_methods.push_back(std::make_shared<schulze>(CM_WV));
	chosen_methods.push_back(std::make_shared<baldwin>(PT_WHOLE, true));
	chosen_methods.push_back(std::make_shared<slash>(cond_ptr, m_borda));

	// Raynaud(Gross Loser)		Eliminate-[Minmax(whatever)] since no truncation
	//							I should implement Gross Loser, though
	//							later.
	// Smith//DAC				Smith//DAC
	// RP(wv)					RP(wv)

	chosen_methods.push_back(std::make_shared<loser_elimination>(m_minmax,
			false, true));
	chosen_methods.push_back(std::make_shared<slash>(smith_ptr,
			std::make_shared<dsc>()));
	chosen_methods.push_back(std::make_shared<ranked_pairs>(CM_WV, false));

	std::cout << "There are " << chosen_methods.size() << " methods." <<
		std::endl;

	std::list<election_method * > methods_ref;
	for (const std::shared_ptr<election_method> & em_ref: chosen_methods) {
		methods_ref.push_back(em_ref.get());
	}

	std::shared_ptr<rng> randomizer = std::make_shared<rng>(0);

	std::vector<std::shared_ptr<pure_ballot_generator> > ballotgens;
	int dimensions = 4;
	// Something is wrong with this one. Check later.
	//ballotgens.push_back(std::make_shared<dirichlet>(false));
	//ballotgens.push_back(std::make_shared<impartial>(true, false));
	auto gauss = std::make_shared<gaussian_generator>(
			true, false, dimensions, false);
	gauss->set_dispersion(1);

	ballotgens.push_back(gauss);

	int num_iterations = 25000; //100000;
	int num_strategy_attempts_per_iter = 3*768; // must have 3 as a factor!

	for (size_t bg = 0; bg < ballotgens.size(); ++bg) {
		std::cout << "Using ballot domain " << ballotgens[bg]->name() << std::endl;
		for (counter = 0; counter < methods_ref.size(); counter ++) {
			std::cout << "\t" << counter << ": " << std::flush;
			test_strategy(chosen_methods[counter], randomizer,
				chosen_methods.size(), ballotgens[bg], numvoters, numcands,
				num_iterations, num_strategy_attempts_per_iter);
		}
	}
}
