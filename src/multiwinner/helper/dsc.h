#pragma once

#include <set>
#include <vector>
#include <iostream>
#include <iterator>

#include "common/ballots.h"
#include "tools/tools.h"

// DSC stuff, copied from old code
// I have a better implementation elsewhere, but let's focus on reproducing
// the old code first.

std::set<unsigned short> get_candidates(ordering::const_iterator begin,
	ordering::const_iterator ending, bool debug);

std::multimap<double, std::set<unsigned short> > get_dsc(
	const election_t & input);

// No regard for ties. The real way to do this is to branch on encountering
// a tie, then say all the candidates that the various (recursive) functions
// return as winners are tied for first place.
void simple_dsc(const election_t input, int num_candidates);