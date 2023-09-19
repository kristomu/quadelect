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

#include "../generator/ballotgen.h"
#include "../generator/impartial.h"
#include "../generator/dirichlet.h"
#include "../generator/spatial/all.h"

#include "../singlewinner/gradual_c_b.h"

#include "../singlewinner/stats/cardinal.h"
#include "../singlewinner/elimination.h"
#include "../singlewinner/positional/positional.h"
#include "../singlewinner/positional/simple_methods.h"

//#include "../tests/tests/monotonicity.h"

#include "../stats/stats.h"

#include "../distances/vivaldi_test.h"

// TODO, split these. Do that after improving pairwise and implementing tte,
// though.
#include "../singlewinner/pairwise/method.h"
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
#include "../singlewinner/sets/max_elements/smith.h"
#include "../singlewinner/sets/max_elements/schwartz.h"
#include "../singlewinner/sets/max_elements/landau.h"
#include "../singlewinner/sets/max_elements/c_tt.h"
#include "../singlewinner/sets/max_elements/sdom.h"
#include "../singlewinner/meta/comma.h"

#include "../singlewinner/stats/var_median/vmedian.h"
#include "../singlewinner/stats/mode.h"

#include "../singlewinner/young.h"

#include "../random/random.h"

std::vector<std::vector<bool> > get_impromptu_cm(const ordering & a,
	int numcands) {

	// toRet[a][b] is 1 if the voter ranked A above B, otherwise 0.

	std::vector<std::vector<bool> > toRet(numcands, std::vector<bool>(numcands,
			false));

	for (ordering::const_iterator pos = a.begin(); pos != a.end(); ++pos) {
		ordering::const_iterator sec = pos;

		while (++sec != a.end())
			if (pos->get_score() > sec->get_score())
				toRet[pos->get_candidate_num()]
				[sec->get_candidate_num()] = true;
	}

	return (toRet);
}

void ktau(const std::vector<ordering> & orderings,
	std::vector<std::vector<double> > & distances, int numcands) {

	// Make CMs
	int num_methods = orderings.size();

	std::vector<std::vector<std::vector<bool> > > cms(num_methods);

	int counter, sec;

	for (counter = 0; counter < num_methods; ++counter) {
		cms[counter] = get_impromptu_cm(orderings[counter], numcands);
	}

	// Get Kendall tau distances.
	for (counter = 0; counter < num_methods; ++counter)
		for (sec = counter+1; sec < num_methods; ++sec) {
			for (int tri = 0; tri < numcands; ++tri)
				for (int tet = tri+1; tet < numcands; ++tet)
					if (cms[counter][tri][tet] !=
						cms[sec][tri][tet]) {
						++distances[counter][sec];
					}

			distances[sec][counter] = distances[counter][sec];
		}
}

// Alternate winner-only check: distance between two orderings is equal to the
// symmetric difference between the two winner sets.

set<int> get_winners(const ordering & a) {

	std::set<int> winners_a; // You don't like winners. Winners? Yes, winners.

	for (ordering::const_iterator pos = a.begin(); pos != a.end() &&
		pos->get_score() == a.begin()->get_score(); ++pos) {
		winners_a.insert(pos->get_candidate_num());
	}

	return (winners_a);
}

void winner_check(const std::vector<ordering> & orderings,
	std::vector<std::vector<double> > & distances, int numcands) {

	std::vector<std::set<int> > winners;
	std::vector<int> symdif, unions;
	size_t counter, sec;

	for (counter = 0; counter < orderings.size(); ++counter) {
		winners.push_back(get_winners(orderings[counter]));
	}

	for (counter = 0; counter < orderings.size(); ++counter)
		for (sec = counter+1; sec < orderings.size(); ++sec) {
			symdif.clear();
			unions.clear();

			set_symmetric_difference(winners[counter].begin(),
				winners[counter].
				end(), winners[sec].begin(),
				winners[sec].end(),
				back_inserter(symdif));

			set_union(winners[counter].begin(),
				winners[counter].end(),
				winners[sec].begin(),
				winners[sec].end(),
				back_inserter(unions));

			distances[counter][sec] += symdif.size()/(double)unions.size();
			distances[sec][counter] = distances[counter][sec];
		}
}

