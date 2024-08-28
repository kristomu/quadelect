#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <numeric>
#include <stdexcept>

#include <math.h>
#include <assert.h>

#include "lib/combinations/combinations.h"

#include "optima.h"

namespace combo {
typedef std::vector<size_t>::const_iterator it;
}

class exhaustive_method : public multiwinner_method {
	private:
		exhaustive_optima current_optimum;

	protected:
		// This function takes iterators to the beginning and end of the
		// vector of seat assignments, and outputs an objective value, i.e.
		// a score or penalty for that particular assignment.
		virtual double evaluate(combo::it & start, combo::it & end) const = 0;

		// This function loads ballots into some private structure to be used
		// by evaluate(). It is called at the beginning of the election function.
		virtual void process_ballots(const election_t & ballots) = 0;

	public:
		exhaustive_method(bool maximize_in) : current_optimum(maximize_in) {
		}

		//bool operator()(combo::it start, combo::it end) {
		bool testing(combo::it start, combo::it end) {
			double objective_value = evaluate(start, end);

			current_optimum.update(objective_value,
				start, end);

			return false; // Keep going.
		}

		operator exhaustive_optima() {
			return current_optimum;
		}

		std::list<int> get_council(int council_size,
			int num_candidates, const election_t & ballots) const;

		std::string name() const = 0;
};

class shunt {
	public:
		exhaustive_method * x;

		bool operator()(combo::it start, combo::it end) {
			return x->testing(start, end);
		}
};