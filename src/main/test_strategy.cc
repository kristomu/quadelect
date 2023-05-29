#include <values.h>
#include <errno.h>

#include <stdexcept>
#include <iterator>
#include <iostream>
#include <fstream>
#include <list>
#include <set>

#include "../tools/ballot_tools.h"
#include "../ballots.h"
#include "../tools/tools.h"

#include "../generator/spatial/gaussian.h"
#include "../generator/spatial/uniform.h"
#include "../generator/impartial.h"
#include "../generator/dirichlet.h"

#include "../singlewinner/gradual_c_b.h"

#include "../singlewinner/stats/cardinal.h"
#include "../singlewinner/elimination.h"
#include "../singlewinner/positional/simple_methods.h"

#include "../tests/tests/monotonicity/mono_raise.h"

#include "../stats/stats.h"

#include "../singlewinner/pairwise/simple_methods.h"
#include "../singlewinner/pairwise/keener.h"
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

#include "../singlewinner/sets/mdd.h"
#include "../singlewinner/sets/condorcet.h"
#include "../singlewinner/sets/max_elements/all.h"
#include "../singlewinner/sets/partition.h"
#include "../singlewinner/meta/comma.h"
#include "../singlewinner/meta/slash.h"

#include "../singlewinner/stats/var_median/vmedian.h"
#include "../singlewinner/stats/median.h"
#include "../singlewinner/stats/mode.h"

#include "../singlewinner/young.h"

#include "../random/random.h"

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

void test_strategy(election_method * to_test, rng & randomizer) {

	// Generate a random ballot set.
	// First should be true: compress the ballots. Second, that's
	// whether truncation is permitted.
	// TODO later: fix last-rank problem with gradual-cond-borda.
	// Also fix memory-hogging loop of doom in positional if this is set on.
	//impartial ic(/*true, true*/true, true);
	//spatial_generator ic(true, false, 4, false);
	gaussian_generator ic(true, false, 4, false);
	//uniform_generator spatial(true, false);
	impartial true_ic(true, false);

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
	int numvoters = 29;
	int initial_numcands = 4, numcands = initial_numcands;

	map<int, string> rcl;
	for (counter = 0; counter < 26; ++counter) {
		string foo = "A";
		foo[0] = 'A' + counter;
		rcl[counter] = foo;
	}

	// --- //
	int worked = 0, f;
	for (f = 0; f < 1000; ++f) {
		if ((f & 31) == 31) {
			cerr << "." << flush;
		}
		srandom(f);
		srand(f);
		srand48(f);

		int ranks = 10;
		ordering::const_iterator pos;
		ordering honest;

		// Ties don't count because any method that's decisive will let a voter
		// break the tie.

		while (ranks > 1) {
			ranks = 0;

			ballots = ic.generate_ballots(numvoters, numcands, randomizer);

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

			/*cout << ordering_tools().ordering_to_text(honest, rcl,
					true) << endl;

			cout << "Ranks: " << ranks << endl;*/
			// TODO: Give up after n tries.
		}

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
				// Add the strategic ballot by drawing from IC.
				int iterations = 1 + tries % 3, q;
				double cumul = 0;
				for (q = 0; q < iterations; ++q) {
					list<ballot_group> strategy;
					while (strategy.empty()) {
						strategy = ic.generate_ballots(1, numcands, randomizer);
					}
					if (q == iterations-1) {
						if (num_prefers_challenger - cumul < 0) {
							throw std::runtime_error("Cumulative sum of "
								"voters preferring challenger exceeds "
								"maximum! Something's seriously wrong.");
						}
						strategy.begin()->weight = num_prefers_challenger - cumul;
					} else {
						strategy.begin()->weight = drand48() * num_prefers_challenger/
							(double)iterations;
						cumul += strategy.begin()->weight;
					}

					// Add the strategic ballot.
					prefers_winner.push_back(*strategy.begin());
				}

				// Determine the winner again! A tie counts if our man
				// is at top rank, because he wasn't, before.
				ordering strat_result = condorcets[0]->elect(
						prefers_winner, numcands, NULL, true);

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
			++worked;
		}
		//else	cout << "Didn't work." << endl;
		//cout << endl << endl;
	}

	cout << "Worked in " << worked << " out of " << f << " for " <<
		condorcets[0]->name() << endl;
}

