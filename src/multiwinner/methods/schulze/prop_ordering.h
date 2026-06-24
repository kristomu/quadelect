#pragma once

// Schulze Proportional ordering.

// Same shunt trick. Extract this into something that gets an ordered
// list, or "guarantees by typing".

#include "multiwinner/methods/methods.h"
#include "common.h"
#include <list>

class SchulzePOCalc : public SchulzeCommon {
	private:
		std::vector<bool> elected;
		std::vector<long double> path;

		void Elimination();
		size_t Filling_the_next_place();

	public:
		council_t analyze_and_get_outcome();
		council_t analyze_and_get_outcome(size_t council_size);
};

class SchulzePropOrdering : public multiwinner_method {

	public:
		// This is a proportional ordering, so the following
		// function gets a list of the candidates in the order
		// that they are elected.
		council_t get_ordered_council(
			size_t num_candidates, const election_t & ballots) const;

		council_t get_council(size_t council_size,
			size_t num_candidates, const election_t & ballots) const;

		std::string name() const {
			return "Schulze proportional ordering";
		}

		// See shuntsstv.h for caveats about this function; I may
		// remove it later... or possibly replace it with a better
		// estimate of time complexity.

		bool polytime() const {
			return false;
		}

};