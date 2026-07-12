#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <limits>
#include <numeric>
#include <stdexcept>

#include <math.h>
#include <assert.h>

#include "multiwinner/score_aggregation/score_aggregate.h"

#include "multiwinner/methods/methods.h"

namespace combo {
typedef std::vector<size_t>::const_iterator it;
}

// This is a "function with state" for exhaustive methods. Exhaustive methods
// involve evaluating some function of the ballots and a proposed council.
// Since they usually don't use the full ballot data, we can speed things up
// by preprocessing it.

// The exhaustive method class both allows preprocessing and evaluating the
// quality of any given proposed council. They're used in conjunction with
// runners; these try different council configurations (e.g. brute-force or
// by greedy improvement).

class exhaustive_method {

	public:
		// This function takes iterators to the beginning and end of the
		// vector of seat assignments, as well as a score aggregate to
		// insert values into, and inserts one or more component
		// objective values, i.e. scores for that particular assignment
		// (usually one per voter, though this may differ).
		virtual void add_method_scores(const combo::it & start,
			const combo::it & end, score_aggregate & output) = 0;

	public:

		virtual ~exhaustive_method() {}

		// This function loads ballots into some private structure
		// to be used by add_method_scores().
		virtual void process_ballots(const election_t & ballots,
			size_t num_candidates) = 0;

		virtual std::string name() const = 0;
};
