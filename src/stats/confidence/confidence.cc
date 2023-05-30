// This class gives confidence intervals for certain statistical
// distributions, currently t and binomial proportion confidence intervals.

// Algorithms taken from
// https://afni.nimh.nih.gov/pub/dist/src_OBSOLETE/mri_stats.c
// ultimately AS 63 appl. statist. (1973), vol.22, no.3, for the incomplete
// beta function and AS 103 for the inverse.

#include "confidence.h"

// Logarithm of the complete beta function.
double confidence_int::lnbeta(double p, double q) const {
	return (gamma(p) + gamma(q) - gamma(p+q));
}

double confidence_int::incbeta(double x, double p, double q,
	double beta) const {

	const double ACU = 1e-15; // Desired accuracy.

	double psq, temp, rx;
	int ns;

	if (p <= 0.0 || q <= 0.0) {
		return (-1.0);    /* error! */
	}
	if (x <= 0.0 || x >= 1.0) {
		return (x);
	}

	/**  change tail if necessary and determine s **/

	psq = p+q;
	if (p < psq*x) {
		return (1 - incbeta(1-x, q, p, beta));
	}

	double term = 1, ai = 1, betain = 1;

	ns     = q + (1-x)*psq;

	/** use soper's reduction formulae **/
	rx = x/(1-x);

	// Cleaned up a lot! -KM-

	while (fabs(term) > ACU || fabs(term) > ACU * betain) {
		if (ns >= 0) {
			temp = q-ai;
			if (ns == 0) {
				rx = x;
			}
		} else {
			temp = psq++;
		}

		term    = term*temp*rx/(p+ai);
		betain += term;

		++ai;
		--ns;
	}

	return (betain * exp(p * log(x) + (q-1) * log(1-x) - beta) / p);
}


// Yuck. Make proper later.
double confidence_int::incbeta_inverse(double alpha, double p, double q,
	double beta) const {

	double fpu, xinbta, a, r, y, t, s, h, w, acu,
		   yprev, prev, sq, g, adj, tx, xin;

	tx = 0;
	fpu = 1e-16;

	if (p <= 0.0 || q <= 0.0 || alpha < 0.0 || alpha > 1.0) {
		throw std::domain_error("incbeta: input parameter out of bounds");
	}

	if (alpha == 0 || alpha == 1) {
		return (alpha);
	}

	/** change tail if necessary **/

	if (alpha > 0.5) {
		return (1 - incbeta_inverse(1-alpha, q, p, beta));
	}

	a    = alpha;

	/** calculate the initial approximation **/

	r = sqrt(-log(a*a));
	y = r - (2.30753 + 0.27061*r) / (1.0+(0.99229+0.04481*r)*r);
	if (p > 1 && q > 1) {
		r = (y*y-3)/6.0;
		s = 1.0/(p+p-1);
		t = 1.0/(q+q-1);
		h = 2.0/(s+t);
		w = y*sqrt(h+r)/h-(t-s)*(r+5.0/6.0-2.0/(3.0*h));
		xinbta = p/(p+q*exp(w+w));
	} else {
		r = q+q;
		t = 1.0/(9.0*q);
		t = r * pow((1.0-t+y*sqrt(t)), 3.0);
		if (t > 0.0) {
			t = (4.0*p+r-2.0)/t;
			if (t <= 1.0) {
				xinbta = exp((log(a*p)+beta)/p);
			} else {
				xinbta = 1.0-2.0/(t+1.0);
			}
		} else {
			xinbta = 1.0-exp((log((1.0-a)*q)+beta)/q);
		}
	}

	/** solve for x by a modified newton-raphson method **/

	r     = 1.0-p;
	t     = 1.0-q;
	yprev = 0.0;
	sq    = 1.0;
	prev  = 1.0;
	if (xinbta < 0.0001) {
		xinbta = 0.0001;
	}
	if (xinbta > 0.9999) {
		xinbta = 0.9999;
	}

	acu = fpu;
	double ft_tolerance;

	do {
		y = incbeta(xinbta, p, q, beta);
		if (y < 0.0) {
			return -1.0;
		}
		xin = xinbta;
		y = (y-a)*exp(beta+r*log(xin)+t*log(1.0-xin));
		if (y*yprev <= 0.0) {
			prev = std::max(sq, fpu);
		}
		g = 1.0;

lab9:
		adj = g*y;
		sq = adj*adj;
		if (sq < prev) {
			tx = xinbta-adj;
		}
		if (tx >= 0.0 && tx <= 1.0) {
			goto lab11;
		}

lab10:
		g = g/3.0;
		goto lab9;

lab11:
		if (tx == 0.0  || tx == 1.0) {
			goto lab10;
		}
		ft_tolerance = fabs(xinbta-tx);

		xinbta = tx;
		yprev = y;
	} while (prev > acu && y*y > acu && ft_tolerance > fpu);

	return xinbta;
}