int main() {

	// Generate a random ballot set.
	// First should be true: compress the ballots. Second, that's
	// whether truncation is permitted.
	// TODO later: fix last-rank problem with gradual-cond-borda.
	// Also fix memory-hogging loop of doom in positional if this is set on.
	//impartial ic(/*true, true*/true, true);
	//spatial_generator ic(true, true);
	uniform_generator spatial(true, false);
	impartial ic(true, false);
	rng randomizer(1);

	int seed = 999;
	srand(seed);
	srandom(seed);
	srand48(seed);

	election_t ballots;

	// A bunch of times, generate ballots and clear the cache. Then try
	// these ballots against numerous Condorcet methods. If we have
	// cached the Condorcet data, that should be faster than if we haven't,
	// but one probably needs Valgrind to see the difference.

	std::vector<pairwise_ident> types;
	types.push_back(CM_WV);
	//types.push_back(CM_LV);
	//types.push_back(CM_MARGINS);
	//types.push_back(CM_LMARGINS);  // may have negative values
	types.push_back(CM_PAIRWISE_OPP);
	//types.push_back(CM_WTV);
	types.push_back(CM_TOURN_WV);
	//types.push_back(CM_TOURN_SYM); // may have negative values
	types.push_back(CM_FRACTIONAL_WV);
	types.push_back(CM_RELATIVE_MARGINS);
	//types.push_back(CM_KEENER_MARGINS);

	size_t counter;

	std::vector<election_method *> condorcets;
	std::vector<stats<float> > method_stats;

	stats_type br_type = MS_INTERROUND;

	double power_min = 0.25;
	double power_max = 2.5;
	double powerstep = 0.25;

	for (counter = 0; counter < types.size(); ++counter) {
		//condorcets.push_back(new kemeny(types[counter]));
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
		//condorcets.push_back(new ranked_pairs(types[counter], true));
		condorcets.push_back(new dquick(types[counter]));
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

		election_method * p = *condorcets.rbegin();

		std::cout << "Last is " << p->name() << std::endl;
	}

	std::vector<completion_type> ct;
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
	condorcets.push_back(new randpair(CM_WV));

	condorcets.push_back(new cardinal_ratings(0, 10, true));
	condorcets.push_back(new cardinal_ratings(0, 10, false));
	condorcets.push_back(new vi_median_ratings(10, true, true));
	condorcets.push_back(new vi_median_ratings(10, false, true));
	//condorcets.push_back(new vi_median_ratings(10, true, false));
	//condorcets.push_back(new vi_median_ratings(10, false, false));

	condorcets.push_back(new mode_ratings());
	/*condorcets.push_back(new young(false, true));
	condorcets.push_back(new young(true, true));*/
	/*condorcets.push_back(new copeland(CM_WV));
	condorcets.push_back(new kemeny(CM_WV));
	condorcets.push_back(new kemeny(CM_MARGINS));
	condorcets.push_back(new schulze(CM_WV));
	condorcets.push_back(new schulze(CM_MARGINS));*/

	// Some positional ones and other fun.
	std::vector<election_method *> condorcetsp;
	condorcetsp.push_back(new plurality(PT_WHOLE));
	condorcetsp.push_back(new borda(PT_WHOLE));
	condorcetsp.push_back(new antiplurality(PT_WHOLE));
	condorcetsp.push_back(new for_and_against(PT_WHOLE));
	condorcetsp.push_back(new nauru(PT_WHOLE));
	//condorcetsp.push_back(new heismantrophy(PT_WHOLE));
	//condorcetsp.push_back(new baseballmvp(PT_WHOLE));
	//condorcetsp.push_back(new eurovision(PT_WHOLE));
	//condorcetsp.push_back(new dabagh(PT_WHOLE));
	condorcetsp.push_back(new nrem(PT_WHOLE));
	/*condorcetsp.push_back(new worstpos(PT_WHOLE));
	condorcetsp.push_back(new worstborda(PT_WHOLE));*/

	for (counter = 0; counter < condorcetsp.size(); ++counter) {
		condorcets.push_back(condorcetsp[counter]);
		condorcets.push_back(new loser_elimination(
				condorcetsp[counter], true, true));
		condorcets.push_back(new loser_elimination(
				condorcetsp[counter], false, true));
	}

	smith_set xa;
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
		//condorcets.push_back(new comma(condorcets[counter], &xa));
		condorcets.push_back(new comma(condorcets[counter], &xb));
		/*condorcets.push_back(new comma(condorcets[counter], &xc));
		condorcets.push_back(new comma(condorcets[counter], &xd));*/
		condorcets.push_back(new comma(condorcets[counter], &xe));
		//condorcets.push_back(new comma(condorcets[counter], &xf));
		condorcets.push_back(new comma(condorcets[counter], &xg));
		/*condorcets.push_back(new comma(condorcets[counter], &xh));
		condorcets.push_back(new comma(condorcets[counter], &xi));*/
	}

	for (counter = 0; counter < condorcets.size(); ++counter)
		method_stats.push_back(stats<float>(br_type,
				condorcets[counter]->name(), false));

	std::cout << "There are " << condorcets.size() << " methods." << std::endl;

	std::vector<std::vector<double> > distances(condorcets.size(),
		std::vector<double>(
			condorcets.size(), 0));

	// 900 times:
	//	Generate a bunch of ballots using a generator.
	//	Calculate the outcomes for a bunch of methods.
	//	Find the Kendall-tau distance between the output orderings.
	//		(Note that we require a full ranking here, not just
	//		 winner.)
	//	Add to distance array.
	// done

	// Then use synthetic coordinates class to create, well, synthetic
	// coordinates!

	cache_map cache;
	ordering out;

	// TODO: check if that actually works.
	cardinal_ratings utility(MININT, MAXINT, false);
	int numvoters = 256;
	int initial_numcands = 12, numcands;

	int report_freq = 1000;
	std::vector<double> utilities(initial_numcands);

	std::map<int, std::string> rcl;
	for (counter = 0; counter < 26; ++counter) {
		string foo = "A";
		foo[0] = 'A' + counter;
		rcl[counter] = foo;
	}

	std::vector<ordering> outputs(condorcets.size());

	double maxiters = 4000;

	for (counter = 1; counter < maxiters; ++counter) {
		srandom(counter);
		srand(counter);
		srand48(counter);

		numcands = 5;// + random() % (initial_numcands-3);
		numvoters = 25; //10 + random() % (256-10);
		//numvoters = 5;

		/*numcands = 6;
		numvoters = 13;*/
		std::cout << counter << ": " << numcands << " cands, " << numvoters <<
			" voters"
			<< std::endl;

		if (counter % 2 == 0) {
			ballots = ic.generate_ballots(numvoters, numcands, randomizer);
		} else	ballots = spatial.generate_ballots(numvoters, numcands,
					randomizer);

		cache.clear();

		// Get the utility scores, maximum and minimum.
		out = utility.elect(ballots, numcands, &cache, false);
		double maxval = out.begin()->get_score(), minval =
				out.rbegin()->get_score();

		// Finally, put all the candidate scores in a vector format so
		// we can easily access the utility of the candidate that won
		// in each method.
		ordering::const_iterator opos;
		for (opos = out.begin(); opos != out.end(); ++opos)
			utilities[opos->get_candidate_num()] =
				opos->get_score();

		size_t sec;

		for (sec = 0; sec < condorcets.size(); ++sec) {
			/*std::cout << "Now testing " << condorcets[sec]->name()
				<< std::endl;*/
			outputs[sec] = condorcets[sec]->elect(ballots,
					numcands, &cache, false);
			out = outputs[sec];

			// Add the utility. Since we're talking about
			// regret, the minimum value is actually maxval
			// and the maximum value is minval.
			double nominator = 0, denominator = 0;
			for (opos = out.begin(); opos != out.end() &&
				opos->get_score() ==
				out.begin()->get_score(); ++opos) {
				++denominator;
				nominator += utilities[opos->
						get_candidate_num()];
			}

			method_stats[sec].add_result(maxval, nominator/
				denominator, minval);
			if (counter % report_freq == 0) {
				std::cout << method_stats[sec].
					display_stats(false, 0.05) << std::endl;
			}
		}

		winner_check(outputs, distances, numcands);
	}

	size_t sec;
	for (counter = 0; counter < distances.size(); ++counter)
		for (sec = 0; sec < distances.size(); ++sec) {
			distances[counter][sec] /= maxiters;
		}

	synth_coordinates test;
	std::vector<coord> coords = test.generate_random_coords(distances.size(),
			2);

	coord minimum = coords[0], maximum = coords[0];
	for (counter = 0; counter < coords.size(); ++counter)
		for (sec = 0; sec < coords[counter].size(); ++sec) {
			minimum[sec] = std::min(minimum[sec], coords[counter][sec]);
			maximum[sec] = std::max(maximum[sec], coords[counter][sec]);
		}

	for (counter = 0; counter < coords.size(); ++counter)
		for (sec = 0; sec < coords[counter].size(); ++sec) {
			coords[counter][sec] -= minimum[sec];
			coords[counter][sec] /= (maximum[0]-minimum[0]);
		}

	std::cout << "Distance dump." << std::endl;
	for (counter = 0; counter < coords.size(); ++counter) {
		std::cout << condorcets[counter]->name() << std::endl;
	}

	for (counter = 0; counter < coords.size(); ++counter) {
		for (sec = 0; sec < coords.size(); ++sec) {
			std::cout << distances[counter][sec] << " ";
		}
		std::cout << std::endl;
	}

	std::cout << std::endl << std::endl;

	for (counter = 0; counter < coords.size(); ++counter) {
		copy(coords[counter].begin(), coords[counter].end(),
			std::ostream_iterator<double>(std::cout, " "));
		std::cout << method_stats[counter].get_mean();
		std::cout << "\t" << condorcets[counter]->name() << std::endl;
	}

	return (0);
}
