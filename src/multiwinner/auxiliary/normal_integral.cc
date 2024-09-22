#include <iostream>
#include <cmath>

// Box-Muller normal variable generation
std::pair<double, double> rnorm() {
	double u1, u2;
	double sigma = 1, mu = 0;

	do {
		u1 = drand48();
	} while (u1 == 0);

	u2 = drand48();

	double mag = sigma * sqrt(-2.0 * log(u1));
	double z0 = mag * cos(2 * M_PI * u2) + mu;
	double z1 = mag * sin(2 * M_PI * u2) + mu;

	return {z0, z1};
}

double dist(double x, double y) {
	return fabs(x-y);
}

// One part of the integral equation
double delta_part(double x_1, double delta) {
	return (2 * x_1 * erfc(x_1/sqrt(2)) - 2 * sqrt(2/M_PI) * exp(-
				(x_1*x_1)/2) -
			3 * x_1 + sqrt(2/M_PI) + 20) / (2 * delta);
}

// The whole integral equation!
// This calculates the Harmonic voting quality function mean value
// with infinite voters and two winners on a normal distribution,
// placed at x_1 < 0 and -x_1.

// Each voter rates each winner 20 - (distance from voter to winner);
// that's where the 20 comes from. It's really not needed because
// Harmonic is scale invariant, but whatever.
double harmonic_integral(double x_1, double delta) {
	return 2 * (20 - sqrt(2/M_PI) + x_1)/(2 + 2 * delta) +
		2 * delta_part(x_1, delta);
}

// Or do this for a Monte Carlo version.
double harmonic_integral_monte_carlo(double x_1, double delta) {
	double sum = 0;
	size_t maxiter = 1e7;

	double sumIA = 0, sumIB = 0;
	double sumIIA = 0, sumIIB = 0;
	double sumIIIA = 0, sumIIIB = 0;
	double sumIVA = 0, sumIVB = 0;
	double debug = 0;

	if (x_1 > 0) {
		throw std::invalid_argument("Harmonic integral MC: x_1 must be "
			"less than zero.");
	}

	double x_2 = -x_1;

	for (int i = 0; i < maxiter; ++i) {
		double x = rnorm().first; // wasteful but wth
		double contrib = 0;
		if (dist(x, x_1) < dist(x, x_2)) {
			contrib = (20 - dist(x, x_1)) / delta +
				(20 - dist(x, x_2)) / (1 + delta);
		} else {
			contrib = (20 - dist(x, x_2)) / delta +
				(20 - dist(x, x_1)) / (1 + delta);
		}

		if (x < x_1) {
			sumIA += (20 - x_1 + x) / delta;
			sumIB += (20 - x_2 + x) / (1 + delta);
		}
		if (x >= x_1 && x < 0) {
			sumIIA += (20 - x + x_1) / delta;
			sumIIB += (20 - x_2 + x) / (1 + delta);
		}
		if (x > 0 && x < x_2) {
			sumIIIA += (20 - x_2 + x) / delta;
			sumIIIB += (20 - x + x_1) / (1 + delta);
		}
		if (x >= x_2) {
			sumIVA += (20 - x + x_1) / (1 + delta);
			sumIVB += (20 - x + x_2) / delta;
		}

		sum += contrib;
	}

	return sum/maxiter;
}

// https://stackoverflow.com/a/75076427
// Calculate inverse erf by Newton's method.

double inverf(double x) {

	double fx, dfx, dx;

	// initial guess
	double result = x, xold = 0.0,
		   tolerance = 1e-15;

	// iterate until the solution converges
	do {
		xold = result;
		fx = erf(result) - x;
		dfx = 2.0 / sqrt(M_PI) * exp(-pow(result, 2.0));
		dx = fx / dfx;

		// update the solution
		result = result - dx;

	} while (fabs(result - xold) >= tolerance);

	return result;
}

int main() {

	double x_1 = -0.2, x_2 = 0.2;
	double delta = 0.5;

	double recordholder = -1, record = -1;

	for (x_1 = -1; x_1 < 0; x_1 += 0.01) {
		if (harmonic_integral(x_1, delta) > record) {
			recordholder = x_1;
			record = harmonic_integral(x_1, delta);
		}
	}

	x_1 = recordholder;

	std::cout << "Where are the optimal winners for Harmonic voting with " <<
		"delta = " << delta << " ?\n\n";

	std::cout << "For delta = " << delta <<
		", the optimally scoring winners are at " << x_1 << " and " << -x_1 <<
		"\n";
	std::cout << "The quality function is then " << record << "\n";

	std::cout << "Compare semi-analytical optimum: " <<
		sqrt(2) * inverf(1 - (2*delta+3)/(2.0*delta+2)) << "\n";
}