#include "gaussian.h"

#include <math.h>
#include <stdexcept>
#include <limits>
#include <vector>

// If we need it later, implement a general covariance matrix.

// Our normal variate generation is currently based on inverse sampling.
// This not measurably slower than rejection sampling and has the benefit
// that it does not consume extra entropy, thus it can be used for Quasi-Monte
// Carlo. It may be faster to use the Ziggurat algorithm for ordinary Monte
// Carlo; I'll implement that later if necessary.

// The qnorm code is from
// https://gist.github.com/kmpm/1211922/6b7fcd0155b23c3dc71e6f4969f2c48785371292
// and ultimately due to
// WICHURA, Michael J. Algorithm AS 241: The percentage points of the normal
// distribution. Journal of the royal statistical society. Series C
// (applied statistics), 1988, 37.3: 477-484.

double gaussian_dist::qnorm(double p, double mu, double sigma) const {
	if (p < 0 || p > 1) {
		throw std::invalid_argument(
			"The probality p must be bigger than 0 and smaller than 1");
	}
	if (sigma < 0) {
		throw std::invalid_argument(
			"The standard deviation sigma must be positive");
	}

	if (p == 0) {
		return std::numeric_limits<double>::infinity();
	}
	if (p == 1) {
		return std::numeric_limits<double>::infinity();
	}
	if (sigma == 0) {
		return mu;
	}

	double q, r, val;

	q = p - 0.5;

	/*-- use AS 241 --- */
	/* double ppnd16_(double *p, long *ifault)*/
	/*      ALGORITHM AS241  APPL. STATIST. (1988) VOL. 37, NO. 3
	        Produces the normal deviate Z corresponding to a given lower
	        tail area of P; Z is accurate to about 1 part in 10**16.
	*/
	if (fabs(q) <= .425) {
		/* 0.075 <= p <= 0.925 */
		r = .180625 - q * q;
		val =
			q * ((((((
									(r * 2509.0809287301226727 + 3430.575583588128105) * r +
									67265.770927008700853) * r +
								45921.953931549871457) * r + 13731.693765509461125) * r +
						1971.5909503065514427) * r + 133.14166789178437745) * r +
				3.387132872796366608)
			/ (((((((r * 5226.495278852854561 +
										28729.085735721942674) * r + 39307.89580009271061) * r +
								21213.794301586595867) * r + 5394.1960214247511077) * r +
						687.1870074920579083) * r + 42.313330701600911252) * r + 1);
	} else {
		/* closer than 0.075 from {0,1} boundary */

		/* r = min(p, 1-p) < 0.075 */
		if (q > 0) {
			r = 1 - p;
		} else {
			r = p;
		}

		r = sqrt(-log(r));
		/* r = sqrt(-log(r))  <==>  min(p, 1-p) = exp( - r^2 ) */

		if (r <= 5) {
			/* <==> min(p,1-p) >= exp(-25) ~= 1.3888e-11 */
			r += -1.6;
			val = (((((((r * 7.7454501427834140764e-4 +
											.0227238449892691845833) * r + .24178072517745061177) *
									r + 1.27045825245236838258) * r +
								3.64784832476320460504) * r + 5.7694972214606914055) *
						r + 4.6303378461565452959) * r +
					1.42343711074968357734)
				/ (((((((r *
											1.05075007164441684324e-9 + 5.475938084995344946e-4) *
										r + .0151986665636164571966) * r +
									.14810397642748007459) * r + .68976733498510000455) *
							r + 1.6763848301838038494) * r +
						2.05319162663775882187) * r + 1);
		} else {
			/* very close to 0 or 1 */
			r += -5;
			val = (((((((r * 2.01033439929228813265e-7 +
											2.71155556874348757815e-5) * r +
										.0012426609473880784386) * r + .026532189526576123093) *
								r + .29656057182850489123) * r +
							1.7848265399172913358) * r + 5.4637849111641143699) *
					r + 6.6579046435011037772)
				/ (((((((r *
											2.04426310338993978564e-15 + 1.4215117583164458887e-7) *
										r + 1.8463183175100546818e-5) * r +
									7.868691311456132591e-4) * r + .0148753612908506148525)
							* r + .13692988092273580531) * r +
						.59983220655588793769) * r + 1);
		}

		if (q < 0.0) {
			val = -val;
		}
	}

	return mu + sigma * val;
}

std::pair<double, double> gaussian_dist::get_2D(
	double sigma_in, coordinate_gen & coord_source) const {

	return get_2D(0, 0, sigma_in, coord_source);
}

std::pair<double, double> gaussian_dist::get_2D(double xmean,
	double ymean, double sigma_in, coordinate_gen & coord_source) const {

	coord_source.start_query();

	std::pair<double, double> coord = {
		qnorm(coord_source.next_double(), xmean, sigma_in),
		qnorm(coord_source.next_double(), ymean, sigma_in)
	};

	coord_source.end_query();

	return coord;
}