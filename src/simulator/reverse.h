#pragma once

#include <stdlib.h>
#include <string>

#include "tests.h"
#include "../../tools/tools.h"

// For finding the worst test instead of the best.

// We don't need an RNG for this... is the constructor in simulator
// going to come back to haunt us?

class pessimal_sim : public simulator {
	private:
		std::shared_ptr<simulator> original;

	protected:
		double do_simulation() {
			return
				original->get_maximum()-original->simulate();
		}

	public:
		pessimal_sim(std::shared_ptr<simulator> original_sim) {
			original = original_sim;
		}

		std::string name() const {
			return "Rev-" + original->name();
		}

		double get_minimum() const {
			return 0;
		}
		double get_maximum() const {
			return
				original->get_maximum()-original->get_minimum();
		}
};