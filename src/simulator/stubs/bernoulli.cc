#include "bernoulli.h"

double bernoulli_stub::do_simulation() {
	if (entropy_source->next_double() < p) {
		return 1;
	} else {
		return 0;
	}
}
