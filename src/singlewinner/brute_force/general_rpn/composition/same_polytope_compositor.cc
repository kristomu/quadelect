
#include "test_tuple_generator.h"
#include "test_generator.h"
#include "vector_ballot.h"
#include "equivalences.h"
#include "eligibility.h"
#include "../isda.cc"

#include <time.h>

#include <stdexcept>

#include <eigen3/Eigen/Dense>

#include "../../../../tools/ballot_tools.h"

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

std::vector<test_instance_generator> get_all_permitted_test_generators(
	double max_numvoters,
	const std::vector<copeland_scenario> canonical_scenarios,
	const relative_criterion_const & relative_criterion,
	const fixed_cand_equivalences before_cand_remapping,
	const fixed_cand_equivalences after_cand_remapping,
	rng & randomizer) {

	std::vector<test_instance_generator> out;

	size_t numcands = canonical_scenarios[0].get_numcands();

	for (copeland_scenario x: canonical_scenarios) {
		for (copeland_scenario y: canonical_scenarios) {
			test_generator cur_test(randomizer.next_long());

			std::cout << "Combination " << x.to_string() << ", "
				<< y.to_string() << ":";
			if (!cur_test.set_scenarios(x, y, max_numvoters,
					relative_criterion)) {
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
				to_add.tgen.set_rng_seed(randomizer.next_long());

				// Get the scenarios by sampling once.
				relative_test_instance ti = to_add.tgen.
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

relative_test_instance get_test_instance(test_instance_generator &
	generator, const std::map<int, fixed_cand_equivalences>
	candidate_equivalences) {

	int before_numcands = generator.before_A.get_numcands(),
		after_numcands = generator.after_A.get_numcands();

	relative_test_instance ti = generator.tgen.sample_instance(
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
	const std::vector<relative_test_instance> & test_instances) {

	std::vector<std::vector<std::vector<double> > > ballot_vectors;
	size_t stride = test_instances.size(), i;

	// Handle stride (since setting up an algorithm takes time.)
	for (i = 0; i < stride; ++i) {
		ballot_vectors.push_back({
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

		if (!passes_so_far[funct_idx]) {
			continue;
		}

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

			bool is_nan = isnan(result_A) || isnan(result_B) ||
				isnan(result_Ap) || isnan(result_Bp);

			if (is_nan || (result_A - result_B > 0 &&
					result_Ap - result_Bp < 0)) {

				passes_so_far[funct_idx] = false;
				std::cout << "Disqualified " << to_test << " at iteration " <<
					global_iter_count << "\n";
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
	size_t linear_count = 0;

	for (int iter = 0; iter < test_iterations; ++iter) {
		std::vector<relative_test_instance> instances;
		for (size_t i = 0; i < stride; ++i) {
			instances.push_back(get_test_instance(
					same_group_test_generators[linear_count++ %
									   same_group_test_generators.size()], candidate_equivalences));
		}

		passes = update_pass_list(global_iter_count,
				functions_to_test, passes_so_far, instances);
	}

	return passes;

}

int main(int argc, char ** argv) {

	int numcands = 3;
	rng randomizer(RNG_ENTROPY);

	// Integrity test.

	gen_custom_function integrity_test(3);
	if (!integrity_test.test()) {
		throw std::runtime_error("Compositor: failed integrity test!");
	}

	// h4x0ring

	// Open some files to get back up to speed

	std::cout << "Reading file... " << std::endl;

	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] <<
			" [file containing sifter output, 3 cands] [output file]" <<
			std::endl;
		return (-1);
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
	other_equivs.insert(std::pair<int, fixed_cand_equivalences>(4,
			fixed_cand_equivalences(4)));;
	other_equivs.insert(std::pair<int, fixed_cand_equivalences>(3,
			fixed_cand_equivalences(3)));;

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

	std::vector<test_instance_generator> test_generators;

	// Perhaps make the constrain generators return the before and after
	// number of candidates? Then we can just pass in other_equivs and
	// not have to care about numcands. However, I think I should make
	// 4,4 first (to establish the recording format), and then go to
	// cloning, and at that point start on fixing all these dependencies
	// on numcands.

	// Having many different scales helps eliminate methods more quickly.
	// Strictly speaking, the polytope with the largest number of max voters
	// covers the other two given enough time, but this approach gives more
	// of a spread right away.
	// Ultimately, this is a symptom that our mixing times are too long,
	// and the proper remedy is to increase the max step size for the
	// billiard sampler (or engage in thinning, which amounts to the same
	// thing...)
	std::vector<double> numvoters_options = {1, 100, 10000};

	for (double numvoters: numvoters_options) {
		std::vector<test_instance_generator> test_generators_mr =
			get_all_permitted_test_generators(numvoters,
				canonical_full_v, mono_raise,
				other_equivs.find(numcands)->second,
				other_equivs.find(numcands)->second,
				randomizer);

		std::copy(test_generators_mr.begin(), test_generators_mr.end(),
			std::back_inserter(test_generators));

		std::vector<test_instance_generator> test_generators_mat =
			get_all_permitted_test_generators(numvoters,
				canonical_full_v, mono_add_top,
				other_equivs.find(numcands)->second,
				other_equivs.find(numcands)->second,
				randomizer);

		std::copy(test_generators_mat.begin(), test_generators_mat.end(),
			std::back_inserter(test_generators));
	}

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

		relative_test_instance ti = get_test_instance(
				itgen, other_equivs);

		ballot_tools().print_ranked_ballots(ti.before_A.election);

		std::cout << "----- And again!\n";

		ti = get_test_instance(itgen, other_equivs);

		ballot_tools().print_ranked_ballots(ti.before_A.election);

		std::cout << "-----" << std::endl << std::endl;
	}

	// Some stuff here

	size_t global_iter_count = 0;
	std::vector<bool> passes_so_far(functions_to_test.size(), true);

	for (;;) {
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
