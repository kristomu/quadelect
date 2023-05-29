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

#include "../generator/impartial.h"

#include "../singlewinner/stats/cardinal.h"
#include "../singlewinner/elimination.h"
#include "../singlewinner/positional/simple_methods.h"

#include "../tests/tests/monotonicity/mono_raise.h"

// TODO, split these. Do that after improving pairwise and implementing tte,
// though.
#include "../singlewinner/pairwise/simple_methods.h"

int main() {

	cardinal_ratings cr(0, 10, true);
	plurality plur(PT_WHOLE);
	ext_minmax eminmax(CM_WV, false);
	rng randomizer(1);

	// Generate a random ballot set.
	impartial ic(true, true);

	int seed = 999;
	srand(seed);
	srandom(seed);
	srand48(seed);

	list<ballot_group> ballots = ic.generate_ballots(4, 4, randomizer);

	// Print 'em.
	ballot_tools btools;

	string f = "!";
	size_t counter;
	map<int, string> fakecand;

	for (counter = 0; counter < 26; ++counter) {
		f[0] = (char)('A' + counter);
		fakecand[counter] = f;
	}

	vector<string> printable = btools.ballots_to_text(btools.compress(
				ballots), fakecand, false);
	copy(printable.begin(), printable.end(), ostream_iterator<string>(cout,
			"\n"));

	cache_map cache;

	ordering out;
	loser_elimination le_plur(&plur, false, true);
	out = eminmax.elect(ballots, 4, &cache, false);
	for (counter = 0; counter < 400000; ++counter) {
		out = eminmax.elect(ballots, 4, &cache, false);
	}

	for (ordering::const_iterator p = out.begin(); p != out.end(); ++p) {
		cout << (char)('A' + p->get_candidate_num()) << "\t";
	}
	cout << endl;

	return (0);
}
