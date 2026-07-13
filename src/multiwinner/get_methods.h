#pragma once

#include "methods/all.h"

#include <vector>

const int EXTENSIVE_PARAM_SWEEP = 1,
		  EXPENSIVE_EXPERIMENTAL_METHODS = 2,
		  EXPENSIVE_MAXMIN_METHODS = 4;

std::vector<std::shared_ptr<multiwinner_method> > get_multiwinner_methods(
	int method_selection);
