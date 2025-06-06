#pragma once

#include "det_sets.h"

// This is a modified class for strongly undominated set, which is the set
// of those who aren't strongly dominated by another candidate. The iterated
// set is done in a Copeland-ish manner, and so the det_sets class is heavily
// modified here.

// Y strongly dominates X if
//  1. Y beats X
// and, for all Z distinct from X and Y,
//  2. if Z beats Y, Z beats X even more,
//  3. if Z beats X, Y beats X even more,
//  4. if X beats Z, Y beats Z even more,
//  5. if Y beats Z, Y beats X even more.

// TODO: Move this off or use some more sophisticated method to account for
// e.g. if A is dominated by 1 and B is dominated by A, then ... > A > B, not
// ... > A = B.

// Possible thing to consider: Have the SDom matrix be like the beatpath matrix
// and thus not inside this at all, but inside pairwise/ something.

// [Informal bug check: odd-looking Yee results. May be buggy.]

// We need more testing here.

// NOTE: I strongly suspect this is wrong. It does need testing,
// but I don't know where to get good test vectors.

class sdom_set : public pairwise_method, private det_sets_relation {
	private:

		// truw = dominates, false = nondominated
		bool strongly_dominates(size_t dominator, size_t dominated,
			const abstract_condmat & input,
			const std::vector<bool> & hopefuls) const;

		// Relation is the same as for the Smith set, but on a
		// different matrix.
		bool relation(const abstract_condmat & input, int a,
			int b, const std::vector<bool> & hopefuls) const {
			return (input.get_magnitude(a, b, hopefuls) >=
					input.get_magnitude(b, a, hopefuls));
		}

	public:
		std::pair<ordering, bool> pair_elect(const abstract_condmat & input,
			const std::vector<bool> & hopefuls,
			cache_map * cache, bool winner_only) const;

		sdom_set() : pairwise_method(CM_WV) {
			update_name();
		}

		std::string pw_name() const {
			return ("SDom");
		}

};