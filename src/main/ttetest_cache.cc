#include <assert.h>
#include <errno.h>

#include <iterator>
#include <iostream>
#include <fstream>
#include <list>
#include <set>

#include "../tools/ballot_tools.h"
#include "../ballots.h"
#include "../cache.h"

#include "../generator/impartial.h"

#include "../singlewinner/stats/cardinal.h"
#include "../singlewinner/elimination.h" // Should this be in meta?
#include "../singlewinner/meta/comma.h"
#include "../singlewinner/sets/all.h"
#include "../singlewinner/positional/simple_methods.h"

#include "../reference_tests/tests/monotonicity/mono_raise.h"
#include "../reference_tests/tests/monotonicity/mono_add.h"

#include "../reference_tests/engine/twotest.h"

// TODO, split these. Do that after improving pairwise and implementing tte,
// though.
#include "../singlewinner/pairwise/simple_methods.h"
#include "../singlewinner/pairwise/least_rev.h"

#include "../random/random.h"

// TODO: Check if caching works. If not, find out why not. Should be relatively
// simple to just change mod_cache to NULL and compare times. In such a
// comparison, the "disregard if we already have a disproof" thing should be
// set to false, so that doesn't factor into runtimes.

int main() {

	std::cout << "BEWARE: This program is experimental and "
		<< "currently doesn't work!\n\n";

	cardinal_ratings cr(0, 10, true);
	plurality plur(PT_WHOLE);

	// Will pass mat. We need Smith. TODO: Move that over, and comma, of course.
	ext_minmax eminmax(CM_MARGINS,
		false);
	ext_minmax eminmin(CM_MARGINS, true);

	// TODO: Absolve it of this, as the type doesn't matter. That requires a
	// criterion compliance thing and then caching will take care of it.
	least_rev cor(CM_MARGINS);
	copeland cpl(CM_MARGINS);
	rng randomizer(1);

	// Generate a random ballot set.
	impartial ic(true, true);

	int seed = 98;
	srand(seed);
	srandom(seed);
	srand48(seed);

	std::list<ballot_group> ballots = ic.generate_ballots(17, 4, randomizer);

	// Print 'em.
	ballot_tools btools;

	std::string f = "!";
	size_t counter;
	std::map<size_t, std::string> fakecand;

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
	loser_elimination le_plur(std::shared_ptr<election_method>(&plur), false,
		true);
	/*out = eminmax.elect(ballots, 4, cache, false);
	for (counter = 0; counter < 100000; ++counter) {
		out = eminmax.elect(ballots, 4, cache, false);
	}*/

	// TODO: "Enumerate cache" debug function.
	/*for (cache_map::const_iterator pos = cache.begin(); pos != cache.end();
			++pos)
		std::cout << pos->first << std::endl;*/

	std::cout << "---" << std::endl;
	ordering_tools otools;
	std::cout << eminmax.name() << ": ";
	std::cout << otools.ordering_to_text(out, fakecand, true) << std::endl;

	// -------------------- //

	// Look for mono-raise and mono-add-top errors
	mono_raise mrtest(true, true);
	mono_add_top mattest(true, true);

	// BLUESKY: Add a way of checking predetermined failure examples so
	// we can quickly "preload" those we already know of.

	// TODO: Either make iter matter here or take it out.
	twotest_engine tte(10, 2, 30, 2, 6);
	tte.add_test(
		&mrtest); // might flake out if in opposite order. Todo, check that.
	tte.add_test(&mattest);

	// Try to make it maximally cache-useful by having a bunch of sets
	// and copies of the same methods for all of them. Ideally, then, the
	// cache should just do a quick lookup for all the derived methods and
	// so should be much quicker than the non-cache case. If not, something
	// is wrong.

	// HACK. TODO fix

	std::vector<std::shared_ptr<election_method> > sets, methods;
	methods.push_back(std::shared_ptr<election_method>(&plur));
	methods.push_back(std::shared_ptr<election_method>(&cpl));
	methods.push_back(std::shared_ptr<election_method>(&eminmax));
	methods.push_back(std::shared_ptr<election_method>(&eminmin));
	methods.push_back(std::shared_ptr<election_method>(&cor));
	methods.push_back(std::shared_ptr<election_method>(&le_plur));

	sets.push_back(std::make_shared<smith_set>());
	sets.push_back(std::make_shared<landau_set>());
	sets.push_back(std::make_shared<cdtt_set>());
	sets.push_back(std::make_shared<schwartz_set>());

	tte.set_generator(&ic);
	for (size_t sec = 0; sec < methods.size(); ++sec) {
		for (counter = 0; counter < sets.size(); ++counter) {

			if (counter == sets.size()) {
				tte.add_method(methods[sec].get());
			} else	tte.add_method(new comma(sets[counter],
						methods[sec]));
		}
	}

	// TODO: Fix mono-raise so that it doesn't make cardinal ratings fail.
	// This means that it should leave other ratings alone, only changing
	// those that are indeed raised/swapped around.

	for (counter = 0; counter < 60000; ++counter) {
		//tte.run_tests(2); // <-- internal?
		// TODO, test that this returns false for IRV/mono-raise only
		// after the violation has been found.
		assert(tte.run_tests(20, randomizer));
	}

	return (0);
}

