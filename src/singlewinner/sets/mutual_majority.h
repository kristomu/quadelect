#pragma once

#include "../method.h"

class mutual_majority_set : public election_method {
	public:
		std::pair<ordering, bool> elect_inner(
			const std::list<ballot_group> & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string name() const {
			return "Mutual majority";
		}
};