int main() {
	vector<election_method *> condorcets; // Although they aren't.

	double power_min = 0.25;
	double power_max = 2.5;
	double powerstep = 0.25;

	size_t counter;

	rng randomizer(1);

	vector<pairwise_ident> types;
	//types.push_back(CM_KEENER_MARGINS);
	types.push_back(CM_WV);
	types.push_back(CM_PAIRWISE_OPP);
	types.push_back(CM_TOURN_WV);
	/*types.push_back(CM_TOURN_SYM); // may have negative values*/
	types.push_back(CM_FRACTIONAL_WV);
	types.push_back(CM_RELATIVE_MARGINS);
	types.push_back(CM_KEENER_MARGINS);

	for (counter = 0; counter < types.size(); ++counter) {
		condorcets.push_back(new kemeny(types[counter]));
		condorcets.push_back(new ext_minmax(types[counter], false));
		condorcets.push_back(new ext_minmax(types[counter], true));
		condorcets.push_back(new ord_minmax(types[counter]));
		//condorcets.push_back(new maxmin(types[counter]));
		for (double power = power_min; power < power_max; power +=
				powerstep) {
			condorcets.push_back(new least_rev(types[counter],
					true, false, power));
			condorcets.push_back(new least_rev(types[counter],
					false, true, power));
			condorcets.push_back(new least_rev(types[counter],
					true, true, power));
		}
		condorcets.push_back(new copeland(types[counter]));
		condorcets.push_back(new copeland(types[counter], 2, 2, 1));
		condorcets.push_back(new copeland(types[counter], 2, 1, 0));
		condorcets.push_back(new schulze(types[counter]));
		condorcets.push_back(new ranked_pairs(types[counter], false));
		condorcets.push_back(new ranked_pairs(types[counter], true));
		//condorcets.push_back(new dquick(types[counter]));
		/*condorcets.push_back(new keener(types[counter], 0.001, false,
		                        false));
		condorcets.push_back(new keener(types[counter], 0.001, false,
		                        true));*/
		condorcets.push_back(new keener(types[counter], 0.001, true,
				false));
		condorcets.push_back(new keener(types[counter], 0.001, true,
				true));
		condorcets.push_back(new sinkhorn(types[counter], 0.001, true));
		//condorcets.push_back(new sinkhorn(types[counter], 0.001, false));
		condorcets.push_back(new odm(types[counter], 0.001));
		condorcets.push_back(new hits(types[counter], 0.001));
		condorcets.push_back(new odm_atan(types[counter], 0.001));
	}

	vector<completion_type> ct;
	ct.push_back(GF_NONE);
	// Doesn't matter, only slows us down.
	/*ct.push_back(GF_LEAST);
	ct.push_back(GF_GREATEST);
	ct.push_back(GF_BOTH);*/

	for (counter = 0; counter < ct.size(); ++counter) {
		condorcets.push_back(new gradual_cond_borda(new smith_set,
				true, ct[counter]));
		condorcets.push_back(new gradual_cond_borda(new smith_set,
				false, ct[counter]));
		condorcets.push_back(new gradual_cond_borda(new schwartz_set,
				true, ct[counter]));
		condorcets.push_back(new gradual_cond_borda(new schwartz_set,
				false, ct[counter]));
		condorcets.push_back(new gradual_cond_borda(new sdom_set,
				true, ct[counter]));
		condorcets.push_back(new gradual_cond_borda(new sdom_set,
				false, ct[counter]));
		/*condorcets.push_back(new gradual_cond_borda(new cdtt_set,
		                        true, ct[counter]));
		condorcets.push_back(new gradual_cond_borda(new cdtt_set,
		                        false, ct[counter]));
		condorcets.push_back(new gradual_cond_borda(new cgtt_set,
		                        true, ct[counter]));
		condorcets.push_back(new gradual_cond_borda(new cgtt_set,
		                        false, ct[counter]));
		condorcets.push_back(new gradual_cond_borda(new mdd_set(false),
		                        true, ct[counter]));
		condorcets.push_back(new gradual_cond_borda(new mdd_set(false),
		                        false, ct[counter]));*/
		condorcets.push_back(new gradual_cond_borda(new landau_set,
				true, ct[counter]));
		condorcets.push_back(new gradual_cond_borda(new landau_set,
				false, ct[counter]));
		condorcets.push_back(new gradual_cond_borda(new condorcet_set,
				true, ct[counter]));
		condorcets.push_back(new gradual_cond_borda(new condorcet_set,
				false, ct[counter]));
		condorcets.push_back(new gradual_cond_borda(new copeland(CM_WV),
				true, ct[counter]));
		condorcets.push_back(new gradual_cond_borda(new copeland(CM_WV),
				false, ct[counter]));
		condorcets.push_back(new gradual_cond_borda(new copeland(CM_WV,
					2, 2, 1), true, ct[counter]));
		condorcets.push_back(new gradual_cond_borda(new copeland(CM_WV,
					2, 2, 1), false, ct[counter]));
		condorcets.push_back(new gradual_cond_borda(new copeland(CM_WV,
					2, 1, 0), true, ct[counter]));
		condorcets.push_back(new gradual_cond_borda(new copeland(CM_WV,
					2, 1, 0), false, ct[counter]));
	}

	//condorcets.push_back(new copeland(CM_WV));

	// TODO: Fix the interaction of this and slash. Same with Cardinal and
	// slash, too; probably same sol'n as Mode-Ratings.
	condorcets.push_back(new randpair(CM_WV));

	condorcets.push_back(new cardinal_ratings(0, 10, true));
	condorcets.push_back(new cardinal_ratings(0, 10, false));
	condorcets.push_back(new vi_median_ratings(10, true, true));
	condorcets.push_back(new vi_median_ratings(10, false, true));
	condorcets.push_back(new vi_median_ratings(10, true, false));
	condorcets.push_back(new vi_median_ratings(10, false, false));

	condorcets.push_back(new mode_ratings());
	vector<election_method *> condorcetsp;
	condorcetsp.push_back(new plurality(PT_WHOLE));
	condorcetsp.push_back(new borda(PT_WHOLE));
	condorcetsp.push_back(new antiplurality(PT_WHOLE));
	condorcetsp.push_back(new for_and_against(PT_WHOLE));
	condorcetsp.push_back(new nauru(PT_WHOLE));
	condorcetsp.push_back(new heismantrophy(PT_WHOLE));
	condorcetsp.push_back(new baseballmvp(PT_WHOLE));
	condorcetsp.push_back(new eurovision(PT_WHOLE));
	condorcetsp.push_back(new dabagh(PT_WHOLE));
	condorcetsp.push_back(new nrem(PT_WHOLE));
	condorcetsp.push_back(new worstpos(PT_WHOLE));
	condorcetsp.push_back(new worstborda(PT_WHOLE));

	for (counter = 0; counter < condorcetsp.size(); ++counter) {
		condorcets.push_back(condorcetsp[counter]);
		condorcets.push_back(new loser_elimination(
				condorcetsp[counter], true, true));
		condorcets.push_back(new loser_elimination(
				condorcetsp[counter], false, true));
	}

	/*smith_set*/ partition_set xa(false);
	schwartz_set xb;
	landau_set xc;
	condorcet_set xd;
	sdom_set xe;
	cdtt_set xf;
	mdd_set xg(true);
	mdd_set xh(false);
	cgtt_set xi;

	// TODO: Really fix comma. DONE, kinda.
	size_t num_nonmeta_methods = condorcets.size();
	for (counter = 0; counter < num_nonmeta_methods; ++counter) {
		condorcets.push_back(new comma(condorcets[counter], &xa));
		condorcets.push_back(new comma(condorcets[counter], &xb));
		condorcets.push_back(new comma(condorcets[counter], &xc));
		condorcets.push_back(new comma(condorcets[counter], &xd));
		condorcets.push_back(new comma(condorcets[counter], &xe));
		condorcets.push_back(new comma(condorcets[counter], &xf));
		condorcets.push_back(new comma(condorcets[counter], &xg));
		//condorcets.push_back(new comma(condorcets[counter], &xh));
		condorcets.push_back(new comma(condorcets[counter], &xi));
	}

	cout << "There are " << condorcets.size() << " methods." << endl;

	/*test_strategy(new antiplurality(PT_WHOLE));
	return(-1);*/

	for (counter = 0; counter < condorcets.size(); ++counter) {
		cout << counter << ": " << flush;
		test_strategy(condorcets[counter], randomizer);
	}

	return (0);
}
