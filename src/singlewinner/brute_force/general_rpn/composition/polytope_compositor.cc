
#include "equivalences.cc"
#include "eligibility.h"
#include "../isda.cc"

#include <time.h>

#include <stdexcept>

#include <eigen3/Eigen/Dense>

#include "../../../../linear_model/polytope/equality_polytope.h"
#include "../../../../linear_model/polytope/simplex.h"
#include "../../../../linear_model/sampler/sampler.h"

#include "../../../../linear_model/lin_relation/lin_relation.h"
#include "../../../../linear_model/lin_relation/constraint.h"
#include "../../../../linear_model/lin_relation/constraint_set.h"

#include "../../../../linear_model/constraints/constraint_polytope.h"
#include "../../../../linear_model/constraints/constraint_tools.h"
#include "../../../../linear_model/constraints/numvoters.h"
#include "../../../../linear_model/constraints/pairwise.h"
#include "../../../../linear_model/constraints/general.h"

#include "../../../../linear_model/constraints/relative_criteria/mono-raise.h"

// A test instance is of this form: (scenario before, scenario after,
// relative criterion).

struct monotonicity_test_instance {
	election_scenario_pair before_A, before_B, after_A, after_B;
};


class test_generator {
	private:
		std::list<ballot_group> vector_election_to_ordering_format(
			const std::vector<double> & election, int numcands) const;
		uint64_t rng_seed;

	public:
		copeland_scenario scenario_before, scenario_after;
		billiard_sampler<constraint_polytope> sampler;
		constraint_polytope election_polytope;
		std::vector<int> before_permutation_indices,
			after_permutation_indices;

		// Samples an instance to get A, A', and then rotates to
		// other_candidate_idx to get B and B'.
		monotonicity_test_instance sample_instance(
			size_t other_candidate_idx,
			const fixed_cand_equivalences before_cand_remapping,
			const fixed_cand_equivalences after_cand_remapping);

		// Returns false if it's impossible to create this particular
		// configuration.
		bool set_scenarios(copeland_scenario before,
			copeland_scenario after, int max_numvoters,
			const relative_criterion_const & rel_criterion);

		test_generator(uint64_t rng_seed_in) : sampler(rng_seed_in) {
			rng_seed = rng_seed_in;
		}
};

std::list<ballot_group> test_generator::vector_election_to_ordering_format(
	const std::vector<double> & election, int numcands) const {

	std::vector<int> cur_cand_ordering(numcands);
	std::iota(cur_cand_ordering.begin(), cur_cand_ordering.end(), 0);

	std::list<ballot_group> out_election;
	int perm_number = 0;

	do {
		ballot_group ballot;
		ballot.complete = true;
		ballot.rated = false;
		for (int i = 0; i < numcands; ++i) {
			ballot.contents.insert(candscore(cur_cand_ordering[i],
				numcands-i));
		}
		ballot.weight = election[perm_number++];
		out_election.push_back(ballot);
	} while (std::next_permutation(cur_cand_ordering.begin(),
		cur_cand_ordering.end()));

	return out_election;
}

