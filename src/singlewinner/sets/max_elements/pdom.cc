
#include "pdom.h"

bool pdom_set::relation(const abstract_condmat & input, int a, int b,
	const vector<bool> & hopefuls) const {
	// A Pareto-dominates B if
	// no voter ranks B ahead of A
	// and at least one voter ranks A ahead of B

	/*bool a_dom_b = input.get_magnitude(b, a, hopefuls) == 0 &&
		input.get_magnitude(a, b, hopefuls) > 0;*/

	bool b_dom_a = input.get_magnitude(a, b, hopefuls) == 0 &&
		input.get_magnitude(b, a, hopefuls) > 0;

	// If mutually nondominated, neither get a true here, which means that if
	// neither A dominates B or vice versa, but they all dominate everybody
	// else, then {A, B} becomes the maximal set.

	return (!b_dom_a);

	//return(input.get_magnitude(b, a, hopefuls) == 0 &&
	//	input.get_magnitude(a, b, hopefuls) > 0);
}
