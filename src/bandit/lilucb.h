#ifndef _BANDIT_LIL_UCB
#define _BANDIT_LIL_UCB

#include "bandit.h"
#include <vector>

// lil'UCB-Heuristic according to 
// JAMIESON, Kevin, et al. lil’ucb: An optimal exploration algorithm for 
// multi-armed bandits. In: Conference on Learning Theory. 2014. p. 423-439.

class Lil_UCB {
	private:
		double delta;

		double get_sigma_sq(double minimum, double maximum) const;

		double C(int num_plays_this, int num_bandits,
			double sigma_sq) const;
		double C(const std::vector<Bandit> & bandits, 
			int which, double sigma_sq) const;

	public:
		Lil_UCB(double delta_in) {
			delta = delta_in;
		}

		Lil_UCB() {
			delta = 0.01; // Default
		}

		std::pair<size_t, double> get_best_bandit_so_far(
			const std::vector<Bandit> & bandits, double sigma_sq) const;
		
		std::pair<size_t, double> get_best_bandit_so_far(
			const std::vector<Bandit> & bandits) const;

		// Returns 1 if we're confident of the result, otherwise a
		// status number on [0,1] indicating how close we are to
		// being confident.
		double pull_bandit_arms(std::vector<Bandit> & bandits, 
			int maxiters, bool verbose);
};

#endif