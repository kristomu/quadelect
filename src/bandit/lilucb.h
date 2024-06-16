#pragma once

#include "../simulator/simulator.h"

#include <memory>
#include <vector>
#include <queue>

// lil'UCB-Heuristic according to
// JAMIESON, Kevin, et al. lil’ucb: An optimal exploration algorithm for
// multi-armed bandits. In: Conference on Learning Theory. 2014. p. 423-439.

typedef std::shared_ptr<simulator> arm_ptr_t;

class queue_entry {
	public:
		arm_ptr_t arm_ref;

		// This evaluation score is based on the simulator's adjusted score
		// (so that higher is always better) and on an exploration bias C.
		// It's called "eval" to distinguish it from the simulator score.
		double MAB_eval;

		// a is less than (inferior to) b if a's score is less than b, or
		// they're equal and a was pulled more recently.

		bool operator<(const queue_entry & other) const {
			if (MAB_eval != other.MAB_eval) {
				return (MAB_eval < other.MAB_eval);
			}
			return (arm_ref->get_simulation_count() >=
					other.arm_ref->get_simulation_count());
		}
};

class Lil_UCB {
	private:
		double delta, sigma_sq;
		int total_num_pulls;
		std::priority_queue<queue_entry> arm_queue;

		// Used for timed pulls.
		double pulls_per_second;

		// First define some shim functions used to pretend that a simulator always
		// is maximizing (higher score is better).

		double get_adjusted_max(arm_ptr_t & arm_to_check) {
			if (arm_to_check->higher_is_better()) {
				return arm_to_check->get_maximum();
			} else {
				return -arm_to_check->get_minimum();
			}
		}

		double get_adjusted_min(arm_ptr_t & arm_to_check) {
			if (arm_to_check->higher_is_better()) {
				return arm_to_check->get_minimum();
			} else {
				return -arm_to_check->get_maximum();
			}
		}

		double get_adjusted_mean(arm_ptr_t & arm_to_check) {
			if (arm_to_check->higher_is_better()) {
				return arm_to_check->get_mean_score();
			} else {
				return -arm_to_check->get_mean_score();
			}
		}


		double get_sigma_sq(double minimum, double maximum) const;
		double C(int num_plays_this, size_t num_arms) const;

		double get_eval(arm_ptr_t & arm, size_t num_arms) {
			return (get_adjusted_mean(arm) + C(arm->get_simulation_count(),
						num_arms));
		}

		queue_entry create_queue_entry(arm_ptr_t arm,
			size_t num_arms) {

			queue_entry out;
			out.arm_ref = arm;
			out.MAB_eval = get_eval(arm, num_arms);
			return (out);
		}

	public:
		// This parameter determines how sure we want to be
		// that the identified best arm is actually best. The
		// bandit is allowed to return the wrong bandit with
		// probability delta.
		void set_accuracy(double delta_in) {
			delta = delta_in;
		}

		Lil_UCB(double delta_in) {
			pulls_per_second = 1;
			set_accuracy(delta_in);
		}

		Lil_UCB() {
			pulls_per_second = 1;
			set_accuracy(0.01); // Default
		}

		void load_arms(std::vector<arm_ptr_t > & arms);

		// Slightly clumsy way of permitting polymorphism without limiting
		// ourselves to vectors
		template<typename T> void load_arms(T & arms) {
			std::vector<arm_ptr_t> translation_container;
			for (size_t i = 0; i < arms.size(); ++i) {
				translation_container.push_back(arms[i]);
			}
			load_arms(translation_container);
		}

		// Returns 1 if we're confident of the result, otherwise a
		// status number on [0,1] indicating how close we are to
		// being confident. The output value can thus be used as a
		// progress indicator.
		double pull_bandit_arms(int maxiters, bool show_status);

		double pull_bandit_arms(int maxiters) {
			return pull_bandit_arms(maxiters, true);
		}

		// Pull bandit arms, but for a certain number of seconds
		// instead of a certain number of iterations.
		double timed_pull_bandit_arms(double seconds);

		const arm_ptr_t get_best_arm_so_far() const {
			return (arm_queue.top().arm_ref);
		}

		int get_total_num_pulls() const {
			return total_num_pulls;
		}
};