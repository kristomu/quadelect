#pragma once

#include "../simulator/simulator.h"

#include <memory>
#include <vector>
#include <queue>

// lil'UCB-Heuristic according to
// JAMIESON, Kevin, et al. lil’ucb: An optimal exploration algorithm for
// multi-armed bandits. In: Conference on Learning Theory. 2014. p. 423-439.

typedef std::shared_ptr<simulator> arm;

class queue_entry {
	public:
		std::shared_ptr<simulator> bandit_ref;

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
			return (bandit_ref->get_simulation_count() >=
					other.bandit_ref->get_simulation_count());
		}
};

class Lil_UCB {
	private:
		double delta, sigma_sq;
		int total_num_pulls;
		std::priority_queue<queue_entry> bandit_queue;

		// First define some shim functions used to pretend that a simulator always
		// is maximizing (higher score is better).


		double get_adjusted_max(arm & arm_to_check) {
			if (arm_to_check->higher_is_better()) {
				return arm_to_check->get_maximum();
			} else {
				return -arm_to_check->get_minimum();
			}
		}

		double get_adjusted_min(arm & arm_to_check) {
			if (arm_to_check->higher_is_better()) {
				return arm_to_check->get_minimum();
			} else {
				return -arm_to_check->get_maximum();
			}
		}

		double get_adjusted_mean(arm & arm_to_check) {
			if (arm_to_check->higher_is_better()) {
				return arm_to_check->get_mean_score();
			} else {
				return -arm_to_check->get_mean_score();
			}
		}


		double get_sigma_sq(double minimum, double maximum) const;
		double C(int num_plays_this, size_t num_bandits) const;

		double get_eval(arm & bandit, size_t num_bandits) {
			return (get_adjusted_mean(bandit) + C(bandit->get_simulation_count(),
						num_bandits));
		}

		queue_entry create_queue_entry(arm bandit,
			size_t num_bandits) {

			queue_entry out;
			out.bandit_ref = bandit;
			out.MAB_eval = get_eval(bandit, num_bandits);
			return (out);
		}

	public:
		Lil_UCB(double delta_in) {
			delta = delta_in;
		}

		Lil_UCB() {
			delta = 0.01; // Default
		}

		void load_bandits(std::vector<std::shared_ptr<simulator> > & bandits);

		// Slightly clumsy way of permitting polymorphism without limiting
		// ourselves to vectors
		template<typename T> void load_bandits(T & bandits) {
			std::vector<std::shared_ptr<simulator>> translation_container;
			for (size_t i = 0; i < bandits.size(); ++i) {
				translation_container.push_back(bandits[i]);
			}
			load_bandits(translation_container);
		}

		// Returns 1 if we're confident of the result, otherwise a
		// status number on [0,1] indicating how close we are to
		// being confident.
		double pull_bandit_arms(int maxiters);

		const std::shared_ptr<simulator> get_best_bandit_so_far() const {
			return (bandit_queue.top().bandit_ref);
		}
};