double confidence_int::qbeta(double p, double a, double b) const {
	double lnbeta_in = lnbeta(a, b);
	return (incbeta_inverse(p, a, b, lnbeta_in));
}

double confidence_int::qt(double p, double dof) const {
	double binv, tt;

	if (p < 0.0 || p >= 1 || dof <= 0) {
		throw std::domain_error("qt: input parameter out of bounds");
	}

	if (p == 0.5) {
		return (0);
	}
	if (p > 0.5) {
		return (-qt(1-p, dof));
	}

	binv = qbeta(p*2, 0.5*dof, 0.5);
	tt   = -sqrt(dof*(1.0/binv-1.0));
	return (tt);
}

// Two sided t confidence interval at confidence given by uncertainty.

double confidence_int::t_interval_half_width(double uncertainty,
	double mean, double variance, int num_samples) const {

	// First determine the multiple of sigma for the interval.
	double t_number = fabs(qt(uncertainty/2.0, num_samples-1));

	// Now we have (sample mean) +/- t * sqrt(variance / num_samples).

	double half_width = t_number * sqrt(variance / num_samples);

	return (half_width);
}

std::pair<double, double> confidence_int::t_interval(double uncertainty,
	double mean, double variance, int num_samples) const {

	double half_width = t_interval_half_width(uncertainty, mean, variance,
			num_samples);

	return (std::pair<double, double>(mean - half_width,
				mean + half_width));
}


// Find a binomial proportion confidence interval using the Jeffreys prior.
// Symmetric 1-p confidence interval for a binomial with total number
// of draws n, and k successes.
std::pair<double, double> confidence_int::bin_prop_interval(double p,
	int k, int n) const {

	double low_p = p * 0.5, high_p = 1 - p * 0.5;

	std::pair<double, double> prop_interval(
		qbeta(low_p, k + 0.5, n - k + 0.5),
		qbeta(high_p, k + 0.5, n - k + 0.5));

	return (prop_interval);
}

bool confidence_int::test() const {
	// All the reference values are from R.

	// Can handle around 4e-7, but I found that value after
	// writing the tests, so it wouldn't necessarily pass cross-
	// validation. Hence I'm certifying a safer 1e-6.
	double tolerance = 1e-6;

	if (fabs(qbeta(0.05, 10, 20)   - 0.2004957) > tolerance) {
		return (false);
	}
	if (fabs(qbeta(0.05, 90, 0.01) - 0.9999624) > tolerance) {
		return (false);
	}
	if (fabs(qbeta(0.05, 0.01, 90) - 1e-131) > tolerance) {
		return (false);
	}
	if (fabs(qbeta(0.55, 45, 100)  - 0.3143163) > tolerance) {
		return (false);
	}

	// These fail at tolerance 1e-7.

	if (fabs(qt(0.05, 5.3) - (-1.990124)) > tolerance) {
		return (false);
	}
	if (fabs(qt(0.45, 5.3) - (-0.1318013)) > tolerance) {
		return (false);
	}
	if (fabs(qt(0.95, 5.3) -   1.990124) > tolerance) {
		return (false);
	}

	// T interval test
	// mean = 0.5, variance = 0.3, 10 samples, confidence 0.05.
	// dof is thus 9 (num samples - 1)
	// The quantile is qt(1 - 0.05/2, 9) = 2.262157
	// The score due to the observations is 0.1732051
	// Thus the confidence interval should be 0.5 +/- 0.3918171.

	std::pair<double, double> test_interval_t = t_interval(0.05,
			0.5, 0.3, 10);
	double half_width = t_interval_half_width(0.05, 0.5, 0.3, 10);

	if (fabs(test_interval_t.first - 0.1081829) > tolerance) {
		return (false);
	}
	if (fabs(test_interval_t.second - 0.8918171) > tolerance) {
		return (false);
	}
	if (fabs(half_width - 0.3918171) > tolerance) {
		return (false);
	}

	// Binomial proportion test
	// not really needed, but eh...
	// successes = 20, tries = 100, p = 0.05
	// Jeffreys prior puts the interval from qbeta(0.025, 20.5, 80.5)
	//	= 0.1307904
	// to qbeta(0.975, 20.5, 80.5) = 0.2862796.

	std::pair<double, double> test_interval_bp = bin_prop_interval(0.05,
			20, 100);

	if (fabs(test_interval_bp.first - 0.1307904) > tolerance) {
		return (false);
	}
	if (fabs(test_interval_bp.second - 0.2862796) > tolerance) {
		return (false);
	}

	return (true);
}

#ifdef _TEST_CI

main() {
	confidence_int x;
	x.test();
}

#endif