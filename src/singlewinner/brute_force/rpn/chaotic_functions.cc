#include "chaotic_functions.h"
#include <math.h>

#include <iostream>

double triangle_wave(double x) {
	double decimal = x - floor(x);
	if (decimal <= 0.5) {
		return(decimal);
	}
	return(1-decimal);
}

// If the interval covers x + 0.5 for some integer x, then the maximum
// must be 0.5. If it contains x + 0, then the minimum must be 0. Apart
// from these particular cases, we can simply evaluate element-wise.
Interval triangle_wave(Interval x) {
	double dec_lower = boost::numeric::lower(x), 
			dec_upper = boost::numeric::upper(x);

	bool contains_half = round(dec_lower) != round(dec_upper);
	bool contains_zero = floor(dec_lower) != floor(dec_upper);

	double lower = triangle_wave(dec_lower),
			upper = triangle_wave(dec_upper);

	double minimum = std::min(lower, upper),
			maximum = std::max(lower, upper);

	if (contains_half) { maximum = 0.5; }
	if (contains_zero) { minimum = 0; }

	return Interval(minimum, maximum);
}

// Generalized Blancmange function, see
// https://en.wikipedia.org/w/index.php?title=Blancmange_curve&oldid=824538531

// Note, x is halved to make the periodicity 2 instead of 1, for reasons
// to do with monotonicity tests. Fix later.
double blancmange(double w, double x) {

	// Quick and dirty stuff to avoid cascading infinities.
	// This is actually undefined in the limit of +/- infinity because
	// the function doesn't converge.
	if (!finite(x)) return(1);

	x /= 2.0;

	double y = 0;
	double pow_of_two = 1;
	double w_power = 1;

	for (int i = 0; i < 64; ++i) {
		y += w_power * triangle_wave(pow_of_two * x);
		pow_of_two *= 2;
		w_power *= w;
	}

	return(y);
}

// I'm not sure if this preserves the inclusion property. Find out later.
Interval blancmange(double w, Interval x) {
	if (!finite(x)) return(1);

	x /= 2.0;

	Interval y = 0;
	double pow_of_two = 1;
	double w_power = 1;

	for (int i = 0; i < 64; ++i) {
		y *= w_power * triangle_wave(pow_of_two * x);
		pow_of_two *= 2;
		w_power *= w;
	}

	return(y);
}


// Minkowski's question-mark function
// https://en.wikipedia.org/w/index.php?title=Minkowski%27s_question-mark_function&oldid=824984747
double minkowski_q(double x) {
	// Quick and dirty stuff to handle infinities.
	if (!finite(x)) return(x);

    long p=x; if ((double)p>x) --p; /* p=floor(x) */
    long q=1, r=p+1, s=1, m, n;
    double d=1, y=p;
    if (x<(double)p||(p<0)^(r<=0)) return x; /* out of range ?(x) =~ x */
    for (;;) /* invariants: q*r-p*s==1 && (double)p/q <= x && x < (double)r/s */
    {
            d/=2; if (y+d==y) break; /* reached max possible precision */
            m=p+r; if ((m<0)^(p<0)) break; /* sum overflowed */
            n=q+s; if (n<0) break; /* sum overflowed */

            if (x<(double)m/n) r=m, s=n;
            else y+=d, p=m, q=n;
    }
    return y+d; /* final round-off */
}

// The Minkowski question mark function is monotone, so we can simply do
// this elementwise expansion. (Perhaps do something with functional
// programming to not repeat myself, later?)
Interval minkowski_q(Interval x) {
	double lower = minkowski_q(boost::numeric::lower(x)), 
			upper = minkowski_q(boost::numeric::upper(x));

	return Interval(std::min(lower, upper), std::max(lower, upper));
}