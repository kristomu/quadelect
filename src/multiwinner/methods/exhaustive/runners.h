#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <numeric>
#include <stdexcept>

#include <math.h>
#include <assert.h>

#include "method.h"

// Runners for the exhaustive methods follow.


// This is for global optimization, currently utilitarian (max sum) only, but
// that will change.

class global_exhaustive_runner : public multiwinner_method {
	private:
		std::shared_ptr<exhaustive_method> method_ref;

	public:
		council_t get_council(size_t council_size,
			size_t num_candidates, const election_t & ballots) const;

		std::string name() const {
			return method_ref->name();
		}

		void set_parameters(std::shared_ptr<exhaustive_method> reference) {
			method_ref = reference;
		}

		global_exhaustive_runner(std::shared_ptr<exhaustive_method> reference) {
			set_parameters(reference);
		}
};

// Ditto sequential.

class sequential_exhaustive_runner : public multiwinner_method {
	private:
		std::shared_ptr<exhaustive_method> method_ref;

	public:
		council_t get_council(size_t council_size,
			size_t num_candidates, const election_t & ballots) const;

		std::string name() const {
			return "Sequential " + method_ref->name();
		}

		void set_parameters(std::shared_ptr<exhaustive_method> reference) {
			method_ref = reference;
		}

		sequential_exhaustive_runner(std::shared_ptr<exhaustive_method>
			reference) {
			set_parameters(reference);
		}
};
