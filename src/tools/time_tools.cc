#include "time_tools.h"
#include "tools.h"

#include <math.h>

std::string format_time(double seconds_total) {
	// Use doubles to handle extreme values, e.g. 1e+40 seconds.

	assert (seconds_total > 0);

	double secs = fmod(seconds_total, 60);
	double x = (seconds_total - secs) / 60.0;
	double mins = fmod(x, 60);
	x = (x - mins) / 60.0;
	double hrs = fmod(x, 24);
	x = (x - hrs) / 24.0;
	double days = fmod(x,365.24);
	x = (x - days) / 365.24;
	double years = x;

	std::string hstr = itos(round(hrs), 2),
		mstr = itos(round(mins), 2), sstr = itos(round(secs), 2);

	// Set up time string.
	std::string time_str;
	if (hrs > 0) {
		time_str = hstr + ":" + mstr + ":" + sstr;
	}
	if (hrs == 0 && mins > 0) {
		time_str = mstr + ":" + sstr;
	}
	if (hrs == 0 && mins == 0) {
		time_str = sstr + "s";
	}

	// Set up date str
	std::string date_str = "";

	if (days > 0) {
		date_str = itos(round(days), 2) + "d and ";
	}
	if (years > 0) {
		date_str = dtos(years) + "y, " + date_str;
	}

	return date_str + time_str;
}

time_pt get_now() {
	return std::chrono::system_clock::now();
}

double to_seconds(const time_delta & duration) {
	return duration.count();
}

double to_seconds(const time_pt & then, const time_pt & now) {
	return to_seconds(now-then);
}