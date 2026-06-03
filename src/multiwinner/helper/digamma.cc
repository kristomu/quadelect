#include "digamma.h"

#include <stdexcept>
#include <math.h>

// Lightly adapted from
// https://github.com/lteacy/decBRL-cpp/blob/master/src/polygamma/digamma.cpp

double digamma(double x) {
	double c = 8.5;
	double d1 = -0.5772156649;
	double r;
	double s = 0.00001;
	double s3 = 0.08333333333;
	double s4 = 0.0083333333333;
	double s5 = 0.003968253968;
	double value;
	double y;

	//  Check the input.

	if (x <= 0.0) {
		throw std::invalid_argument("digamma: x <= 0 is not supported");
	}

	//  Initialize.

	y = x;
	value = 0.0;

	//  Use approximation if argument <= S.

	if (y <= s) {
		value = d1 - 1.0 / y;
		return value;
	}

	//  Reduce to DIGAMA(X + N) where (X + N) >= C.

	while (y < c) {
		value = value - 1.0 / y;
		y = y + 1.0;
	}

	//  Use Stirling's (actually de Moivre's) expansion if argument > C.

	r = 1.0 / y;
	value = value + log(y) - 0.5 * r;
	r = r * r;
	value = value - r * (s3 - r * (s4 - r * s5));

	return value;
}
