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

#include "../generator/all.h"

#include "../singlewinner/stats/cardinal.h"
#include "../singlewinner/elimination.h"
#include "../singlewinner/positional/simple_methods.h"

#include "../tests/tests/monotonicity/mono_raise.h"
#include "../singlewinner/pairwise/simple_methods.h"

#include "../random/random.h"

int main() {

	cardinal_ratings cr(0, 10, true);
	plurality plur(PT_WHOLE);
	ext_minmax eminmax(CM_MARGINS, false);
	rng randomizer(1); // TODO: Standard constructor taking seed from time()

	// Generate a random ballot set.
	impartial ic(true, true);

	int seed = 98;
	srand(seed);
	srandom(seed);
	srand48(seed);

	election_t ballots = ic.generate_ballots(17, 4, randomizer);

	// Print 'em.
	ballot_tools btools;

	string f = "!";
	int counter;
	std::map<int, std::string> fakecand;

	for (counter = 0; counter < 26; ++counter) {
		f[0] = (char)('A' + counter);
		fakecand[counter] = f;
	}

	std::vector<std::string> printable = btools.ballots_to_text(
			btools.compress(
				ballots), fakecand, true);
	copy(printable.begin(), printable.end(),
		std::ostream_iterator<std::string>(std::cout,
			"\n"));

	cache_map cache;

	ordering out;
	loser_elimination le_plur(&plur, false, true);
	/*out = eminmax.elect(ballots, 4, cache, false);
	for (counter = 0; counter < 100000; ++counter) {
		out = eminmax.elect(ballots, 4, cache, false);
	}*/

	std::cout << "---" << std::endl;
	ordering_tools otools;
	std::cout << eminmax.name() << ": ";
	std::cout << otools.ordering_to_text(out, fakecand, true) << std::endl;

	// -------------------- //

	// Look for mono-raise errors on a method we know fails it (IRV).
	mono_raise mrtest(true, true);

	for (counter = 0; counter < 400000; ++counter) {
		cache.clear();
		int numcand = 4;
		election_t orig = ic.generate_ballots(
				random() % 17 + 2, numcand, randomizer);

		if (mrtest.pass(&le_plur, orig,
				numcand/*, cache, *(cache_map *)NULL*/) == TFALSE) {
			std::cout << "Found IRV failure with " << numcand << " cands, counter = "
				<<
				counter << std::endl;
		}
		//std::cout << "After that: " << cache.size() << std::endl;
		if (mrtest.pass(&plur, orig,
				numcand/*, cache, *(cache_map *)NULL*/) == TFALSE) {
			std::cout << "Found Plurality failure with " << numcand <<
				" cands, counter = "
				<< counter << std::endl;
		}
	}
	return (0);
}

