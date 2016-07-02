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

#include "../singlewinner/cardinal.cc"
#include "../singlewinner/elimination.cc"
#include "../singlewinner/positional/positional.cc"

#include "../tests/tests/monotonicity.cc"

// TODO, split these. Do that after improving pairwise and implementing tte, 
// though.
#include "../singlewinner/pairwise/methods.cc"

main() {

	cardinal_ratings cr(0, 10, true);
	plurality plur(PT_WHOLE);
	ext_minmax eminmax(CM_MARGINS, false);

	// Generate a random ballot set.
	impartial iic(true, true);

	int seed = 98;
	srand(seed);
	srandom(seed);
	srand48(seed);

	list<ballot_group> ballots = iic.generate_ballots(17, 4);

	// Print 'em.
	ballot_tools btools;

	string f = "!";
	int counter, sec;
	map<int, string> fakecand;

	for (counter = 0; counter < 26; ++counter) {
		f[0] = (char)('A' + counter);
		fakecand[counter] = f;
	}

	vector<string> printable = btools.ballots_to_text(btools.compress(
				ballots), fakecand, true);
	copy(printable.begin(), printable.end(), ostream_iterator<string>(cout,
				"\n"));

	cache_map cache;

	ordering out;
	loser_elimination le_plur(&plur, false, true);
	/*out = eminmax.elect(ballots, 4, cache, false);
	for (counter = 0; counter < 100000; ++counter) {
		out = eminmax.elect(ballots, 4, cache, false);
	}*/

	for (cache_map::const_iterator pos = cache.begin(); pos != cache.end();
			++pos)
		cout << pos->first << endl;

	cout << "---" << endl;
	ordering_tools otools;
	cout << eminmax.name() << ": ";
	cout << otools.ordering_to_text(out, fakecand, true) << endl;

	// -------------------- //
	
	// Look for mono-raise errors on a method we know fails it (IRV).
	mono_raise mrtest(true, true);

	for (counter = 0; counter < 400000; ++counter) {
		cache.clear();
		int numcand = 4;
		list<ballot_group> orig = iic.generate_ballots(
				random() % 17 + 2, numcand);

		if (mrtest.pass(&le_plur, orig, numcand/*, cache, *(cache_map *)NULL*/) == TFALSE)
			cout << "Found IRV failure with " << numcand << " cands, counter = " << counter << endl;
		//cout << "After that: " << cache.size() << endl;
		if (mrtest.pass(&plur, orig, numcand/*, cache, *(cache_map *)NULL*/) == TFALSE)
			cout << "Found Plurality failure with " << numcand << " cands, counter = " << counter << endl;
	}

}

