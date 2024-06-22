#include "lilucb.h"
#include "../tools/tools.h"

#include <iostream>
#include <chrono>
#include <vector>

#include <stdlib.h>
#include <math.h>

double Lil_UCB::C(size_t num_plays_this, size_t num_arms,
	double sigma_sq) const {
	// These parameters are set according to the Heuristic version, p. 5.
	// TODO: Find deltamod.
	double epsilon = 0, beta = 1/2.0, deltamod = delta/5.0;

	// p. 4

	// TODO? Implement footnote 2?
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

	size_t arm_idx;

	for (arm_idx = 0; arm_idx < arms.size(); ++arm_idx) {
		if (arms[arm_idx]->get_simulation_count() == 0) {
			arms[arm_idx]->simulate(true);
			played_new_bandit = true;
		}
		total_num_pulls += arms[arm_idx]->get_simulation_count();

		// Only show status if we've actually done some initial pulls.
		if (played_new_bandit && (arm_idx % 1000 == 0)) {
			std::cout << "Initial pull: " << arm_idx << " of "
				<< arms.size() << " or "
				<< round(100.0 * arm_idx/(double)arms.size(), 2)
				<< "%" << std::endl;
		}
	}

	for (arm_idx = 0; arm_idx < arms.size(); ++arm_idx) {
		queue_entry to_add = create_queue_entry(arms[arm_idx],
				arms.size());

		arm_queue.push(to_add);
	}
}

// TODO: Check (with Bernoulli simulators) that we didn't get a regression.

double Lil_UCB::pull_bandit_arms(size_t max_pulls, bool show_status) {

	// Parameter also set according to spec for Heuristic, p. 5.
	double lambda = 1 + 10/double(arm_queue.size());

	// Values are set so that if bandit size is 0, we directly return 1.
	size_t recordholder_pulled = 1, other_pulled = 0;

	size_t num_arms = arm_queue.size();

	for (size_t i = 0; i < max_pulls; ++i) {
		if (show_status) {
			std::cerr << i << "   " << max_pulls << "   \r" << std::flush;
		}

		queue_entry at_top = arm_queue.top();
		arm_queue.pop();

		// Pull this bandit arm.
		at_top.arm_ref->simulate(true);
		++total_num_pulls;

		// Update score.
		at_top.MAB_eval = get_eval(at_top.arm_ref, num_arms);

		arm_queue.push(at_top);

		// Check for the termination criterion.
		recordholder_pulled = at_top.arm_ref->get_simulation_count();
		other_pulled = total_num_pulls - recordholder_pulled;

		if (recordholder_pulled >= 1 + lambda * other_pulled) {
			return 1; // We know what the best arm is (with delta confidence).
		}
	}

	return (std::min(recordholder_pulled / (1 + lambda * other_pulled),
				1.0));
}

template<typename T> double to_seconds(const T & duration) {
	return std::chrono::duration<double>(duration).count();
}

double Lil_UCB::timed_pull_bandit_arms(double seconds) {
	auto start = std::chrono::system_clock::now();
	std::chrono::duration<double> s(seconds);
	auto end = start + s;

	// Now we divide the requested time into a number of periods,
	// and cover each using our current estimate of the number of
	// pulls per second. We then update our estimate according to
	// the time actually passed.

	int num_periods = 5;
	double confidence;

	for (int period_idx = 0; period_idx < num_periods; ++period_idx) {

		auto period_start = std::chrono::system_clock::now();
		double period = to_seconds(end-period_start)/(num_periods-period_idx);

		size_t pulls = std::max(1.0, pulls_per_second * period);
		confidence = pull_bandit_arms(pulls, false);

		// If we're confident what the best arm is, return immediately.
		if (confidence == 1) {
			return confidence;
		}

		// If our time's up (say we missed estimating the time it
		// would take), exit early.
		if (std::chrono::system_clock::now() > end) {
			return confidence;
		}

		auto elapsed = std::chrono::system_clock::now() - period_start;
		double elapsed_secs = to_seconds(elapsed);

		double new_pulls_per_second = 0.5 * pulls_per_second +
			0.5 * pulls/elapsed_secs;

		// Quick and dirty thresholding to defend against outliers.
		if (new_pulls_per_second > 2 * pulls_per_second) {
			pulls_per_second *= 2;
		} else {
			pulls_per_second = new_pulls_per_second;
		}
	};

	return confidence;
}
