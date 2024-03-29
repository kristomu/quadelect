#pragma once

#include "bandit.h"
#include <memory>
#include <vector>
#include <queue>

// lil'UCB-Heuristic according to
// JAMIESON, Kevin, et al. lil’ucb: An optimal exploration algorithm for
// multi-armed bandits. In: Conference on Learning Theory. 2014. p. 423-439.

class queue_entry {
	public:
		std::shared_ptr<Bandit> bandit_ref;
		double score;

		// a is less than (inferior to) b if a's score is less
		// than b, or they're equal and a was pulled more recently.
		bool operator<(const queue_entry & other) const {
			if (score != other.score) {
				return (score < other.score);
			}
			return (bandit_ref->get_num_pulls() >=
					other.bandit_ref->get_num_pulls());
		}
};

class Lil_UCB {
	private:
		double delta, sigma_sq;
		int total_num_pulls;
		std::priority_queue<queue_entry> bandit_queue;

		double get_sigma_sq(double minimum, double maximum) const;
		double C(int num_plays_this, size_t num_bandits) const;

		double get_score(Bandit & bandit, size_t num_bandits) {
			return (bandit.get_mean() + C(bandit.get_num_pulls(),
						num_bandits));
		}

		queue_entry create_queue_entry(std::shared_ptr<Bandit> bandit,
			size_t num_bandits) {

			queue_entry out;
			out.bandit_ref = bandit;
			out.score = get_score(*bandit, num_bandits);
			return (out);
		}

	public:
		Lil_UCB(double delta_in) {
			delta = delta_in;
		}

		Lil_UCB() {
			delta = 0.01; // Default
		}

		void load_bandits(std::vector<std::shared_ptr<Bandit> > & bandits);

		// Slightly clumsy way of permitting polymorphism without limiting
		// ourselves to vectors
		template<typename T> void load_bandits(T & bandits) {
			std::vector<std::shared_ptr<Bandit>> translation_container;
			for (size_t i = 0; i < bandits.size(); ++i) {
				translation_container.push_back(bandits[i]);
			}
			load_bandits(translation_container);
		}

		// Returns 1 if we're confident of the result, otherwise a
		// status number on [0,1] indicating how close we are to
		// being confident.
		double pull_bandit_arms(int maxiters);

		const std::shared_ptr<Bandit> get_best_bandit_so_far() const {
			return (bandit_queue.top().bandit_ref);
		}
};