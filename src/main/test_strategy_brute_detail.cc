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
#include "../singlewinner/brute_force/all.h"
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

void test_strategy(election_method * to_test, rng & randomizer,
                   int num_methods, pure_ballot_generator * ballot_gen,
                   int numvoters, int numcands) {

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

    list<ballot_group> ballots;

    // A bunch of times, generate ballots and clear the cache. Then try
    // these ballots against numerous Condorcet methods. If we have
    // cached the Condorcet data, that should be faster than if we haven't,
    // but one probably needs Valgrind to see the difference.

    int counter;

    cerr << "Now trying " << to_test->name() << endl;

    cache_map cache;

    map<int, string> rcl;
    for (counter = 0; counter < 26; ++counter) {
        string foo = "A";
        foo[0] = 'A' + counter;
        rcl[counter] = foo;
    }

    // --- //

    vector<pure_ballot_generator *> ballotgens;
    ballotgens.push_back(ballot_gen);
    ///ballotgens.push_back(new gaussian_generator(true, false, dimensions, false));
   /* ballotgens.push_back(new dirichlet(true));*/

	StrategyTest st(ballotgens, &ic, numvoters, numcands, randomizer, 
		to_test, 0);

    int worked = 0, f = 1;
    int fmax = 5000; //100000;
    int total_generation_attempts = 0;

    for (f = 0; f < fmax; ++f) {
        if (st.perform_test() == 0) {
            ++worked;
        }

		if ((f & 63) == 63) {
            cerr << "." << flush;
            if ((f & 4095) == 4095)
                cerr << f/(double)fmax << flush;
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

    //#pragma omp critical
    {
        cout << "Worked in " << worked << " ("<< lower_bound << ", " << upper_bound << ") out of " << f << " for " <<
             to_test->name() << " ties: " << total_generation_attempts-f << " (" << tiefreq << ")" << endl;
        cout << "strat;" << ballot_gen->name() << ";" << to_test->name() 
            << ";" << lower_bound << ";" << upper_bound << ";" << tiefreq
            << endl;
    }
}

int main(int argc, const char ** argv) {
    vector<election_method *> condorcets; // Although they aren't.
    vector<election_method *> condorcetsrc;

    size_t counter;

    vector<pairwise_ident> types;
    types.push_back(CM_WV);

    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " [num voters]" 
            << " [num candidates]" << endl;
        return(-1);
    }

    int numvoters = atoi(argv[1]), numcands = atoi(argv[2]);

    //string name = argv[1];
    //ifstream tests_in(argv[1]);
    vector<unsigned long long> tests;

    /*while (!tests_in.eof()) {
        unsigned long long tin;
        tests_in >> tin;
        if (incounter >= got_this_far_last_time &&
                incounter % numprocs == thisproc) {
            condorcetsrc.push_back(new cond_brute(tin));
        }
        ++incounter;
    }*/

    // This is why you shouldn't rely entirely on IC. Antiplurality does
    // extremely well on IC alone, for some reason. (Maybe something to
    // keep in mind for bandits...)
    // condorcets.push_back(new comma(new antiplurality(PT_WHOLE), new smith_set()));
    // Sinkhorn and Keener all have around 0.85 IC susceptibility.
    // MMPO fails to impress. 0.78 IC susceptibility
    //condorcets.push_back(new ext_minmax(CM_PAIRWISE_OPP, false));
    // Smith,FPP also fails to impress. 0.78 IC susceptibility
    //condorcets.push_back(new comma(new plurality(PT_WHOLE), new smith_set()));
    // And alas. With 4 cddts, Smith-IRV does much better than fpA-fpC.
    condorcetsrc.push_back(new loser_elimination(new plurality(PT_WHOLE), false, true));
    condorcetsrc.push_back(new loser_elimination(new antiplurality(PT_WHOLE), false, true));
    condorcetsrc.push_back(new antiplurality(PT_WHOLE));
    condorcetsrc.push_back(new fpa_experiment());
    condorcetsrc.push_back(new minmaxy_experimental());
    condorcetsrc.push_back(new schulze(CM_WV));

    //cout << "Test time!" << endl;
    //cout << "Thy name is " << name << endl;

    condorcet_set cond;
    smith_set smith;

	for (counter = 0; counter < condorcetsrc.size(); ++counter) {
		if (numcands < 4) {
			// faster
			condorcets.push_back(new comma(condorcetsrc[counter], &cond));
		} else {
			// more general
			condorcets.push_back(new comma(condorcetsrc[counter], &smith));
		}
    }

    cout << "There are " << condorcets.size() << " methods." << endl;

    vector<rng> randomizers;
    randomizers.push_back(rng(0));

    vector<pure_ballot_generator *> ballotgens;
    int dimensions = 4;
    // Something is wrong with this one. Check later.
    //ballotgens.push_back(new dirichlet(false));
    ballotgens.push_back(new gaussian_generator(true, false, dimensions, false));
    ballotgens.push_back(new impartial(true, false));

    for (size_t bg = 0; bg < ballotgens.size(); ++bg) {
	cout << "Using ballot domain " << ballotgens[bg]->name() << endl;
	for (counter = 0; counter < condorcets.size(); counter ++) {
            	cout << "\t" << counter << ": " << flush;
		test_strategy(condorcets[counter], randomizers[0], 
			condorcets.size(), ballotgens[bg], numvoters, numcands);
        }
    }
}