#pragma once

#include "method.h"

// These methods optimize a function that depends on the proposed
// seat assignment and the number of points given to each candidate
// by each voter.

// This category includes LPV0+ and birational voting.

class scored_ballot {
	public:
		double weight; // #voters
		double min, max;
		std::vector<double> scores; // #voters by #cands

		double get_norm_score(size_t candidate) const {
			assert(min <= max);
			assert(scores[candidate] >= min);
			assert(scores[candidate] <= max);

			return renorm(min, max,
					scores[candidate], 0.0, 1.0);
		}
};

class scored_method : public exhaustive_method {
	private:
		double evaluate(combo::it & start, combo::it & end) {
			double sum = 0;

			for (const scored_ballot & ballot: scored_ballots) {
				sum += evaluate(start, end, ballot);
			}

			return sum;
		}

	protected:
		std::vector<scored_ballot> scored_ballots;

		virtual double evaluate(combo::it & start, combo::it & end,
			const scored_ballot & this_ballot) = 0;

	public:
		void process_ballots(const election_t & ballots,
			size_t num_candidates);
};