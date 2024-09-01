#pragma once

// This contains code for determining solid and acquiescing coalitions.
// These are used in the Descending Solid Coalitions and
// Descending Acquiescing Coalitions methods respectively. The former is
// also used for determining sets based on solid coalitions (such as the
// mutual majority set).

// Helper class. Coalition X is greater than Y if X's support is greater, or
// if it's equal and X contains the priority candidate whereas Y does not.
// The priority candidate mechanism is used for tiebreaks in Descending Solid
// Coalitions. (Maybe it should be separated out.)

#include <set>
#include <vector>

#include "common/ballots.h"

class coalition_data {
	public:
		std::set<size_t> coalition;
		double support;
		int priority_candidate;

		bool operator>(const coalition_data & other) const {
			if (support != other.support) {
				return (support > other.support);
			}

			return (coalition.find(priority_candidate) != coalition.end() &&
					other.coalition.find(priority_candidate) ==
					other.coalition.end());
		}

		coalition_data(double support_in, const std::set<size_t> & coalition_in) {
			coalition = coalition_in;
			support = support_in;
			priority_candidate = -1;
		}
};

// Should we put this in a class instead?

std::vector<coalition_data> get_solid_coalitions(
	const election_t & election,
	const std::vector<bool> & hopefuls, int numcands);

std::vector<coalition_data> get_acquiescing_coalitions(
	const election_t & election,
	const std::vector<bool> & hopefuls, int numcands);

void print_coalitions(const std::vector<coalition_data> & coalitions);