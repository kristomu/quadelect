#pragma once

#include "method.h"

#include "multiwinner/score_aggregation/score_aggregate.h"

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
		void add_method_scores(const combo::it & start,
			const combo::it & end, score_aggregate & output) {

			for (const scored_ballot & ballot: scored_ballots) {
				add_method_score(start, end, ballot, output);
			}
		}

	protected:
		std::vector<scored_ballot> scored_ballots;

		// Use a more imperative style both so we can pass both
		// weight and objective to the aggregate, and so that we
		// can use a leximin aggregate later.
		virtual void add_method_score(const combo::it & start,
			const combo::it & end,
			const scored_ballot & this_ballot,
			score_aggregate & output) = 0;

	public:
		void process_ballots(const election_t & ballots,
			size_t num_candidates);
};
