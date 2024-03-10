#include <memory>
#include <vector>

#include "../../ballots.h"
#include "../../random/random.h"
#include "../../generator/all.h"
#include "../../bandit/tests/tests.h"

// Quick and dirty monotonicity checker. Something is up with the way I've
// structured my strategy tests and other classes, and it makes it pretty
// much impossible to write new test types. I really need to refactor and
// clean it up. Until then, I have to find out if my experimental methods
// are monotone, hence this hack.

class monotone_check : public Test {
	private:
		std::shared_ptr<impartial> ballot_gen;
		std::shared_ptr<rng> rnd;
		std::shared_ptr<election_method> method_tested;

		// How many attempts should we make in trying to get a
		// particular election to show nonmonotonicity?
		size_t tests_per_election;

		int max_numcands, max_numvoters;
		int numcands, numvoters;

		std::vector<int> shuffle_indices;

		// Everything below this point is used for
		// printing counterexamples. Very ugly. TODO: Fix.

		ordering before_ordering, after_ordering;
		election_t ballots_before, ballots_after;
		bool old_winner_present, weakly_monotone;
		int winner;

	public:
		monotone_check(std::shared_ptr<impartial> ballot_gen_in,
			std::shared_ptr<rng> rnd_in,
			std::shared_ptr<election_method> method_in,
			int max_numcands_in, int max_numvoters_in) {

			ballot_gen = ballot_gen_in;
			rnd = rnd_in;
			method_tested = method_in;
			max_numcands = max_numcands_in;
			max_numvoters = max_numvoters_in;

			shuffle_indices.resize(max_numvoters_in+1);

			old_winner_present = true;
			weakly_monotone = true;

			tests_per_election = 8192; // fix later
		}

		double perform_test(); // returns 1 if monotone, 0 otherwise

		std::string name() const {
			return "Monotone-(" + method_tested->name() + ")";
		}

		double get_minimum() const {
			return 0;
		}

		double get_maximum() const {
			return 1;
		}

		void print_counterexample();
};