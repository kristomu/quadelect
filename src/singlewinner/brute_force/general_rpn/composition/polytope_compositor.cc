#include "groups/test_generator_groups.h"
#include "groups/test_generator_group.h"
#include "logistics/vector_test_instance.h"
#include "test_results.h"
#include "test_instance_gen.h"
#include "test_generator.h"
#include "vector_ballot.h"
#include "equivalences.h"
#include "eligibility.h"
#include "../isda.cc"

#include <time.h>

#include <stdexcept>
#include <memory>

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
			test_generator cur_test(randomizer.long_rand());

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
				to_add.tgen.set_rng_seed(randomizer.long_rand());

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

// Beware of ugly hacks. The triple nested loop is the way it is so as to
// minimize the amount of seeking that needs to be done in the memory-
// mapped file. It's pretty hideous.
void update_results(const std::vector<algo_t> & functions_to_test,
	test_results & results_so_far,
	const std::vector<vector_test_instance> & elections) {

	// HACK! Fix later. Will not work when numcands are different
	// (e.g. cloning)
	int numcands = elections[0].ti.before_A.scenario.get_numcands();

	gen_custom_function evaluator(numcands);

	// Linear count for the progress report
	double max_count = NUM_REL_ELECTION_TYPES * functions_to_test.size();
	size_t cur_count = 0;

	for (int type = TYPE_A; type <= TYPE_B_PRIME; ++type) {

		for (size_t funct_idx = 0; funct_idx < functions_to_test.size();
			++funct_idx) {

			algo_t to_test = functions_to_test[funct_idx];

			// Forcing an algorithm skips various sanity checks and is
			// faster. So only do those checks the first time we set the
			// evaluator to a particular algorithm.
			if (type == TYPE_A) {
				assert(evaluator.set_algorithm(to_test));
			} else {
				evaluator.force_set_algorithm(to_test);
			}

			std::cout << "Generating results for " << to_test << "(rel: " <<
				cur_count++/max_count << ")\n";

			for (size_t test_number = 0; test_number < elections.size(); 
				++test_number) {
		
				double result = evaluator.evaluate(elections[test_number].ballot_vectors[type]);
				results_so_far.set_result(funct_idx, test_number,
					(test_election)type, result);
			}
		}
	}
}

void test(size_t desired_samples, 
	const std::vector<algo_t> & functions_to_test,
	test_results & results_so_far, test_generator_group & group,
	const std::map<int, fixed_cand_equivalences> & candidate_equivalences) {

	update_results(functions_to_test, results_so_far, group.sample(
		desired_samples, candidate_equivalences));
}

// TODO: Find out why 0x5140-0x52c0 is empty. 
// (There's a method that returns 0 all the time, that's why.)

// Then next: refactor, and test_generator_groups class that
// does "if suitable tgg exists, dump next test_instance into it,
// otherwise append a new tgg to the end". Perhaps also name() for
// constraints?
// And then more testing?

int main(int argc, char ** argv) {

	int numcands = 4;
	rng randomizer(1);
	//rng randomizer(RNG_ENTROPY);

	// Integrity test.

	gen_custom_function integrity_test(3);
	if (!integrity_test.test()) {
		throw std::runtime_error("Compositor: failed integrity test!");
	}

	// h4x0ring

	// Open some files to get back up to speed

	std::cout << "Reading file... " << std::endl;

	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " [file containing sifter output, " << numcands << " cands] [output file]" <<
			std::endl;
		return(-1);
	}

	std::string filename = argv[1];
	std::ifstream sifter_file(filename);

	if (!sifter_file) {
		std::cerr << "Could not open " << filename << std::endl;
	}

	std::vector<std::vector<algo_t> > prospective_functions(5);

	get_first_token_on_lines(sifter_file, prospective_functions[numcands]);

	std::vector<algo_t> functions_to_test = prospective_functions[numcands];

	sifter_file.close();

	std::cout << "... done (read " << prospective_functions[numcands].size()
		<<  " functions)." << std::endl;

	// h4x0ring end

	std::string out_filename = argv[2];

	std::cout << "Initializing equivalences..." << std::endl;

	std::map<int, fixed_cand_equivalences> cand_equivs = 
		get_cand_equivalences(4);
	
	std::set<copeland_scenario> canonical_full = get_nonderived_scenarios(
		numcands, cand_equivs.find(numcands)->second);

	std::vector<copeland_scenario> canonical_full_v;
	std::copy(canonical_full.begin(), canonical_full.end(),
		std::back_inserter(canonical_full_v));

	std::cout << "... done." << std::endl;

	copeland_scenario example_desired_A, example_desired_B;

	// I don't know why references don't work here...
	for (const copeland_scenario x: canonical_full) {
		std::cout << "Smith set " << numcands << " canonical: "
			<< x.to_string() << std::endl;
	}

	std::vector<std::unique_ptr<relative_criterion_const> > 
		relative_constraints;

	// Add some relative constraints. (Kinda ugly, but what can you do.)
	relative_constraints.push_back(
		std::make_unique<mono_raise_const>(numcands));
	relative_constraints.push_back(
		std::make_unique<mono_add_top_const>(numcands));

	// Perhaps make the constrain generators return the before and after
	// number of candidates? Then we can just pass in cand_equivs and
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

	// Something kinda is wrong with get_all_permitted_test_generators;
	// needs more design attention. Should it return a group, or iterate
	// over all groups, or something entirely different? On the one hand,
	// std::vector<test_instance_generator> is sort of a mask for a
	// test_generator_group. On the other... it's not constrained the way
	// a test_generator_group is.

	// More problems...
	std::vector<double> numvoters_options = {1, 100, 10000};

	test_generator_groups grps;

	for (double numvoters: numvoters_options) {
		for (auto & constraint : relative_constraints) {
			std::vector<test_instance_generator> test_generators =
				get_all_permitted_test_generators(numvoters,
					canonical_full_v, *constraint,
					cand_equivs.find(numcands)->second,
					cand_equivs.find(numcands)->second,
					randomizer);

			for (test_instance_generator itgen : test_generators) {
				grps.insert(itgen);
			}
		}
	}

	std::cout << "Number of groups: " << grps.groups.size() << "\n";

	for (size_t i = 0; i < grps.groups.size(); ++i) {

		std::string fn_prefix = "algo_testing/" + itos(i) + "_" + out_filename;

		test_generator_group grp = grps.groups[i];

		grp.print_members();

		// Some stuff here
		int num_tests = 100;
		test_results results(num_tests, functions_to_test.size());

		std::cout << "Space required: " << results.get_bytes_required() << "\n";
		results.allocate_space(fn_prefix);

		// Write metadata
		ofstream out_meta(fn_prefix + ".meta");

		out_meta << fn_prefix << " ";
		grp.print_scenarios(out_meta);
		out_meta << " " << results.num_tests << " ";
		std::copy(results.num_methods.begin(), results.num_methods.end(),
			ostream_iterator<int>(out_meta, " "));
		out_meta << "\n";

		out_meta.close();

		test(num_tests, functions_to_test, results, grp, cand_equivs);
	}

	return 0;
}