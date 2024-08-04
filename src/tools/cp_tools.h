#pragma once

#include "linear_model/constraints/cand_pairs.h"

// Tools for candidate pairs (see cand_pairs.h for more info about those).

class cp_tools {
	public:
		static cand_pairs compose(const cand_pairs & first,
			const cand_pairs & second, bool accept_dangling_links);

		static cand_pairs reverse(const cand_pairs & to_reverse);
};