bool test_generator::set_scenarios(copeland_scenario before,
			copeland_scenario after, int max_numvoters,
			const relative_criterion_const & rel_criterion) {

	// Construct a proper polytope for the desired scenarios and relative
	// criterion. If we get an exception, the parameters produce an
	// infeasible setup, e.g. mono-raise from a scenario with A>B to
	// a scenario with B>A is impossible since A can't lose by being
	// raised.

	constraint_set all_constraints;

	// Set constraints on the number of voters
	voter_constraints voters("v");
	all_constraints.add(voters.max_numvoters_definition(before.get_numcands(),
		"before"));
	all_constraints.add(voters.max_numvoters_definition(after.get_numcands(),
		"after"));
	all_constraints.add(voters.max_numvoters_upper_bound(max_numvoters));

	// Set scenario constraints.
	pairwise_constraints pairwise;
	all_constraints.add(pairwise.beat_constraints(before.get_short_form(),
		"before", before.get_numcands()));
	all_constraints.add(pairwise.beat_constraints(after.get_short_form(),
		"after", after.get_numcands()));

	// Add relative constraints linking the before- and after-election.
	all_constraints.add(rel_criterion.relative_constraints("before",
		"after"));

	// Set the margin of pairwise victory and add a nonnegativity
	// constraint.
	all_constraints.set_fixed_param("min_victory_margin", 1);
	all_constraints.add(general_const::all_nonnegative(all_constraints));

	// Set up the polytope and sampler. Return true if we can do so,
	// otherwise.

	try {
		election_polytope = constraint_polytope(all_constraints);
		sampler = billiard_sampler<constraint_polytope>(election_polytope,
			true, true, rng_seed);
	} catch (const std::runtime_error & e) {
		return false;
	}

	// Set the lookup permutations we need in order to return ballots.

	before_permutation_indices = election_polytope.
		get_all_permutations_indices(before.get_numcands(), "before");
	after_permutation_indices = election_polytope.
		get_all_permutations_indices(before.get_numcands(), "after");

	scenario_before = before;
	scenario_after = after;

	return true;
}

monotonicity_test_instance test_generator::sample_instance(
	size_t other_candidate_idx,
	const fixed_cand_equivalences before_cand_remapping,
	const fixed_cand_equivalences after_cand_remapping) {

	Eigen::VectorXd point = sampler.billiard_walk();

	monotonicity_test_instance out;
	size_t i;

	// This is kinda unwieldy: we go from vector<double> to
	// list<ballot_group> only so we can rotate and determine scenarios;
	// then we go right back because gen_custom_function takes vector<double>.
	// TODO at some later time: skip the middle-man. Probably will involve
	// making a vector<double> election class.

	std::vector<double> vector_election(before_permutation_indices.size());
	for (i = 0; i < before_permutation_indices.size(); ++i) {
		vector_election[i] = point(before_permutation_indices[i]);
	}

	out.before_A.election = vector_election_to_ordering_format(
		vector_election, scenario_before.get_numcands());
	out.before_A.from_perspective_of = 0;
	out.before_A.scenario = scenario_before;

	vector_election.resize(after_permutation_indices.size());
	for (i = 0; i < after_permutation_indices.size(); ++i) {
		vector_election[i] = point(after_permutation_indices[i]);
	}

	out.after_A.election = vector_election_to_ordering_format(
		vector_election, scenario_after.get_numcands());
	out.after_A.from_perspective_of = 0;
	out.after_A.scenario = scenario_after;

	// ??? How do we do this for e.g. cloning? Or through ISDA?
	// Remember to keep that in mind!


	// TODO: Check that these are canonical...
	isomorphism cand_remapping = before_cand_remapping.
		get_candidate_remapping(out.before_A.scenario, other_candidate_idx);

	out.before_B.election = permute_election_candidates(out.before_A.election,
		cand_remapping.cand_permutations[0]);
	out.before_B.from_perspective_of = other_candidate_idx;
	out.before_B.scenario = cand_remapping.to_scenario;

	cand_remapping = before_cand_remapping.get_candidate_remapping(
		out.after_A.scenario, other_candidate_idx);

	out.after_B.election = permute_election_candidates(out.after_A.election,
		cand_remapping.cand_permutations[0]);
	out.after_B.from_perspective_of = other_candidate_idx;
	out.after_B.scenario = cand_remapping.to_scenario;

	return out;
}

// Identified tests. Each such corresponds to a single (A,A',B,B',
// candidate number for B, relative criterion) combination. These are all
// the ones we can test against.

class identified_test_generator {
	public:
		copeland_scenario before_A, after_A, before_B, after_B;
		int cand_B_idx;
		test_generator tgen;

		identified_test_generator(test_generator in) : tgen(in) {}

};

