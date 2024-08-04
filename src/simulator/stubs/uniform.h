#pragma once

#include "../simulator.h"
#include "tools/tools.h"
#include "random/random.h"

class uniform_stub : public simulator {
	private:
		double p;
		bool is_higher_better;

	protected:
		double do_simulation() {
			return entropy_source->next_double(0, p);
		}

	public:
		uniform_stub(double p_in, bool is_higher_better_in,
			std::shared_ptr<coordinate_gen> entropy) : simulator(entropy) {

			assert(p_in >= 0 && p_in <= 1);
			p = p_in;
			is_higher_better = is_higher_better_in;
		}

		uniform_stub(double p_in,
			std::shared_ptr<coordinate_gen> entropy) : uniform_stub(
					p_in, false, entropy) {}

		uniform_stub(double p_in, bool is_higher_better_in,
			rseed_t seed) : uniform_stub(p_in, is_higher_better_in,
					std::make_shared<rng>(seed)) {}

		uniform_stub(double p_in, rseed_t seed) : uniform_stub(
				p_in, std::make_shared<rng>(seed)) {}

		std::string name() const {
			std::string opt_direction = "minimizing";
			if (is_higher_better) {
				opt_direction = "maximizing";
			}

			return "Uniform([0 ... " + dtos(p) + "], " + opt_direction + ")";
		}

		double get_minimum() const {
			return 0;
		}
		double get_maximum() const {
			return p;
		}
};