
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
#include "../../../../linear_model/constraints/relative_criteria/mono-add-top.h"

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

		void set_rng_seed(uint64_t seed) { sampler.set_rng_seed(seed);}

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

// Test instance generator. Each such corresponds to a single (A,A',B,B',
// candidate number for B, relative criterion) combination. These are all
// the ones we can test against.

class test_instance_generator {
	public:
		copeland_scenario before_A, after_A, before_B, after_B;
		int cand_B_idx;
		test_generator tgen;

		test_instance_generator(test_generator in) : tgen(in) {}

};

std::vector<test_instance_generator> get_all_permitted_test_generators(
	const std::vector<copeland_scenario> canonical_scenarios,
	const relative_criterion_const & relative_criterion,
	const fixed_cand_equivalences before_cand_remapping,
	const fixed_cand_equivalences after_cand_remapping,
	rng & randomizer) {

	std::vector<test_instance_generator> out;

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
				test_instance_generator to_add(cur_test);
				// Set a different seed but use the same sampler and
				// polytope as we created earlier.
				to_add.tgen.set_rng_seed(randomizer.long_rand());

				// Get the scenarios by sampling once.
				monotonicity_test_instance ti = to_add.tgen.
					sample_instance(i, before_cand_remapping, 
						after_cand_remapping);
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

// TODO: Group the identified test generators into groups that we can
// test all at once. E.g. if both test instance generators A and B have
// the same scenario 4-tuple, then we can test both at once.

// Our general design when the scenarios in the 4-tuple aren't all the
// same, is that for each algorithm and for k tests, we have a k-vector
// giving what that algorithm scored the A-election of that test. Then
// we have other such vectors for B, A', and B'; and the monotonicity
// criterion for algorithms aA, aA', aB, aB' is satisfied if
// score(aA, eA) - score(aB, eB) <= score(aA', eA') - score(aB', eB'),
// where eA, eB, eA', eB' are the elections in question.

// A group would consist of all test instance generators where scenarios
// [A, B, A', B'] are the same, or the scenario list can be permuted to
// make it the same.

// I'm not yet sure whether I'll implement that group, so I'll leave it
// for later. For now, just do the "every scenario is the same" bit.

monotonicity_test_instance get_test_instance(test_instance_generator &
	generator, const std::map<int, fixed_cand_equivalences>
	candidate_equivalences) {

	int before_numcands = generator.before_A.get_numcands(),
		after_numcands = generator.after_A.get_numcands();

	monotonicity_test_instance ti = generator.tgen.sample_instance(
			generator.cand_B_idx, 
			candidate_equivalences.find(before_numcands)->second,
			candidate_equivalences.find(after_numcands)->second);

	return ti;
}

// Returns the number of passes after the test has been evaluated on
// all currently passing algorithms.
size_t update_pass_list(size_t & global_iter_count,
	const std::vector<algo_t> & functions_to_test,
	std::vector<bool> & passes_so_far, 
	const std::vector<monotonicity_test_instance> & test_instances) {

	std::vector<std::vector<std::vector<double> > > ballot_vectors;
	size_t stride = test_instances.size(), i;

	// Handle stride (since setting up an algorithm takes time.)
	for (i = 0; i < stride; ++i) {
		ballot_vectors.push_back( {
			get_ballot_vector(test_instances[i].before_A),
			get_ballot_vector(test_instances[i].before_B),
			get_ballot_vector(test_instances[i].after_A),
			get_ballot_vector(test_instances[i].after_B)
		});
	};

	size_t passes = 0;

	// HACK! Fix later. Will not work when numcands are different.
	int numcands = test_instances[0].before_A.scenario.get_numcands();

	gen_custom_function evaluator(numcands);

	for (size_t funct_idx = 0; funct_idx < passes_so_far.size();
		++funct_idx) {

		if (!passes_so_far[funct_idx]) { continue; }

		algo_t to_test = functions_to_test[funct_idx];

		// set_algorithm is slower than force_set_algorithm because the
		// former checks the algorithm against a default instance. Thus,
		// we skip the testing if we're past the first global iteration.
		if (global_iter_count == 0) {
			assert(evaluator.set_algorithm(to_test));
		} else {
			evaluator.force_set_algorithm(to_test);
		}

		bool passed_this = true;
		for (i = 0; i < stride && passed_this; ++i) {
			double result_A = evaluator.evaluate(ballot_vectors[i][0]),
				result_B = evaluator.evaluate(ballot_vectors[i][1]),
				result_Ap = evaluator.evaluate(ballot_vectors[i][2]),
				result_Bp = evaluator.evaluate(ballot_vectors[i][3]);

			if (result_A - result_B > 0 && result_Ap - result_Bp < 0) {
				passes_so_far[funct_idx] = false;
				std::cout << "Disqualified " << to_test << " at iteration " << global_iter_count << "\n";
				passed_this = false;
			}
		}
		if (passed_this) {
			++passes;
		}
	}
	++ global_iter_count;
	return passes;
}

size_t test(int test_iterations, size_t & global_iter_count,
	const std::vector<algo_t> & functions_to_test,
	std::vector<bool> & passes_so_far,
	std::vector<test_instance_generator> & same_group_test_generators,
	const std::map<int, fixed_cand_equivalences> & candidate_equivalences) {

	size_t passes = 0;
	size_t stride = 10;

	for (int iter = 0; iter < test_iterations; ++iter) {
		std::vector<monotonicity_test_instance> instances;
		for (size_t i = 0; i < stride; ++i) {
			instances.push_back(get_test_instance(
			same_group_test_generators[iter % 
				same_group_test_generators.size()], candidate_equivalences));
		}

		passes = update_pass_list(global_iter_count,
			functions_to_test, passes_so_far, instances);
	}

	return passes;

}

int main(int argc, char ** argv) {

	int seed = 104;
	int numcands = 3;
	rng randomizer(seed);

	srand(seed);
	srandom(seed);
	srand48(seed);

	// Integrity test.

	gen_custom_function integrity_test(3);
	if (!integrity_test.test()) {
		throw std::runtime_error("Compositor: failed integrity test!");
	}

	// h4x0ring

	// Open some files to get back up to speed

	std::cout << "Reading file... " << std::endl;

	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " [file containing sifter output, 3 cands] [output file]" <<
			std::endl;
		return(-1);
	}

	std::string filename = argv[1];
	std::ifstream sifter_file(filename);

	if (!sifter_file) {
		std::cerr << "Could not open " << filename << std::endl;
	}

	std::vector<std::vector<algo_t> > prospective_functions(5);

	get_first_token_on_lines(sifter_file, prospective_functions[3]);

	std::vector<algo_t> functions_to_test = prospective_functions[3];

	sifter_file.close();

	std::cout << "... done (read " << prospective_functions[3].size() 
		<<  " functions)." << std::endl;

	// h4x0ring end

	std::string out_filename = argv[2];

	std::cout << "Initializing equivalences..." << std::endl;

	fixed_cand_equivalences three_equivalences(3);

	std::map<int, fixed_cand_equivalences> other_equivs;
	other_equivs.insert(std::pair<int, fixed_cand_equivalences>(4, fixed_cand_equivalences(4)));;
	other_equivs.insert(std::pair<int, fixed_cand_equivalences>(3, fixed_cand_equivalences(3)));;

	std::set<copeland_scenario> canonical_full = get_nonderived_scenarios(
		numcands, other_equivs.find(numcands)->second);

	std::vector<copeland_scenario> canonical_full_v;
	std::copy(canonical_full.begin(), canonical_full.end(),
		std::back_inserter(canonical_full_v));

	std::cout << "... done." << std::endl;

	copeland_scenario example_desired_A, example_desired_B;

	// I don't know why references don't work here...
	for (copeland_scenario x: canonical_full) {
		std::cout << "Smith set " << numcands << " canonical: " 
			<< x.to_string() << std::endl;
	}

	mono_raise_const mono_raise(numcands);
	mono_add_top_const mono_add_top(numcands);

	std::vector<test_instance_generator> test_generators =
		get_all_permitted_test_generators(canonical_full_v, mono_raise,
			other_equivs.find(numcands)->second, 
			other_equivs.find(numcands)->second,
			randomizer);

	std::vector<test_instance_generator> test_generators_mat =
		get_all_permitted_test_generators(canonical_full_v, mono_add_top,
			other_equivs.find(numcands)->second, 
			other_equivs.find(numcands)->second,
			randomizer);

	std::copy(test_generators_mat.begin(), test_generators_mat.end(),
		std::back_inserter(test_generators));

	for (test_instance_generator itgen : test_generators) {
		std::cout << "A: " << itgen.before_A.to_string()
			<< " A': " << itgen.after_A.to_string()
			<< " B: " << itgen.before_B.to_string()
			<< " B': " << itgen.after_B.to_string() << "\t"
			<< "cddt B = # " << itgen.cand_B_idx << "\n";
		std::cout << "Random sample:\n";

		// Known bug: random sample is not the same for all cddt Bs
		// with the same set of A, A'. It should be since all of them are
		// initialized from the same test generator which starts off with
		// the same seed.

		std::cout << "Now sampling an instance." << std::endl;

		monotonicity_test_instance ti = get_test_instance(
			itgen, other_equivs);
		
		ballot_tools().print_ranked_ballots(ti.before_A.election);

		std::cout << "----- And again!\n";

		ti = get_test_instance(itgen, other_equivs);
		
		ballot_tools().print_ranked_ballots(ti.before_A.election);

		std::cout << "-----\n\n";
	}

	// Some stuff here

	size_t global_iter_count = 0;
	std::vector<bool> passes_so_far(functions_to_test.size(), true);

	for(;;) {
		size_t x = test(20, global_iter_count, functions_to_test,
			passes_so_far, test_generators, other_equivs);

		std::ofstream out_file(out_filename);
		gen_custom_function evaluator(numcands);

		for (size_t i = 0; i < prospective_functions[3].size(); ++i) {
			if (passes_so_far[i]) {
				evaluator.force_set_algorithm(prospective_functions[3][i]);
				out_file << prospective_functions[3][i] << "\t" 
					<< evaluator.to_string() << "\n";
			}
		}

		std::cout << "passes: " << x << std::endl;
	}

	return 0;
}
