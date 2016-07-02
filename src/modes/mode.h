// A very simple ABC for now. We want:
//	- bool init() - prepares the mode. Returns false if certain data, which
//			differ from mode to mode, have not been provided (e.g.
//			election methods for Bayesian regret).
//	- int get_max_rounds() - Determines the maximum number of rounds. Return
//				 sentinel value if the number of rounds isn't
//				 clear? ("until t-tests distinguish them" BR,
//				 for instance, would depend on variances of the
//				 methods themselves). Or perhaps an aux bool for
//				 "is approximation".
//	- int get_current_round() - Determines the current round (you can 
//				    probably see where I'm going with this).
//	- bool do_round() - does just that; runs a round. Returns false if not
//			    inited or everything's finished.
//	- v<string> provide_status() - Gives a set of strings that inform the
//			user about status so far. In BR this is just the stats.

#ifndef _VOTE_MODE
#define _VOTE_MODE

#include "../random/random.h"

#include <vector>
#include <string>

using namespace std;

class mode {

	public:

		virtual bool init(rng & randomizer) = 0;
		virtual int get_max_rounds() const = 0;
		virtual int get_current_round() const = 0;

		virtual string do_round(bool give_brief_status,
				bool reseed, rng & randomizer) = 0;

		virtual vector<string> provide_status() const = 0;
};

#endif
