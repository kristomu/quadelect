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
//			    inited or everything's finished. (TODO? Exceptions instead?)
//	- v<std::string> provide_status() - Gives a set of strings that inform the
//			user about status so far. In BR this is just the stats.
//	- void set_coordinate_gen(use_type, std::shared_ptr<coordinate_gen> generator).
//			Sets the mode's coordinate generator for a particular use type
//			to the given generator - for QMC/MC.

// BLUESKY: Merge operator so we can split this among multiple processes.
// Real Bluesky^n: v<std::string> op for dumping the data that needs to be
// synced - then we can do client-server! (Albeit without redundancy.)

#pragma once

#include "../stats/coordinate_gen.h"

#include <stdexcept>
#include <vector>
#include <string>
#include <memory>
#include <map>

// Common purposes:

// This is the purpose for the coordinate generator that feeds the
// ballot generators, if there's only one such coordinate generator.
const int PURPOSE_BALLOT_GENERATOR = 1;

// This is the purpose that creates candidate data, such as their
// positions in issue space in spatial models. It's most notably used
// in Yee to set the coordinates for the candidate centers.
const int PURPOSE_CANDIDATE_DATA = 2;

// This is the purpose for a coordinate generator that feeds many
// different things. QMC will not work here as the dimension isn't
// fixed, and the use of this purpose indicates a need to refactor.
const int PURPOSE_MULTIPURPOSE = 1000;

class mode {

	protected:
		// This is used to throw exceptions when a purpose intended
		// for a mode is used with another mode.
		virtual bool is_valid_purpose(uint32_t purpose) const = 0;
		std::map<uint32_t, std::shared_ptr<coordinate_gen> >
		coordinate_sources;

	public:

		virtual void set_coordinate_gen(uint32_t purpose,
			std::shared_ptr<coordinate_gen> coord_source) {

			if (!is_valid_purpose(purpose)) {
				throw std::invalid_argument(name() + ": Invalid purpose!");
			}

			coordinate_sources[purpose] = coord_source;
		}

		virtual bool init() = 0;
		virtual int get_max_rounds() const = 0;
		virtual int get_current_round() const = 0;

		virtual std::string do_round(bool give_brief_status) = 0;

		virtual std::vector<std::string> provide_status() const = 0;

		virtual std::string name() const = 0;
};