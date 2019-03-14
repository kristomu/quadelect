#pragma once

#include <boost/numeric/interval.hpp>

typedef boost::numeric::interval<double, 
	boost::numeric::interval_lib::policies<
		boost::numeric::interval_lib::save_state<
			boost::numeric::interval_lib::rounded_transc_std<double> >,
			boost::numeric::interval_lib::checking_base<double> > > Interval;

Interval exp(Interval in);
Interval log(Interval in);
Interval sqrt(Interval in);

bool finite(Interval in);
bool isnan(Interval in);
Interval get_nan();