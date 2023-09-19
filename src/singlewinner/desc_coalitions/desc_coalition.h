#pragma once

// A general superstructure for descending coalitions methods. Currently the
// only such method implemented is descending solid coalitions, but this
// ABC should make it easy to extend to DAC or HDSC later once the
// algorithm itself has been coded.

#include "../method.h"
#include "../../coalitions/coalitions.h"

class desc_coalition_method : public election_method {

	private:
		void sort_by_candidate(
			std::vector<coalition_data> & coalitions,
			int candidate) const;
		bool can_candidate_win(std::vector<coalition_data> & coalitions,
			const std::set<int> & starting_candidate_set,
			int candidate, int num_candidates) const;

	protected:
		virtual std::vector<coalition_data> get_coalitions(
			const election_t & election,
			const std::vector<bool> & hopefuls,
			int numcands) const = 0;

	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		virtual std::string name() const = 0;
};