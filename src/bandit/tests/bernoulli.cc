#include <stdlib.h>

#include "bernoulli.h"

double Bernoulli::perform_test() {
	if (randomizer.next_double() < p) {
		return (1);
	} else {
		return (0);
	}
}
