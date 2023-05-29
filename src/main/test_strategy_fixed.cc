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

#include "../tests/tests/monotonicity/monotonicity.h"

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
#include "../bandit/lucb.h"

#include "strat_test.h"

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
pair<double, double> confidence_interval(int n, double p_mean, double z) {

	double p_mark = (p_mean * n + 0.5 * z * z) / (n + z * z);

	double W = sqrt(p_mark * (1 - p_mark) / (n + z * z));

	return (pair<double, double>(p_mark - W, p_mark + W));
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

// Lawrdy o mighty is this ugly.

bool test_once(list<ballot_group> & ballots,
	pure_ballot_generator & ballot_gen, int numvoters, int numcands,
	rng & randomizer, int & total_generation_attempts,
	vector<election_method *> & condorcets) {

	int ranks = 10;
	int ties_in_a_row = 0;
	ordering::const_iterator pos;
	ordering honest;

	// Ties don't count because any method that's decisive will let a voter
	// break the tie.
	// Err, a method may not always be decisive! TODO: Fix that. A tie
	// should count as a method with many equal-winners and be considered
	// susceptible to strategy if someone else can be made a winner, or one
	// of the equal-winners turned into a sole winner.

	while (ranks > 1) {
		ranks = 0;
		++ties_in_a_row;
		if (ties_in_a_row > 100) {
			cout << "Too many ties in a row. Aborting!" << endl;
			// Should be a third option here, not true or false.
			// TODO that.
			return (false);
		}

		ballots = ballot_gen.generate_ballots(numvoters, numcands, randomizer);

		//cache.clear();

		// First, get the honest winner. If it's a tie, disregard. Otherwise,
		// for every other candidate, find the ballots corresponding to voters
		// who rank that candidate ahead of the winner. Then, a bunch of times,
		// run the IIC generator with numvoters = 1 and use that ballot for the
		// strategist coalition. (An exhaustive approach may come later.) If
		// this makes the candidate in question win, we're done, otherwise
		// try again until we give up and go to the next candidate, or give up
		// completely and consider the method invulnerable in this particular
		// case.

		honest = condorcets[0]->elect(ballots, numcands, NULL, true);

		// Check that there isn't a tie.
		//int ranks = 0;
		//ordering::const_iterator pos;
		for (pos = honest.begin(); pos != honest.end() && pos->get_score() ==
			honest.begin()->get_score(); ++pos) {
			++ranks;
		}

		// This is actually how many are at first rank. Rename the vars to
		// make that more clear.
		total_generation_attempts += ranks;

		/*cout << ordering_tools().ordering_to_text(honest, rcl,
				true) << endl;

		cout << "Ranks: " << ranks << endl;*/
		// TODO: Give up after n tries.
	}

	ties_in_a_row = 0;

	int winner = honest.begin()->get_candidate_num();

	//cout << "The winner is " << winner << endl;

	bool strategy_worked = false;

	for (int counter = 0; counter < numcands && !strategy_worked;
		++counter) {
		if (counter == winner) {
			continue;
		}

		//cout << "Trying to rig in favor of " << counter << endl;

		list<ballot_group> prefers_winner;
		double num_prefers_challenger = 0;

		// Find those who prefer the challenger. Add those that
		// don't into prefers_winner and add up the score of those that
		// do.

		for (list<ballot_group>::const_iterator bgpos = ballots.begin();
			bgpos != ballots.end(); ++bgpos) {
			int saw_winner = -1, saw_challenger = -1;
			int rank = 0;

			for (pos = bgpos->contents.begin(); pos !=
				bgpos->contents.end() &&
				(saw_winner == -1 ||
					saw_challenger == -1); ++pos) {
				if (pos->get_candidate_num() == winner) {
					saw_winner = rank;
				}
				if (pos->get_candidate_num() == counter) {
					saw_challenger = rank;
				}

				++rank;
			}

			// We consider equal-ranks to be in favor of the winner,
			// thus we only explicitly check if the challenger was
			// rated above the winner.
			if (saw_challenger < saw_winner) {
				num_prefers_challenger += bgpos->weight;
			} else {
				prefers_winner.push_back(*bgpos);
			}
		}

		/*cout << num_prefers_challenger << " prefers " << counter
			<< endl;*/

		if (num_prefers_challenger == 0) {
			continue;
		}

		for (int tries = 0; tries < 512 && !strategy_worked; ++tries) {
			// Add the strategic ballot by drawing from IIC.
			int iterations = 1 + tries % 3, q;
			double cumul = 0;
			for (q = 0; q < iterations; ++q) {
				list<ballot_group> strategy;
				while (strategy.empty())
					strategy = ballot_gen.generate_ballots(1, numcands,
							randomizer);
				if (q == iterations-1) {
					assert(num_prefers_challenger - cumul > 0);
					strategy.begin()->weight = num_prefers_challenger - cumul;
				} else {
					strategy.begin()->weight = randomizer.drand() * num_prefers_challenger/
						(double)iterations;
					cumul += strategy.begin()->weight;
				}

				// Add the strategic ballot.
				prefers_winner.push_back(*strategy.begin());
			}

			// Determine the winner again! A tie counts if our man
			// is at top rank, because he wasn't, before.
			ordering strat_result = condorcets[0]->elect(prefers_winner,
					numcands, NULL, true);

			/*cout << ordering_tools().ordering_to_text(
			  strat_result,
					rcl, true) << endl;*/

			// Then remove the strategic coalition ballot so another
			// one can be inserted later.
			for (q = 0; q < iterations; ++q) {
				prefers_winner.pop_back();
			}

			// Check if our candidate is now at top rank.
			for (pos = strat_result.begin(); pos !=
				strat_result.end() &&
				pos->get_score() == strat_result.
				begin()->get_score(); ++pos)
				if (pos->get_candidate_num() == counter) {
					strategy_worked = true;
				}
		}
		/*if (strategy_worked)
			cout << "Strategy to elect " << counter << " worked!" << endl;*/
	}

	if (strategy_worked) {
		//	cout << "Strategy worked!" << endl;
		return (true);
	}
	//else	cout << "Didn't work." << endl;
	//cout << endl << endl;
	return (false);
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

	list<ballot_group> ballots;

	// A bunch of times, generate ballots and clear the cache. Then try
	// these ballots against numerous Condorcet methods. If we have
	// cached the Condorcet data, that should be faster than if we haven't,
	// but one probably needs Valgrind to see the difference.

	int counter;

	vector<election_method *> condorcets;

	condorcets.push_back(to_test);
	cerr << "Now trying " << to_test->name() << endl;

	cache_map cache;

	// TODO: check if that actually works.
	int numvoters = 37; // was 29
	int initial_numcands = 3/*4*/, numcands = initial_numcands;

	map<int, string> rcl;
	for (counter = 0; counter < 26; ++counter) {
		string foo = "A";
		foo[0] = 'A' + counter;
		rcl[counter] = foo;
	}

	// --- //

	vector<pure_ballot_generator *> ballotgens;
	ballotgens.push_back(ballot_gen);
	//ballotgens.push_back(&iic);
	///ballotgens.push_back(new gaussian_generator(true, false, dimensions, false));
	/* ballotgens.push_back(new dirichlet(true));*/

	StrategyTest st(ballotgens, &iic, numvoters, numcands, randomizer,
		condorcets, 0);

	int worked = 0, f;
	int fmax = 100000; //50000;
	int total_generation_attempts = 0;
	for (f = 0; f < fmax; ++f) {

		if ((f & 63) == 63) {
			cerr << "." << flush;
		}
		if ((f & 4095) == 4095) {
			cerr << f/(double)fmax << flush;
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

	pair<double, double> c_i = confidence_interval(f, prop, zv);

	double lower_bound = c_i.first;
	double upper_bound = c_i.second;

	lower_bound = round(lower_bound*10000)/10000.0;
	upper_bound = round(upper_bound*10000)/10000.0;

	double tiefreq = (total_generation_attempts-f)/(double)f;
	tiefreq = round(tiefreq*10000)/1000.0;

	cout << "Worked in " << worked << " ("<< lower_bound << ", " << upper_bound
		<< ") out of " << f << " for " <<
		condorcets[0]->name() << " ties: " << total_generation_attempts-f << " ("
		<< tiefreq << ")" << endl;
}

int main(int argc, const char ** argv) {
	vector<election_method *> condorcets; // Although they aren't.
	vector<election_method *> condorcetsrc;

	size_t counter;

	vector<pairwise_ident> types;
	types.push_back(CM_WV);

	condorcetsrc.push_back(new plurality(PT_WHOLE));
	condorcetsrc.push_back(new antiplurality(PT_WHOLE));

	cout << "Test time!" << endl;

	smith_set xd;

	// TODO: Really fix comma. DONE, kinda.
	for (counter = 0; counter < condorcetsrc.size(); ++counter) {
		condorcets.push_back(new slash(condorcetsrc[counter], &xd));
		condorcets.push_back(new comma(condorcetsrc[counter], &xd));
	}

	//condorcets.push_back(new ext_minmax(CM_WV, false));
	//condorcets.push_back(new schulze(CM_WV));
	//condorcets.push_back(new ranked_pairs(CM_WV, false));
	//condorcets.push_back(new ranked_pairs(CM_WV, true));

	cout << "There are " << condorcets.size() << " methods." << endl;

	vector<rng> randomizers;
	randomizers.push_back(rng(1));

	vector<pure_ballot_generator *> ballotgens;
	int dimensions = 4;
	ballotgens.push_back(new impartial(true, false));
	// Something is wrong with this one. Check later.
	ballotgens.push_back(new gaussian_generator(true, false, dimensions,
			false));
	//ballotgens.push_back(new dirichlet(false));

	for (size_t bg = 0; bg < ballotgens.size(); ++bg) {
		cout << "Using ballot domain " << ballotgens[bg]->name() << endl;
		for (counter = 0; counter < condorcets.size(); counter ++) {
			cout << "\t" << counter << ": " << flush;
			test_strategy(condorcets[counter], randomizers[0],
				condorcets.size(), ballotgens[bg]);
		}
	}
}
