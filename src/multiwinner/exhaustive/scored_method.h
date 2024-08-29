#pragma once

#include "method.h"
#include "multiwinner/hard_card.h"

// These methods optimize a function that depends on the proposed
// seat assignment and the number of points given to each candidate
// by each voter.

// This category includes LPV0+ and birational voting.
// (TODO: Add sources.)

/*struct scored_ballot {
	double weight; // #voters
	std::vector<double> scores; // #voters by #cands
};*/

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