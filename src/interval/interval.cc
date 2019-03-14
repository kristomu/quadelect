#include "interval.h"
#include <numeric>

Interval exp(Interval in) {
	return boost::numeric::exp(in);	
}

Interval log(Interval in) {
	// I need to do it manually like this because Boost's log
	// sets log 0 = nan, whereas my code assumes log 0 = -inf. Since
	// log is monotone, transforming the endpoints suffice in every
	// case.
	return Interval(log(boost::numeric::lower(in)),
		log(boost::numeric::upper(in)));
}

Interval sqrt(Interval in) {
	return boost::numeric::sqrt(in);
}

bool finite(Interval in) {
	return finite(boost::numeric::lower(in)) && 
		finite(boost::numeric::upper(in));
}

bool isnan(Interval in) {
	return std::isnan(boost::numeric::lower(in)) ||
		std::isnan(boost::numeric::upper(in));
}

Interval get_nan() {
	return Interval(std::numeric_limits<double>::quiet_NaN(),
		std::numeric_limits<double>::quiet_NaN());
}