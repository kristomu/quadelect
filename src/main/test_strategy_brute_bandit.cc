#include <values.h>
#include <assert.h>
#include <errno.h>

#include <iterator>
#include <iostream>
#include <fstream>
#include <list>
#include <set>

#include "../ballot_tools.h"
#include "../ballots.h"
#include "../tools.h"

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
            return(false);
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
                honest.begin()->get_score(); ++pos)
            ++ranks;

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
        if (counter == winner) continue;

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
                if (pos->get_candidate_num() == winner)
                    saw_winner = rank;
                if (pos->get_candidate_num() == counter)
                    saw_challenger = rank;

                ++rank;
            }

            // We consider equal-ranks to be in favor of the winner,
            // thus we only explicitly check if the challenger was
            // rated above the winner.
            if (saw_challenger < saw_winner)
                num_prefers_challenger += bgpos->weight;
            else
                prefers_winner.push_back(*bgpos);
        }

        /*cout << num_prefers_challenger << " prefers " << counter
        	<< endl;*/

        if (num_prefers_challenger == 0) continue;

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
                    assert (num_prefers_challenger - cumul > 0);
                    strategy.begin()->weight = num_prefers_challenger - cumul;
                } else {
                    strategy.begin()->weight = randomizer.drand() * num_prefers_challenger/(double)iterations;
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
            for (q = 0; q < iterations; ++q)
                prefers_winner.pop_back();

            // Check if our candidate is now at top rank.
            for (pos = strat_result.begin(); pos !=
                    strat_result.end() &&
                    pos->get_score() == strat_result.
                    begin()->get_score(); ++pos)
                if (pos->get_candidate_num() == counter)
                    strategy_worked = true;
        }
        /*if (strategy_worked)
        	cout << "Strategy to elect " << counter << " worked!" << endl;*/
    }

    if (strategy_worked) {
        //	cout << "Strategy worked!" << endl;
        return(true);
    }
    //else	cout << "Didn't work." << endl;
    //cout << endl << endl;
    return(false);
}


void test_with_bandits(vector<election_method *> & to_test, 
	rng & randomizer, vector<pure_ballot_generator *> & ballotgens,
	pure_ballot_generator * strat_generator) {

	impartial iic(true, false);

	vector<Bandit> bandits;
	vector<StrategyTest> sts;

	vector<election_method *> condorcets;
	condorcets.push_back(to_test[0]);

	/*StrategyTest st(iic,  numvoters, numcands, randomizer, condorcets,
		0);*/

	int numvoters = 5000; // was 37
    int initial_numcands = 3/*4*/, numcands = initial_numcands;

    size_t i;

	for (i = 0; i < to_test.size(); ++i) {
		condorcets[0] = to_test[i];
		sts.push_back(StrategyTest(ballotgens, strat_generator,
			numvoters, numcands, randomizer, condorcets, 0));
	}

	for (i = 0; i < to_test.size(); ++i) {
		bandits.push_back(Bandit(&sts[i]));
	}

	LUCB lucb;

	bool confident = false;

	double num_methods = sts.size();
    double report_significance = 0.05;
    double zv = ppnd7(1-report_significance/num_methods);

    time_t startpt = time(NULL);

	for (int j = 1; j < 1000000 && !confident; ++j) {
		// Bleh, why is min a macro?
		int num_tries = max(100, (int)sts.size());
		// Don't run more than 20k at a time because otherwise
		// feedback is too slow.
		num_tries = min(40000, num_tries);
		double progress = lucb.pull_bandit_arms(bandits, num_tries, true);
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
			vector<pair<double, int> > so_far;
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
				so_far.push_back(pair<double, int>(bandits[k].get_mean(), k));
			}
			sort(so_far.begin(), so_far.end());
			reverse(so_far.begin(), so_far.end());

			size_t how_many = 10;
			cout << "Interim report (by mean):" << endl;
			for (k = 0; k < how_many; ++k) {
				double inv_mean = 1-bandits[so_far[k].second].get_mean();

				pair<double, double> c_i = confidence_interval(
					bandits[so_far[k].second].get_num_pulls(), 
					inv_mean, zv);
				double lower = round(c_i.first * 1000)/1000.0;
				double middle = round(inv_mean * 1000)/1000.0;
				double upper = round(c_i.second * 1000)/1000.0;

				cout << k+1 << ". " << bandits[so_far[k].second].name() << "(" <<
					lower << ", " << middle << ", " << upper << ")" << endl;
			}

		}
	}
	
	std::pair<int, double> results = lucb.get_best_bandit_so_far(bandits);

	cout << "Best so far is " << results.first << " with CB of " << results.second << std::endl;
	cout << "Its name is " << bandits[results.first].name() << endl;
	cout << "It has a mean of " << bandits[results.first].get_mean() << endl;

}

int main(int argc, const char ** argv) {
    vector<election_method *> condorcets; // Although they aren't.
    vector<election_method *> condorcetsrc;

    int counter;

    //int got_this_far_last_time = 0;

/*    int radix=5;

    for (int j = 0; j < pow(radix, 6); ++j) {
    	cond_brute cbp(j, radix);

        if (cbp.is_monotone() && cbp.passes_mat() /v*&& cbp.reversal_symmetric()*v/) {
            condorcetsrc.push_back(new cond_brute(j, radix));
            cout << "Adding " << j << endl;
	   }
    }
*/

    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " [brute_rpn_number_list.txt]"
            << endl;
        return(-1);
    }

    string name = argv[1];
    ifstream tests_in(argv[1]);
    vector<unsigned long long> tests;

    while (!tests_in.eof()) {
        unsigned long long tin;
        tests_in >> tin;
        condorcetsrc.push_back(new cond_brute_rpn(tin));
    }

    condorcet_set xd;

    for (counter = condorcetsrc.size()-1; counter >= 0; --counter) {
        condorcets.push_back(new comma(condorcetsrc[counter], &xd));
    }

    reverse(condorcets.begin(), condorcets.end());

    cout << "There are " << condorcets.size() << " methods." << endl;

    // Generate separate RNGs for each thread.
    const int numthreads = 16;
    vector<rng> randomizers;
    for (counter = 0; counter < numthreads; ++counter) {
        randomizers.push_back(rng(counter));
    }

    vector<pure_ballot_generator *> ballotgens;
    int dimensions = 4;
    ballotgens.push_back(new impartial(true, false));
    // Something is wrong with this one. Check later.
    //ballotgens.push_back(new dirichlet(false));
    ballotgens.push_back(new gaussian_generator(true, false, dimensions, false));

    test_with_bandits(condorcets, randomizers[0], ballotgens, ballotgens[0]);

    return(0);
}
