// Approval sorted margins
// https://electowiki.org/wiki/Approval_Sorted_Margins

// We'll be using mean utility thresholding for now. Fix later -
// probably going to need default constructors for the filters,
// and then a way to add the base method into them after the fact.
// (and an ABC so we can use "filter" as a designated type...)

#pragma once

#include "../method.h"
#include "../meta/filters/mean_utility.h"
#include "../stats/cardinal.h"
#include <memory>

class app_sorted_margins : public election_method {

	private:
		// Used for turning rated ballots into approval
		// ballots.
		std::shared_ptr<const election_method> approval_filter;

	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		virtual std::string name() const {
			return "Approval Sorted Margins-[" +
				approval_filter->name() + "]";
		}

		app_sorted_margins() {
			approval_filter = std::make_shared<mean_utility>(
					std::make_shared<cardinal_ratings>(0, 1, false), 1);
		}
};