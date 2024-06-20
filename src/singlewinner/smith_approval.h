// Smith//Approval (implicit, mean utility)
// The voters rate each candidate, and each ballot gets an effective
// Approval cutoff at the voter's mean utility. Only ranks above this
// cutoff are considered when determining the Smith set. The most
// approved Smith set member wins.

#pragma once

#include "method.h"
#include "meta/filters/mean_utility.h"
#include "meta/filters/meanutil_trunc.h"
#include "sets/max_elements/"
#include "stats/cardinal.h"

#include <memory>

class smith_approval_imp : public election_method {

	private:
		std::shared_ptr<mean_utility> muthresh;
		std::shared_ptr<mean_utility_trunc> mutrunc;
		std::shared_ptr<smith_set> smith;
		std::shared_ptr<cardinal_ratings> approval;

	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		virtual std::string name() const {
			return "Smith//Approval (implicit, mean utility cutoff)";
		}

		smith_approval_imp() {
			approval = std::make_shared<cardinal_ratings>(0, 1, false);
			smith = std::make_shared<smith_set>();
			muthresh = std::make_shared<mean_utility>(approval);
			mutrunc = std::make_shared<mean_utility_trunc>(smith);
		}
};
