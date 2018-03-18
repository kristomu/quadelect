#ifndef _BANDIT_LUCB
#define _BANDIT_LUCB

#include "bandit.h"
#include <vector>

class LUCB {
	private:
		double delta, tolerance;

		double C(int num_plays, int num_plays_this, int num_bandits) const;
		double C(int num_plays, const std::vector<Bandit> & bandits, 
			int which) const;

	public:
		LUCB(double delta_in, double tolerance_in) {
			delta = delta_in;
			tolerance = tolerance_in;
		}

		LUCB() {
			delta = 0.01; // Default
			tolerance = 0.01; // ditto
		}

		std::pair<size_t, double> get_best_bandit_so_far(int num_plays,
			const std::vector<Bandit> & bandits, 
			size_t excluded_bandit_idx) const;

		std::pair<int, double> get_best_bandit_so_far(
			const std::vector<Bandit> & bandits) const;

		// Returns 1 if we're confident of the result, otherwise a
		// status number on [0,1] indicating how close we are to
		// being confident.
		double find_best_bandit(std::vector<Bandit> & bandits, 
			int maxiters, bool verbose) const;
};

#endif