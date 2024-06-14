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

#include "../stats/confidence/as241.h"

#include "../generator/all.h"

#include "../singlewinner/gradual_c_b.h"

#include "../singlewinner/meta/filters/all.h"

#include "../singlewinner/stats/cardinal.h"
#include "../singlewinner/elimination/all.h"
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
#include "../singlewinner/experimental/rmr1.h"
#include "../singlewinner/brute_force/brute.h"
#include "../singlewinner/brute_force/bruterpn.h"

#include "../singlewinner/cardinal/all.h"

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

void get_itemized_stats(
	std::shared_ptr<election_method> to_test,
	std::shared_ptr<coordinate_gen> randomizer,
	int num_methods, std::shared_ptr<pure_ballot_generator> ballot_gen) {

	std::map<std::string, std::vector<bool> > results;

	std::cerr << "Itemized stats: Now trying " << to_test->name() << std::endl;

	int numvoters = 99;
	int initial_numcands = 4, numcands = initial_numcands;

	int tests_per_ballot = 32767;//1024;//32768;//512;
	test_provider tests;
	test_runner st(ballot_gen, numvoters, numcands, numcands,
		randomizer, to_test, tests_per_ballot);

	st.set_name("Strategy");
	for (auto test: tests.get_tests_by_category("Strategy")) {
		st.add_test(test);
	}

	size_t f, fmax = 500000, num_non_ties = 0;
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

	std::vector<std::shared_ptr<pure_ballot_generator> > ballotgens;

	std::shared_ptr<rng> randomizer = std::make_shared<rng>(RNG_ENTROPY);

	int dimensions = 4;

	auto gauss = std::make_shared<gaussian_generator>(
			true, false, dimensions, false);
	gauss->set_dispersion(1);
	ballotgens.push_back(gauss);

	double tenth_percentile = gauss->get_score_quantile(
			*randomizer, 0.10, 10000);
	std::cout << "10% rating quantile: " << tenth_percentile << std::endl;

	auto cond_ptr = std::make_shared<condorcet_set>();
	auto smith_ptr = std::make_shared<smith_set>();
	auto schwartz_ptr = std::make_shared<schwartz_set>();
	auto landau_ptr = std::make_shared<landau_set>();
	auto copeland_ptr = std::make_shared<copeland>(CM_WV);

	auto irv = std::make_shared<instant_runoff_voting>(PT_WHOLE, true);
	auto m_borda = std::make_shared<borda>(PT_WHOLE);
	auto m_minmax = std::make_shared<ord_minmax>(CM_WV, false);
	auto m_plur = std::make_shared<plurality>(PT_WHOLE);

	auto m_range = std::make_shared<cardinal_ratings>(0, 5, false);
	auto m_range_ten = std::make_shared<cardinal_ratings>(0, 10, false);
	auto normed_range = std::make_shared<cardinal_ratings>(0, 5, true);

	auto fixed_scale_range = std::make_shared<clamp>(m_range,
			tenth_percentile, 0, 5);
	auto fixed_scale_range_ten = std::make_shared<clamp>(m_range_ten,
			tenth_percentile, 0, 10);
	auto fixed_scale_approval = std::make_shared<clamp>(
			std::make_shared<cardinal_ratings>(0, 1, false),
			tenth_percentile, 0, 1);

	// Approval, MR				don't have this
	// Smith//Approval			don't have this
	// Double Defeat Hare		don't have this
	// Majority Judgement		don't have this
	// Max Strength Tr BP		don't have this
	// Approval			which one? don't have this
	// Margins-Sorted			don't have these

	// This isn't admissible, but if it were, would give an indication
	// of where MJ falls.
	//chosen_methods.push_back(std::make_shared<median_ratings>(0, 10, false, true));

	/*chosen_methods.push_back(std::make_shared<comma>(
		std::make_shared<inner_burial_set>(),
		std::make_shared<borda>(PT_WHOLE)));*/

	/*chosen_methods.push_back(std::make_shared<comma>(
		std::make_shared<condorcet_set>(),
		std::make_shared<plurality>(PT_WHOLE)));*/

	chosen_methods.push_back(std::make_shared<btr_irv>(PT_WHOLE, true));
	//chosen_methods.push_back(std::make_shared<fpa_sum_fpc>());

	// Smith//DAC (mean utility truncation)
	/*chosen_methods.push_back(std::make_shared<mean_utility_trunc>(
		std::make_shared<slash>(smith_ptr,
			std::make_shared<dac>())));*/

	// Range (objective scale)
	chosen_methods.push_back(fixed_scale_range);		// c'est ridicule!
	chosen_methods.push_back(fixed_scale_approval);

	// Smith//Range (objective scale)
	chosen_methods.push_back(std::make_shared<clamp>(
			std::make_shared<slash>(smith_ptr, m_range_ten), tenth_percentile, 0, 10));
	chosen_methods.push_back(std::make_shared<clamp>(
			std::make_shared<slash>(smith_ptr, m_range), tenth_percentile, 0, 5));
	// STAR (objective scale)
	chosen_methods.push_back(std::make_shared<clamp>(
			std::make_shared<auto_runoff>(m_range), tenth_percentile, 0, 5));

	// Approval (mean-utility strategy)
	auto approval = std::make_shared<mean_utility>(m_range, 1);
	auto implicit_smith = std::make_shared<mean_utility_trunc>(smith_ptr);

	// Smith//Approval (implicit)
	chosen_methods.push_back(std::make_shared<comma>(
			implicit_smith, approval));

	// Normalized Range (ten slot)
	chosen_methods.push_back(std::make_shared<normalize>(
			m_range_ten, 10));
	// Five slot
	chosen_methods.push_back(std::make_shared<normalize>(
			m_range, 5));

	/*
	// First Borda for reference (checking against JGA's published figures)
	// And BTR-IRV to show Robert Bristow-Johnson.
	chosen_methods.push_back(std::make_shared<loser_elimination>(m_plur,
			false, true, true));*/

	// Smith//Score (normalized)
	chosen_methods.push_back(std::make_shared<normalize>(
			std::make_shared<slash>(smith_ptr, m_range), 5));
	chosen_methods.push_back(std::make_shared<normalize>(
			std::make_shared<slash>(smith_ptr, m_range_ten), 10));

	// Approval (mean-utility strategy)
	chosen_methods.push_back(approval);

	// STAR, with Range for reference.
	//chosen_methods.push_back(m_range);
	//chosen_methods.push_back((std::make_shared<auto_runoff>(m_range)));
	chosen_methods.push_back(std::make_shared<normalize>(
			m_range, 5));
	chosen_methods.push_back(std::make_shared<normalize>(
			std::make_shared<auto_runoff>(m_range), 5));
	//chosen_methods.push_back(std::make_shared<auto_runoff>(m_range));
	//chosen_methods.push_back(normed_range);
	//chosen_methods.push_back(std::make_shared<auto_runoff>(normed_range));

	// Smith//Approval (explicit, normalized):
	// Smith,above-mean[Approval].
	chosen_methods.push_back(std::make_shared<comma>(smith_ptr,
			std::make_shared<mean_utility>(m_range, 1)));

	// Smith//Score (but beware, raw Range gives me much higher results than JGA
	// gets).
	chosen_methods.push_back(std::make_shared<slash>(smith_ptr, m_range));
	chosen_methods.push_back(std::make_shared<comma>(smith_ptr,
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

	//chosen_methods.resize(2);

	std::cout << "There are " << chosen_methods.size() << " methods." <<
		std::endl;

	/*std::cout << "90% rating quantile: " << gauss->get_score_quantile(
		randomizers[0], 0.90, 10000) << std::endl;
	return(0);*/

	//int dimensions = 4;
	//ballotgens.push_back(std::make_shared<impartial>(true, false));
	// Something is wrong with this one. Check later.
	/*ballotgens.push_back(new gaussian_generator(true, false, dimensions,
			false));*/
	//ballotgens.push_back(new dirichlet(false));

	int stepsize = atoi(argv[2]);
	int offset = atoi(argv[1]);

	size_t counter;

	for (size_t bg = 0; bg < ballotgens.size(); ++bg) {
		std::cout << "Using ballot domain " << ballotgens[bg]->name() << std::endl;
		std::cout << "Stepsize " << stepsize << ", offset " << offset << std::endl;
		for (counter = offset; counter < chosen_methods.size();
			counter += stepsize) {
			std::cout << "\t" << counter << ": " << std::flush;
			get_itemized_stats(chosen_methods[counter], randomizer,
				chosen_methods.size(), ballotgens[bg]);
		}
	}
}
