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
//	- v<std::string> provide_status() - Gives a set of strings that inform the
//			user about status so far. In BR this is just the stats.

// BLUESKY: Merge operator so we can split this among multiple processes.
// Real Bluesky^n: v<std::string> op for dumping the data that needs to be
// synced - then we can do client-server! (Albeit without redundancy.)

// Now that we're aiming to use quasi MC, there's a bit of a problem.
// We want different parts to use either MC or QMC depending on the
// settings. For instance, if we do Yee with Quasi-Monte Carlo, we don't
// want that *candidate positions* to be chosen using QMC! So how do we
// provide such information to the modes without having to turn things
// inside-out by IoC etc? That's not an option because it would ruin the
// current simple design...

#pragma once

#include "../random/random.h"

#include <vector>
#include <string>

class mode {

	public:

		virtual bool init(coordinate_gen & coord_source) = 0;
		virtual int get_max_rounds() const = 0;
		virtual int get_current_round() const = 0;

		virtual std::string do_round(bool give_brief_status,
			bool reseed, coordinate_gen & coord_source) = 0;

		virtual std::vector<std::string> provide_status() const = 0;
};