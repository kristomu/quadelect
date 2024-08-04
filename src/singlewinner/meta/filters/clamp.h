#pragma once

// Linear clamping: map the scale [min_in, max_in) to the integers [0, ..., max_out].
// Values outside of this range get clamped to 0 or n depending.

// This is a cut and paste of normalize. Maybe I can reduce the duplicate code
// later, but I don't think there's much to save.

#include "singlewinner/method.h"
#include "common/ballots.h"
#include "tools/tools.h"

#include <memory>

class clamp : public election_method {

	private:
		std::shared_ptr<const election_method> forward_to;
		double minimum_in, maximum_in;
		double maximum_out;

	protected:
		std::pair<ordering, bool> elect_inner(const election_t &
			papers, const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

	public:

		// the use of an integer input is intentional. The casting to
		// a double makes renormalization easier in the main body of
		// the code.
		clamp(std::shared_ptr<const election_method> forward_in,
			double min_in, double max_in, int max_out) {
			forward_to = forward_in;
			minimum_in = min_in;
			maximum_in = max_in;
			maximum_out = max_out;
		}

		std::string name() const {
			return "Clamp(" + dtos(minimum_in) + "-" + dtos(maximum_in) + "-->" +
				itos(maximum_out) + ")-[" + forward_to->name() + "]";
		}
};