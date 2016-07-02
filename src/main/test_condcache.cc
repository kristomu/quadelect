#include <assert.h>
#include <errno.h>

#include <iterator> 
#include <iostream> 
#include <fstream>
#include <list>
#include <set>

#include "../ballot_tools.h"
#include "../ballots.h"
#include "../tools.cc"

#include "../generator/ballotgen.cc"

#include "../singlewinner/stats/cardinal.cc"
#include "../singlewinner/elimination.cc"
#include "../singlewinner/positional/positional.cc"

#include "../tests/tests/monotonicity.cc"

// TODO, split these. Do that after improving pairwise and implementing tte, 
// though.
#include "../singlewinner/pairwise/methods.cc"
#include "../singlewinner/pairwise/least_rev.cc"
#include "../singlewinner/pairwise/keener.cc"
//#include "../singlewinner/pairwise/tournament.cc"

main() {

	ext_minmax eminmax(CM_WV, false);

	// Generate a random ballot set.
	impartial iic(true, true);

	int seed = 999;
	srand(seed);
	srandom(seed);
	srand48(seed);

	list<ballot_group> ballots;
	
	// A bunch of times, generate ballots and clear the cache. Then try
	// these ballots against numerous Condorcet methods. If we have
	// cached the Condorcet data, that should be faster than if we haven't,
	// but one probably needs Valgrind to see the difference.
	
	vector<pairwise_type> types;
	types.push_back(CM_WV);
	types.push_back(CM_LV);
	types.push_back(CM_MARGINS);
	types.push_back(CM_LMARGINS);
	types.push_back(CM_PAIRWISE_OPP);
	types.push_back(CM_WTV);
	types.push_back(CM_TOURN_WV);
	types.push_back(CM_TOURN_SYM);
	types.push_back(CM_FRACTIONAL_WV);
	types.push_back(CM_FRACTIONAL_WV);
	types.push_back(CM_RELATIVE_MARGINS);

	int counter;

	vector<election_method *> condorcets;

	for (counter = 0; counter < types.size(); ++counter) {
		condorcets.push_back(new ext_minmax(types[counter], false));
		condorcets.push_back(new ext_minmax(types[counter], true));
		//condorcets.push_back(new minmax(types[counter]));
		condorcets.push_back(new maxmin(types[counter]));
		condorcets.push_back(new least_rev(types[counter], true, 
					false, 1));
		condorcets.push_back(new least_rev(types[counter], false,
					true, 1));
		condorcets.push_back(new copeland(types[counter]));
		condorcets.push_back(new copeland(types[counter], 2, 2, 1));
		condorcets.push_back(new copeland(types[counter], 2, 1, 0));
		condorcets.push_back(new schulze(types[counter]));
		condorcets.push_back(new keener(types[counter], 0.0001, false, false));
	}

	cache_map cache;
	ordering out;

	for (counter = 0; counter < 9001; ++counter) {
		//cout << counter << endl;

		ballots = iic.generate_ballots(100, 4);
		cache.clear();

		for (int sec = 0; sec < condorcets.size(); ++sec) {
			//cout << condorcets[sec]->name() << endl;
			out = condorcets[sec]->elect(ballots, 4, cache, false);
		}
	}
	
	for (ordering::const_iterator p = out.begin(); p != out.end(); ++p)
		cout << (char)('A' + p->get_candidate_num()) << "\t";
	cout << endl;
}
