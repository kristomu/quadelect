#pragma once

#include "equivalences.h"

#include "../../../../linear_model/sampler/sampler.h"
#include "../../../../linear_model/constraints/constraint_polytope.h"
#include "../../../../linear_model/constraints/relative_criterion.h"

#include "../../../../ballots.h"

#include <list>
#include <vector>

struct relative_test_instance {
	election_scenario_pair before_A, before_B, after_A, after_B;
};

// A test generator is parameterized by (scenario before, scenario after,
// relative criterion). It generates before- and after-elections that when
// evaluated by an election method, tests that method's compliance with
// the relative criterion in question.

class test_generator {
	private:
		std::list<ballot_group> vector_election_to_ordering_format(
			const std::vector<double> & election, int numcands) const;
		uint64_t rng_seed;

		copeland_scenario scenario_before, scenario_after;
		billiard_sampler<constraint_polytope> sampler;
		constraint_polytope election_polytope;
		std::vector<int> before_permutation_indices,
			after_permutation_indices;

	public:
		// Samples an instance to get A, A', and then rotates to
		// other_candidate_idx to get B and B'.
		relative_test_instance sample_instance(
			size_t other_candidate_idx_before,
			size_t other_candidate_idx_after,
			const fixed_cand_equivalences before_cand_remapping,
			const fixed_cand_equivalences after_cand_remapping);

		// Returns false if it's impossible to create this particular
		// configuration.
		bool set_scenarios(copeland_scenario before,
			copeland_scenario after, int max_numvoters,
			const relative_criterion_const & rel_criterion);

		void set_rng_seed(uint64_t seed) { sampler.set_rng_seed(seed);}

		test_generator(uint64_t rng_seed_in) : sampler(rng_seed_in) {
			rng_seed = rng_seed_in;
		}
};