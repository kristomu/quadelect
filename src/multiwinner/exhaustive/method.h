#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <numeric>
#include <stdexcept>

#include <math.h>
#include <assert.h>

#include "lib/combinations/combinations.h"

#include "multiwinner/methods.h"
#include "optima.h"

namespace combo {
typedef std::vector<size_t>::const_iterator it;
}

// Because the combinations library's way of doing things clashes with my
// functional style, there are two classes to this setup. A mthod, which does
// the calculation, and a runner, which iterates it over every combination. We
// hide this implementation inelegance under typedefs later.

// It's still not ideal. The way we set parameters by cloning onto a type T
// is very hacky, for instance, and exposes the internals to the caller.
// Maybe find a better way to do this later...

class exhaustive_method {
	private:
		exhaustive_optima current_optimum;

	protected:
		// This function takes iterators to the beginning and end of the
		// vector of seat assignments, and outputs an objective value, i.e.
		// a score or penalty for that particular assignment.
		virtual double evaluate(combo::it & start, combo::it & end) = 0;

		// What is the direction of optimization - greater is better or
		// less is better?
		virtual bool maximize() const = 0;

	public:

		bool operator()(combo::it start, combo::it end) {
			double objective_value = evaluate(start, end);

			current_optimum.update(objective_value,
				start, end);

			return false; // Keep going.
		}

		operator exhaustive_optima() {
			// Update the optimum variable so we know which direction
			// is optimum. We don't need to do this before it's returned,
			// so let's do it here instead of wasting cycles inside
			// operator() which will be called tons of times.

			current_optimum.set_optimum_direction(maximize());
			return current_optimum;
		}

		// This function loads ballots into some private structure
		// to be used by evaluate().
		virtual void process_ballots(const election_t & ballots,
			size_t num_candidates) = 0;

		virtual std::string name() const = 0;
};

template<class T> class exhaustive_method_runner : public
	multiwinner_method {
	private:
		T params_set;

	public:
		std::list<int> get_council(int council_size,
			int num_candidates, const election_t & ballots) const;

		std::string name() const {
			return T().name();
		}

		void set_parameters(const T reference) {
			params_set = reference;
		}
};

template<class T> std::list<int> exhaustive_method_runner<T>::get_council(
	int council_size, int num_candidates,
	const election_t & ballots) const {

	std::vector<size_t> v(num_candidates);
	std::iota(v.begin(), v.end(), 0);

	// This is a bit hacky, but necessary due to how
	// for_each_combination works. We clone our derived
	// class, pass it as a function, and then this clone
	// carries out all the calculations and returns it
	// through operator(). The constructor initializes
	// any auxiliary structures we might need, like score
	// arrays.
	T evaluator(params_set);
	evaluator.process_ballots(ballots, num_candidates);

	exhaustive_optima optimum = for_each_combination(v.begin(),
			v.begin() + council_size, v.end(), evaluator);

	std::list<int> out;

	// FIX LATER
	for (size_t i: optimum.get_optimal_solution()) {
		out.push_back(i);
	}

	return out;
}