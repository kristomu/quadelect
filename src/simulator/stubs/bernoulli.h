#pragma once

#include "../simulator.h"
#include "../../tools/tools.h"
#include "../../random/random.h"

class bernoulli_stub : public simulator {
	private:
		double p;
		bool is_higher_better;

	protected:
		double do_simulation();

	public:
		bernoulli_stub(double p_in, bool is_higher_better_in,
			std::shared_ptr<coordinate_gen> entropy) : simulator(entropy) {

			assert(p_in >= 0 && p_in <= 1);
			p = p_in;
			is_higher_better = is_higher_better_in;
		}

		// Maximize by default.
		bernoulli_stub(double p_in,
			std::shared_ptr<coordinate_gen> entropy) : bernoulli_stub(
					p_in, true, entropy) {}

		bernoulli_stub(double p_in, bool is_higher_better_in,
			rseed_t seed) : bernoulli_stub(p_in, is_higher_better_in,
					std::make_shared<rng>(seed)) {}

		bernoulli_stub(double p_in, rseed_t seed) : bernoulli_stub(
				p_in, std::make_shared<rng>(seed)) {}

		std::string name() const {
			std::string opt_direction = "minimizing";
			if (is_higher_better) {
				opt_direction = "maximizing";
			}

			return "Bernoulli(" + dtos(p) + ", " + opt_direction + ")";
		}

		double get_minimum() const {
			return 0;
		}
		double get_maximum() const {
			return 1;
		}

		bool higher_is_better() const {
			return is_higher_better;
		}
};