std::vector<identified_test_generator> get_all_permitted_test_generators(
	const std::vector<copeland_scenario> canonical_scenarios,
	const relative_criterion_const & relative_criterion,
	const fixed_cand_equivalences before_cand_remapping,
	const fixed_cand_equivalences after_cand_remapping,
	rng & randomizer) {

	std::vector<identified_test_generator> out;

	size_t numcands = canonical_scenarios[0].get_numcands();

	for (copeland_scenario x: canonical_scenarios) {
		for (copeland_scenario y: canonical_scenarios) {
			test_generator cur_test(randomizer.long_rand());

			std::cout << "Combination " << x.to_string() << ", "
				<< y.to_string() << ":";
			if (!cur_test.set_scenarios(x, y, 100, relative_criterion)) {
				std::cout << "not permitted\n";
				continue;
			}

			std::cout << "permitted\n";

			// Warning: take note of that numcands might vary for
			// e.g. cloning or ISDA.
			for (size_t i = 1; i < numcands; ++i) {
				monotonicity_test_instance ti = cur_test.sample_instance(i,
					before_cand_remapping, after_cand_remapping);
				// TODO: permit changing the rng seed here
				identified_test_generator to_add(cur_test);
				to_add.before_A = ti.before_A.scenario;
				to_add.before_B = ti.before_B.scenario;
				to_add.after_A = ti.after_A.scenario;
				to_add.after_B = ti.after_B.scenario;
				to_add.cand_B_idx = i;
				out.push_back(to_add);
			}
		}
	}
	return out;
}


int main(int argc, char ** argv) {

	int seed = 102;
	int numcands = 4;
	rng randomizer(seed);

	srand(seed);
	srandom(seed);
	srand48(seed);

	// Integrity test.

	gen_custom_function integrity_test(3);
	if (!integrity_test.test()) {
		throw std::runtime_error("Compositor: failed integrity test!");
	}

	std::cout << "Initializing equivalences..." << std::endl;

	fixed_cand_equivalences three_equivalences(3);

	std::map<int, fixed_cand_equivalences> other_equivs;
	other_equivs.insert(std::pair<int, fixed_cand_equivalences>(4, fixed_cand_equivalences(4)));;
	other_equivs.insert(std::pair<int, fixed_cand_equivalences>(3, fixed_cand_equivalences(3)));;

	std::set<copeland_scenario> canonical_full = get_nonderived_scenarios(4,
		other_equivs.find(4)->second);

	std::vector<copeland_scenario> canonical_full_v;
	std::copy(canonical_full.begin(), canonical_full.end(),
		std::back_inserter(canonical_full_v));

	std::cout << "... done." << std::endl;

	copeland_scenario example_desired_A, example_desired_B;

	// I don't know why references don't work here...
	for (copeland_scenario x: canonical_full) {
		std::cout << "Smith set 4 canonical: " << x.to_string() << std::endl;
	}

	mono_raise_const mono_raise(numcands);

	std::vector<identified_test_generator> test_generators =
		get_all_permitted_test_generators(canonical_full_v, mono_raise,
			other_equivs.find(4)->second, other_equivs.find(4)->second,
			randomizer);

	for (identified_test_generator itgen : test_generators) {
		std::cout << "A: " << itgen.before_A.to_string()
			<< " B: " << itgen.before_B.to_string()
			<< " A': " << itgen.after_A.to_string()
			<< " B': " << itgen.after_B.to_string() << "\t"
			<< "cddt B = # " << itgen.cand_B_idx << "\n";
		std::cout << "Random sample:\n";

		// Known bug: random sample is not the same for all cddt Bs
		// with the same set of A, A'. It should be since all of them are
		// initialized from the same test generator which starts off with
		// the same seed.

		monotonicity_test_instance ti = itgen.tgen.sample_instance(
			itgen.cand_B_idx, other_equivs.find(4)->second,
			other_equivs.find(4)->second);

		ballot_tools().print_ranked_ballots(ti.before_A.election);

		std::cout << "-----\n\n";
	}

	return 0;
}
