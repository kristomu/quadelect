#pragma once

#include "dhwl_mat.h"
#include "methods.h"
#include "singlewinner/pairwise/method.h"

#include <list>
#include <memory>

class reweighted_condorcet : public multiwinner_method {
	private:
		std::shared_ptr<pairwise_method> base;

	public:
		std::list<int> get_council(int council_size,
			int num_candidates,
			const election_t & ballots) const;

		std::string name() const {
			return ("DHwL(" + base->name() + ")");
		}

		reweighted_condorcet(std::shared_ptr<pairwise_method> base_in) {
			base = base_in;
		}

};