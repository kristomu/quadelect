#pragma once

// Abstract base class for interpreters. Interpreters are classes that take
// text input of some form and returns a set of ballots (and possibly candidate
// names). They're used as part of determining winners according to given
// ballots and election methods.

// TODO: Somehow support limited interpreters, e.g. CONDMAT_ROW, CONDMAT_COL,
// positional matrices...

#include "common/ballots.h"
#include "tools/tools.h"

#include <vector>
#include <list>
#include <map>

// Type that holds candidate names and an election.
typedef std::pair<std::map<size_t, std::string>, election_t>
names_and_election;

class interpreter {

	public:
		// Ternary with value for "don't know"?
		virtual bool is_this_format(const std::vector<std::string> &
			inputs) const = 0;

		virtual names_and_election interpret_ballots(
			const std::vector<std::string> & inputs, bool debug) const = 0;

		virtual std::string name() const = 0;

		virtual ~interpreter() {}

		// virtual v<s> explain ?
};
