#pragma once

// This contains code for determining solid and acquiescing coalitions.
// These are used in the Descending Solid Coalitions and
// Descending Acquiescing Coalitions methods respectively. The former is
// also used for determining sets based on solid coalitions (such as the
// mutual majority set).

// Helper class. Coalition X is greater than Y if X's score is greater, or
// if it's equal and X contains the priority candidate whereas Y does not.
// The priority candidate mechanism is used for tiebreaks in Descending Solid
// Coalitions. (Maybe it should be separated out.)

#include <set>
#include <vector>

#include "../ballots.h"

class coalition_data {
	public:
		std::set<int> coalition;
		double score;
		int priority_candidate;

		bool operator>(const coalition_data & other) const {
			if (score != other.score) {
				return (score > other.score);
			}

			return (coalition.find(priority_candidate) != coalition.end() &&
					other.coalition.find(priority_candidate) ==
					other.coalition.end());
		}

		coalition_data(double score_in, const std::set<int> & coalition_in) {
			coalition = coalition_in;
			score = score_in;
			priority_candidate = -1;
		}
};

std::vector<coalition_data> get_solid_coalitions(
	const election_t & election,
	const std::vector<bool> & hopefuls, int numcands);

std::vector<coalition_data> get_acquiescing_coalitions(
	const election_t & election,
	const std::vector<bool> & hopefuls, int numcands);