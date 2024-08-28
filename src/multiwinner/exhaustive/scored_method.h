#pragma once

#include "method.h"

// These methods optimize a function that depends on the proposed
// seat assignment and the number of points given to each candidate
// by each voter.

// This category includes LPV0+ and birational voting.
// (TODO: Add sources.)

struct scored_ballot {
	double weight; // #voters
	std::vector<double> scores; // #voters by #cands
};

class scored_method : public exhaustive_method {
	private:
		double evaluate(combo::it & start, combo::it & end) {
			double sum = 0;

			for (const scored_ballot & ballot: scored_ballots) {
				sum += evaluate(start, end, this_ballot);
			}
		}

	protected:
		std::vector<scored_ballot> scored_ballots;

		double evaluate(combo::it & start, combo::it & end,
			const scored_ballot & this_ballot) const = 0;

		void process_ballots(const election_t & ballots);


	public:
		scored_method(double maximize_in) : exhaustive_method(
				maximize_in) {}
};