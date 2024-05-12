// p-norm normalized continuous cumulative voting.

// Each voter's scores are normalized to lie on the interval [0..1), and then
// normalized to have unit p-norm. The initial normalization, although somewhat
// of a hack, is required because otherwise this method fails scale invariance,
// since multiplying every score by alpha will change the norm by ||alpha||^p.
// If 1 < p < infty, then ballots a wider range will be weakened by this effect;
// similarly if 0 < p < 1, they will be strengthened.

// The unit normalization has no effect on p=1 (ordinary cumulative vote) or
// p=infinity (max norm, continuous Range voting).

// The candidate with the highest sum of normalized scores wins.

#pragma once

#include "../method.h"
#include "../meta/filters/mean_utility.h"
#include "../stats/cardinal.h"
#include <memory>

class cumulative_voting : public election_method {

	private:
		double p; // normalization parameter

	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		virtual std::string name() const {
			return "[" + dtos(p) + "-cumulative vote]";
		}

		cumulative_voting(double p_in) {
			if (p_in <= 0) {
				throw std::invalid_argument("Cumulative voting: "
					"p-norm must be positive");
			}
			p = p_in;
		}

		cumulative_voting() : cumulative_voting(1) {}
};