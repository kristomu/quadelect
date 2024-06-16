// Different simply implemented positional voting methods.

#include "../../ballots.h"
#include "../method.h"
#include <list>
#include <vector>

#include "simple_methods.h"

/////////////////////////////////////////////

// TODO: What's this?

double nrem::gauss(size_t curcand, size_t num_cands) const {

	// Blom's approximation

	double alpha = 0.375;

	return (ppnd16((curcand - alpha)/(num_cands - 2.0*alpha + 1)));
}

double nrem::pos_weight(size_t position, size_t last_position) const {

	int numcands = last_position + 1;

	return (gauss(numcands - position, numcands)); // I think?
}
