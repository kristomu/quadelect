#pragma once

#include "singlewinner/pairwise/simple_methods.h"

#include "common/ballots.h"
#include "methods.h"
#include <list>

// This is the "Proportionality of Solid Coalitions by
// Condorcet Loser Elimination" method.
// https://electowiki.org/wiki/PSC-CLE

// We induce constraints based on solid coalitions and the Droop proportionality
// criterion. Then we start with a set of all candidates, and repeatedly eliminate
// losers of a ranking as long as that doesn't make the resulting set break the
// criteria. Once we have the desired number of candidates, return it.

// Now with more generalization!

class coalition_elimination : public multiwinner_method {

	protected:
		std::shared_ptr<election_method> base_method;

		size_t quota_limit(double total_weight,
			double support, size_t council_size) const;

		double quota_mod;

	public:
		std::list<int> get_council(int council_size,
			int num_candidates, const election_t & ballots) const;

		coalition_elimination(std::shared_ptr<election_method>
			base_method_in) {

			base_method = base_method_in;
			quota_mod = 1;
		}

		coalition_elimination(std::shared_ptr<election_method>
			base_method_in, double quota_mod_in) {

			if (quota_mod_in < 0 || quota_mod_in > 1) {
				throw std::range_error("Coalition-elimination: "
					"Quota modifier must be between 0 and 1.");
			}

			base_method = base_method_in;
			quota_mod = quota_mod_in;
		}

		std::string name() const {
			std::string out_name = "C-Elim[" + base_method->name() + "]";

			if (quota_mod == 0) {
				out_name += " (Hare)";
			} else if (quota_mod < 1) {
				out_name += " (Quota = " + dtos(quota_mod) + ")";
			}

			return out_name;
		}
};

class PSC : public coalition_elimination {

	public:

		PSC() : coalition_elimination(
				std::make_shared<schulze>(CM_WV)) {}

		PSC(double quota_mod_in) : coalition_elimination(
				std::make_shared<schulze>(CM_WV), quota_mod_in) {}

		std::string name() const {
			if (quota_mod == 1) {
				return "PSC-CLE";
			}
			if (quota_mod == 0) {
				return "PSC-CLE (Hare)";
			}
			return "PSC-CLE (Quota = " + dtos(quota_mod) + ")";
		}

};