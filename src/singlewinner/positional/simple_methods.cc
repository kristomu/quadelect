// Different simply implemented positional voting methods.

#include "../../ballots.h"
#include "../method.h"
#include "as241.c"
#include <list>
#include <vector>

#include "simple_methods.h"

using namespace std;

/////////////////////////////////////////////

double nrem::gauss(int curcand, int num_cands) const {

	// Blom's approximation

	double alpha = 0.375;

	return(ppnd16((curcand - alpha)/(num_cands - 2.0*alpha + 1)));
}

double nrem::pos_weight(int position, int last_position) const {

	int numcands = last_position + 1;

	return(gauss(numcands - position, numcands)); // I think?
}
