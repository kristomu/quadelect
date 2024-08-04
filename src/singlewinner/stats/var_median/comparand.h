
#include "common/ballots.h"
#include "grad_median/grad_median.h"

#ifndef _VOTE_AUX_COMPARAND
#define _VOTE_AUX_COMPARAND

// This is an auxiliary comparison function/class for rank comparisons: compare
// first by established rank, and if that produces a tie, compare by current
// score.

class gf_comparand {
	private:
		const grad_fracile * ref;

	public:
		gf_comparand(const grad_fracile * ref_in) {
			ref = ref_in;
		}

		bool operator()(const std::pair<double, size_t> & a,
			const std::pair<double, size_t> & b) const {
			if (a.first != b.first) {
				return (a.first < b.first);
			}
			return (ref->get_score(a.second) <
					ref->get_score(b.second));
		}

		bool equals(const std::pair<double, size_t> & a,
			const std::pair<double, size_t> & b) const {
			if (a.first != b.first) {
				return (a.first == b.first);
			}
			return (ref->get_score(a.second) == ref->get_score(
						b.second));
		}
};

#endif
