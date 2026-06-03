#pragma once

#include "methods/all.h"

#include <vector>

// TODO: include file that includes shared_ptr

std::vector<std::shared_ptr<multiwinner_method> >
get_multiwinner_methods(bool extensive_param_sweep,
	bool expensive_experimental_methods);