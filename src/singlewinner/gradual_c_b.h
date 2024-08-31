#pragma once

#include "pairwise/grad_matrix.h"
#include "grad_median/grad_median.h"
#include "pairwise/method.h"
#include "method.h"

#include <iterator>
#include <iostream>
#include <memory>

#include <glpk.h>
#include <assert.h>

// This code needs a review; I think it may be buggy.

class gradual_cond_borda : public election_method {
	private:
		std::shared_ptr<const pairwise_method> base_method;
		std::string cached_name;
		completion_type completion;
		bool cardinal;

		std::string determine_name() const;

	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string name() const {
			return (cached_name);
		}

		gradual_cond_borda(
			std::shared_ptr<const pairwise_method> base_method_in,
			bool cardinal_in, completion_type completion_in);

};
