// The fpA-sum fpC and fpA-max fpC generalizations listed in
// https://electowiki.org/wiki/FpA-fpC

// let A be the candidate whose score we are evaluating. Then
// A's score is either fpA - sum over cddts. C beating A pairwise: fpC
// or fpA - max over cddts. C beating A pairwise: fpC.

#pragma once

#include "../method.h"

class fpa_fpc : public election_method {
	protected:
		// Function for combining different first prefs
		virtual double aggregate(double first_prefs,
			double new_first_pref) const = 0;

	public:
		std::pair<ordering, bool> elect_inner(
			const std::list<ballot_group> & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;
};

class fpa_sum_fpc : public fpa_fpc {
	protected:
		double aggregate(double first_prefs,
			double new_first_pref) const {

			return first_prefs + new_first_pref;
		}

	public:
		virtual std::string name() const {
			return "fpA - sum fpC";
		}
};

class fpa_max_fpc : public fpa_fpc {
	protected:
		double aggregate(double first_prefs,
			double new_first_pref) const {

			return std::max(first_prefs, new_first_pref);
		}

	public:
		virtual std::string name() const {
			return "fpA - max fpC";
		}
};