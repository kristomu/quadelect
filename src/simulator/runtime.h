#pragma once

#include "bounded.h"

#include "common/ballots.h"

#include "generator/ballotgen.h"
#include "singlewinner/method.h"
#include "random/random.h"

// This is a simulator that returns the runtime of a particular
// method, so we can (e.g.) find the fastest methods on average by
// bandit search. Because the distribution of runtimes is unknown,
// it needs to be bounded above to produce a sub-Gaussian distribution
// as required by the bandit search.

// Anything taking more time than max_num_seconds has its runtime
// truncated to that maximum.

// TODO: Use a scaling factor on the returned values so that they're
// scaled to Kemeny=1, making it easier for the bandit search program
// to show their relative performance (and to create combinations of
// simulations) without everything just showing as 0.

// For now, display values are in microseconds.

class runtime_sim : public bounded_simulator {
	private:
		size_t numcands, numvoters;
		double max_num_seconds;

		std::shared_ptr<election_method> method;
		std::shared_ptr<pure_ballot_generator> ballot_gen;

	protected:
		double do_simulation();

	public:
		bool higher_is_better() const {
			return false;
		}

		runtime_sim(double max_seconds_in,
			std::shared_ptr<coordinate_gen> entropy_src_in,
			std::shared_ptr<election_method> method_in,
			std::shared_ptr<pure_ballot_generator> ballot_gen_in,
			size_t numcands_in,
			size_t numvoters_in) : bounded_simulator(entropy_src_in) {

			numcands = numcands_in;
			numvoters = numvoters_in;
			method = method_in;
			ballot_gen = ballot_gen_in;
			max_num_seconds = max_seconds_in;
		}

		// Convenience feature to make the differences in runtime
		// actually visible.
		double get_exact_value(
			double linearized, bool total) const {

			return 1e6 * linearized;
		}

		// Also return generator name?
		std::string name() const {
			return "Runtime[" + method->name() + "]";
		}

		double get_minimum() const {
			return 0;
		}

		double get_maximum() const {
			return max_num_seconds;
		}
};
