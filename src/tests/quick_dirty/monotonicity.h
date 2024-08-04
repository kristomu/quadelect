#include <memory>
#include <vector>

#include "common/ballots.h"
#include "random/random.h"
#include "generator/all.h"
#include "simulator/bernoulli.h"

// Quick and dirty monotonicity checker. Something is up with the way I've
// structured my strategy tests and other classes, and it makes it pretty
// much impossible to write new test types. I really need to refactor and
// clean it up. Until then, I have to find out if my experimental methods
// are monotone, hence this hack.

class monotone_check : public bernoulli_simulator {
	private:
		std::shared_ptr<pure_ballot_generator> ballot_gen;
		std::shared_ptr<election_method> method_tested;

		// How many attempts should we make in trying to get a
		// particular election to show nonmonotonicity?
		size_t tests_per_election;

		// This is entirely a convenience feature: the number of
		// elections to try before giving up and returning that the
		// method is monotone. The point is to make bandit stats
		// show a value different from 1 when the methods aren't
		// monotone, even if the monotonicity rate is very high.
		size_t elections_tested_at_once;

		int max_numcands, max_numvoters;
		int numcands, numvoters;

		std::vector<int> shuffle_indices;

		// Everything below this point is used for
		// printing counterexamples. Very ugly. TODO: Fix.

		// We have weak nonmonotonicity if the candidate we raised
		// is no longer in the set of winners after raising, or the
		// number of winners increased (lowering the probability that
		// this candidate could win).
		// We have strong nonmonotonicity if none of the winners
		// of the first election are winners of the second.

		// (Really, we have strong if for every winner of the first
		// election, there exists some way to raise him that makes
		// him lose. But that's not implemented here yet.)

		// I might want a different definition that basically says
		// "some compositions of this method could be nonmonotone".
		// Not sure yet though...

		ordering before_ordering, after_ordering;
		election_t ballots_before, ballots_after;
		bool weakly_nonmonotone, strongly_nonmonotone, equal_rank;
		int winner;

	public:
		monotone_check(std::shared_ptr<pure_ballot_generator> ballot_gen_in,
			std::shared_ptr<coordinate_gen> rnd_in,
			std::shared_ptr<election_method> method_in,
			int max_numcands_in,
			int max_numvoters_in) : bernoulli_simulator(rnd_in) {

			ballot_gen = ballot_gen_in;
			method_tested = method_in;
			max_numcands = max_numcands_in;
			max_numvoters = max_numvoters_in;

			shuffle_indices.resize(max_numvoters_in+1);

			weakly_nonmonotone = false;
			strongly_nonmonotone = false;

			tests_per_election = 15; // fix later
			elections_tested_at_once = 15; // ditto
		}

		double do_simulation(); // returns 1 if monotone, 0 otherwise

		std::string name() const {
			return "Monotone-(" + method_tested->name() + ")";
		}

		double get_minimum() const {
			return 0;
		}

		double get_maximum() const {
			return 1;
		}

		// More monotonicity = better
		bool higher_is_better() const {
			return true;
		}

		void print_counterexample();
};
