#pragma once

#include "../bounded.h"

#include "../../ballots.h"

#include "../../generator/ballotgen.h"
#include "../../singlewinner/method.h"
#include "../../random/random.h"

// This class calculates the frequency that a method elects the optimal
// candidate. If the method produces a tie, then the simulation score for
// that election is the chance that a randomly picked winner is the one
// with optimal utility.

// This is a bounded distribution on [0..1] and is very close to
// Bernoulli. It is not exactly Bernoulli due to the tiebreaking
// mechanism.

class utility_freq_sim : public bounded_simulator {
	private:
		size_t numcands, numvoters;

		std::shared_ptr<election_method> method;
		std::shared_ptr<pure_ballot_generator> ballot_gen;

		double do_simulation_inner();

	protected:
		double do_simulation();

	public:
		bool higher_is_better() const {
			return true;
		}

		utility_freq_sim(std::shared_ptr<coordinate_gen> entropy_src_in,
			std::shared_ptr<election_method> method_in,
			std::shared_ptr<pure_ballot_generator> generator_in,
			size_t numcands_in, size_t numvoters_in) : bounded_simulator(
					entropy_src_in) {

			reset();

			numcands = numcands_in;
			numvoters = numvoters_in;
			method = method_in;
			ballot_gen = generator_in;
		}

		// Also return generator name?
		// OUF is for "Optimal Utility Frequency".
		std::string name() const {
			return "OUF[" + method->name() + "]";
		}

		double get_minimum() const {
			return 0;
		}
		double get_maximum() const {
			return 1;
		}
};
