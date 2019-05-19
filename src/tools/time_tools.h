#pragma once

#include <string>
#include <chrono>

// Turn a number of seconds into a years-months-days hh:mm:ss string.
std::string format_time(double seconds_total);

typedef std::chrono::time_point<std::chrono::system_clock> time_pt;
typedef std::chrono::duration<double> time_delta;

time_pt get_now();

double to_seconds(const time_delta & duration);
double to_seconds(const time_pt & then, const time_pt & now);