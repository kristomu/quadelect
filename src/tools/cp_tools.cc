#include "tools.h"
#include "cp_tools.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>

// Compose two candidate pairs. This works as follows: if (x, y) is a
// valid pair according to first, and (y, z) is a valid pair according
// to second, then the composition makes (x, z) into a valid pair. It's
// used when the ISDA layer remaps x to y, and the inner criterion
// remaps y to z.

// If accept_dangling_links is true, then (x,y) with no (y,z) will
// simply be discarded. Otherwise, the existence of any such links
// will cause an exception.
cand_pairs cp_tools::compose(const cand_pairs & first,
	const cand_pairs & second, bool accept_dangling_links) {

	cand_pairs out_pair;

	for (const auto & first_pairs: first) {
		size_t x = first_pairs.first, y = first_pairs.second;

		if (second.num_after_cands_by_before(y) == 0) {
			if (!accept_dangling_links) {
				throw std::logic_error("cp_tools: Dangling link (" + 
					itos(x) + ", " + itos(y) + ") detected!");
			}
			continue;
		}

		for (std::set<size_t>::const_iterator pos = second.begin_by_cand(y);
			pos != second.end_by_cand(y); ++pos) {
			size_t z = *pos;

			// We've verified that (x, y) exists in first and that
			// (y, z) exists in second, so (x, z) is thus a valid
			// combination.

			out_pair.set_pair(x, z);
		}
	}

	return out_pair;
}

// If to_reverse says (x, y) is a valid pair, then the output says (y, x)
// is a valid pair.
cand_pairs cp_tools::reverse(const cand_pairs & to_reverse) {

	cand_pairs reversed;

	for (const auto & pairs: to_reverse) {
		size_t x = pairs.first, y = pairs.second;

		reversed.set_pair(y, x);
	}

	return reversed;
}