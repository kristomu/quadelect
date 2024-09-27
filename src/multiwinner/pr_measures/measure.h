#pragma once

#include <list>

#include "generator/spatial/spatial.h"
#include "multiwinner/types.h"

// This ABC defines a way to measure proportionality of winners.
// Classes that implement proportionality measures first gather some
// information on the candidates and voters based on a spatial model
// as implemented by a generator; that's the prepare method. Then it
// turns an outcome (a list of winners) into an error measure, where
// lower is better.

class proportionality_measure {
	public:
		virtual void prepare(const positions_election & p_e) = 0;
		virtual double get_error(const council_t & outcome) = 0;
};
