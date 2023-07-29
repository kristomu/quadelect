#pragma once

#include "method.h"
#include "../ballots.h"
#include "../tools/tools.h"
#include "../tools/ballot_tools.h"

#include <list>

// Loser-elimination meta-method. This method takes a base method and
// repeatedly disqualifies the loser (if loser-elimination) or those with a
// below-mean score (if average loser elimination).

// The tiebreaks are by first difference, but they can be set to be by last
// difference with a boolean. If the tie remains, it is broken by random
// candidate, and thus the method is not technically cloneproof. (Might do
// something with that later)

class loser_elimination : public election_method {
	private:
		const election_method * base;

		// If average_loser_elim is true, then all candidates at
		// or below mean score is eliminated. If not, only the loser
		// is eliminated.
		bool average_loser_elim;

		// If first_differences is set to false, then the method breaks
		// ties in last in first out order. The more usual tiebreak is
		// the method of first differences, which is what we use if
		// the variable is true, and that breaks ties by the first
		// ballot that expresses a difference between the tied
		// candidates.
		bool first_differences;

		std::string cached_name;

		ordering break_tie(const ordering & original_ordering,
			const std::list<ordering> & past_ordering,
			int num_candidates) const;

	protected:
		std::pair<ordering, bool> elect_inner(const
			std::list<ballot_group> & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string determine_name() const;

	public:

		loser_elimination(const election_method * base_method,
			bool average_loser, bool use_first_diff);

		std::string name() const {
			return (cached_name);
		}
};