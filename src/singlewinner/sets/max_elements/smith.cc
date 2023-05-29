
#include "smith.h"

bool smith_set::relation(const abstract_condmat & input, int a, int b,
	const vector<bool> & hopefuls) const {
	// Was get_raw_contest_score. TODO, fix better.
	return (input.get_magnitude(a, b, hopefuls) >=
			input.get_magnitude(b, a, hopefuls));
}
