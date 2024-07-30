#pragma once

#include <string>
#include <chrono>

// Turn a number of seconds into a years-months-days hh:mm:ss string.
std::string format_time(double seconds_total);

typedef std::chrono::time_point<std::chrono::system_clock> time_pt;
typedef std::chrono::duration<double> time_delta;

time_pt get_now();

double secs_elapsed(const time_delta & duration);
double secs_elapsed(const time_pt & then, const time_pt & now);

// This is a HACK for MSVC compat and should not be used. Fix later.
double FIX_secs_since_epoch();