#include "lilucb.h"
#include <vector>

#include <iostream>
#include <stdlib.h>
#include <math.h>

double Lil_UCB::get_sigma_sq(double minimum, double maximum) const {

	// For a distribution almost surely bounded on [b..a], (a-b)^2/4
	// suffices; see footnote 1 of the paper.
	return (maximum-minimum)*(maximum-minimum)/4.0;
}

double Lil_UCB::C(int num_plays_this, size_t num_arms) const {
	// These parameters are set according to the Heuristic version.
	double epsilon = 0, beta = 1/2.0, deltamod = delta/5.0;

	// p. 4

	double inner_log_part = log((1+epsilon) * num_plays_this/deltamod);
	double bigstuff = 2 * sigma_sq * (1 + epsilon) * log(inner_log_part) /
		(double)num_plays_this;
	double score = (1 + beta) * (1 + sqrt(epsilon)) * sqrt(bigstuff);

	return score;
}

void Lil_UCB::load_arms(std::vector<arm_ptr_t> & arms) {

	// Empty the queue of what might already be there.
	// STL priority queues don't have a .clear() for some reason.
	arm_queue = std::priority_queue<queue_entry>();

	total_num_pulls = 0;
	bool played_new_bandit = false;

	double minimum = INFINITY, maximum = -INFINITY;

	size_t arm_idx;

	for (arm_idx = 0; arm_idx < arms.size(); ++arm_idx) {
		if (arms[arm_idx]->get_simulation_count() == 0) {
			arms[arm_idx]->simulate();
			played_new_bandit = true;
		}
		total_num_pulls += arms[arm_idx]->get_simulation_count();

		// Only show status if we've actually done some initial pulls.
		if (played_new_bandit && (arm_idx % 1000 == 0)) {
			std::cout << "Initial pull: " << arm_idx << " of " <<
				arms.size() << " or " <<
				round(arm_idx/(double)arms.size() * 100 * 100)/100.0 << std::endl;
		}

		minimum = std::min(minimum, get_adjusted_min(arms[arm_idx]));
		maximum = std::max(maximum, get_adjusted_max(arms[arm_idx]));
	}

	sigma_sq = get_sigma_sq(minimum, maximum);

	for (arm_idx = 0; arm_idx < arms.size(); ++arm_idx) {
		queue_entry to_add = create_queue_entry(arms[arm_idx],
				arms.size());

		arm_queue.push(to_add);
	}
}

// TODO: Check (with Bernoulli simulators) that we didn't get a regression.

double Lil_UCB::pull_bandit_arms(int max_pulls) {

	// Parameter also set according to spec for Heuristic.
	double lambda = 1 + 10/double(arm_queue.size());

	// Values are set so that if bandit size is 0, we directly return 1.
	int recordholder_pulled = 1, other_pulled = 0;

	size_t num_arms = arm_queue.size();

	for (int i = 0; i < max_pulls; ++i) {
		std::cerr << i << "   " << max_pulls << "   \r" << std::flush;

		queue_entry at_top = arm_queue.top();
		arm_queue.pop();

		// Pull this bandit arm.
		at_top.arm_ref->simulate();
		++total_num_pulls;

		// Update score.
		at_top.MAB_eval = get_eval(at_top.arm_ref, num_arms);

		arm_queue.push(at_top);

		// Check for the termination criterion.
		recordholder_pulled = at_top.arm_ref->get_simulation_count();
		other_pulled = total_num_pulls - recordholder_pulled;

		if (recordholder_pulled >= 1 + lambda * other_pulled) {
			return 1; // TODO: Make more clear what this means.
		}
	}

	return (std::min(recordholder_pulled / (1 + lambda * other_pulled),
				1.0));
}