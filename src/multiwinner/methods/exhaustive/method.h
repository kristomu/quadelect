#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <numeric>
#include <stdexcept>

#include <math.h>
#include <assert.h>

#include "lib/combinations/combinations.h"

#include "multiwinner/methods/methods.h"
#include "optima.h"

namespace combo {
typedef std::vector<size_t>::const_iterator it;
}

// Because the combinations library's way of doing things clashes with my
// functional style, there are two classes to this setup. A method, which does
// the calculation, and a runner, which iterates it over every combination. We
// hide this implementation inelegance under typedefs later.

// It's still not ideal. The way we set parameters by cloning onto a type T
// is very hacky, for instance, and exposes the internals to the caller.
// Maybe find a better way to do this later...

// Perhaps also find a way to make a sequential version of the runner.

// TODO: Call this something else than "exhaustive_method", since
// we're going to want to have a sequential runner too, and then the
// method isn't "exhaustive" - just optimization-based.

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

		virtual ~exhaustive_method() {}

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
		council_t get_council(size_t council_size,
			size_t num_candidates, const election_t & ballots) const;

		std::string name() const {
			return params_set.name();
		}

		void set_parameters(const T reference) {
			params_set = reference;
		}

		exhaustive_method_runner() {}
		exhaustive_method_runner(const T reference) {
			set_parameters(reference);
		}
};

template<class T> council_t
exhaustive_method_runner<T>::get_council(
	size_t council_size, size_t num_candidates,
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

	council_t out;

	// FIX LATER - we should cache the result in case the
	// evaluation is expensive.
	for (size_t i: optimum.get_optimal_solution()) {
		out.push_back(i);
	}

	return out;
}

// Duplication HACK

template<class T> class sequential_method_runner : public
	multiwinner_method {
	private:
		T params_set;

	public:
		council_t get_council(size_t council_size,
			size_t num_candidates, const election_t & ballots) const;

		std::string name() const {
			return "Sequential " + params_set.name();
		}

		void set_parameters(const T reference) {
			params_set = reference;
		}

		sequential_method_runner() {}
		sequential_method_runner(const T reference) {
			set_parameters(reference);
		}
};

// TODO? Sequential with multiple step sizes, e.g.
// find the optimal first two candidates, then the
// optimal next two, etc?

// Also TODO? Other kinds of local search? Deletion
// is an obvious one...

template<class T> council_t
sequential_method_runner<T>::get_council(
	size_t council_size, size_t num_candidates,
	const election_t & ballots) const {

	std::vector<size_t> v, already_elected;
	std::vector<bool> already_elected_bool(num_candidates, false);
	//std::iota(v.begin(), v.end(), 0);

	for (size_t seat = 0; seat < council_size; ++seat) {
		T evaluator(params_set);
		evaluator.process_ballots(ballots, num_candidates);

		v = already_elected;
		v.push_back(0);		// Create space for next cddt

		auto next_cddt_pos = v.rbegin();

		for (*next_cddt_pos = 0; *next_cddt_pos < num_candidates;
			++*next_cddt_pos) {
			if (already_elected_bool[*next_cddt_pos]) {
				continue;
			}

			evaluator(v.begin(), v.end()); // Update optimum
		}

		// Get the optimal argument (i.e. best council) seen so far.
		exhaustive_optima optimum(evaluator);

		already_elected.clear();

		for (size_t i: optimum.get_optimal_solution()) {
			already_elected.push_back(i);
			already_elected_bool[i] = true;
		}
	}

	council_t out;

	for (size_t i: already_elected) {
		out.push_back(i);
	}

	return out;
}
