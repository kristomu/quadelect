#include <stdlib.h>

#include "bernoulli.h"

double Bernoulli::perform_test() {
	if (drand48() < p) {
		return(1);
	} else {
		return(0);
	}
}
