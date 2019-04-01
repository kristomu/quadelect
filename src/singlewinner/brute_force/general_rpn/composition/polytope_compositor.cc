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

#include "../../../../linear_model/constraints/relative_criterion_producer.h"
#include "../../../../linear_model/constraints/relative_criteria/mono-raise.h"
#include "../../../../linear_model/constraints/relative_criteria/mono-add-top.h"

#include "../../../../config/general_rpn.h"

// Beware of ugly hacks. The triple nested loop is the way it is so as to
// minimize the amount of seeking that needs to be done in the memory-
// mapped file. It's pretty hideous.
void update_results(
	const std::vector<std::vector<algo_t> > & functions_to_test,
	test_results & results_so_far,
	const std::vector<vector_test_instance> & elections) {

	// HACK! Fix later.
	std::vector<int> numcands_by_type(4);
	numcands_by_type[TYPE_A] = elections[0].ti.before_A.scenario.
		get_numcands();
	numcands_by_type[TYPE_B] = elections[0].ti.before_B.scenario.
		get_numcands();
	numcands_by_type[TYPE_A_PRIME] = elections[0].ti.after_A.scenario.
		get_numcands();
	numcands_by_type[TYPE_B_PRIME] = elections[0].ti.after_B.scenario.
		get_numcands();

	std::cout << elections[0].ti.before_A.scenario.to_string() << ", " <<
		elections[0].ti.before_B.scenario.to_string() << std::endl;
	std::cout << elections[0].ti.before_A.scenario.get_numcands() << ", " <<
		elections[0].ti.before_B.scenario.get_numcands() << std::endl;

	std::vector<gen_custom_function> evaluator = {
		gen_custom_function(0), gen_custom_function(1),
		gen_custom_function(2), gen_custom_function(3),
		gen_custom_function(4)};

	// Linear count for the progress report
	// TODO, make this actually work.
	double max_count = NUM_REL_ELECTION_TYPES * functions_to_test[3].size();
	size_t cur_count = 0;

	for (int type = TYPE_A; type <= TYPE_B_PRIME; ++type) {

		int numcands = numcands_by_type[type];

		for (size_t funct_idx = 0; funct_idx <
			functions_to_test[numcands].size(); ++funct_idx) {

			algo_t to_test = functions_to_test[numcands][funct_idx];

			// Forcing an algorithm skips various sanity checks and is
			// faster. So only do those checks the first time we set the
			// evaluator to a particular algorithm.
			// TODO, fix this: only needs to be first for a particular
			// number of candidates.
			if (type == TYPE_A) {
				assert(evaluator[numcands].set_algorithm(to_test));
			} else {
				evaluator[numcands].force_set_algorithm(to_test);
			}

			std::cout << "Generating results for " << to_test
				<< " (numcands: " << numcands << ", rel: "
				<< cur_count++/max_count << ")\n";

			for (size_t test_number = 0; test_number < elections.size();
				++test_number) {

				double result = evaluator[numcands].evaluate(elections[test_number].ballot_vectors[type]);
				results_so_far.set_result(funct_idx, test_number,
					(test_election)type, result);
			}
		}
	}
}

void test(size_t desired_samples,
	const std::vector<std::vector<algo_t> > & functions_to_test,
	test_results & results_so_far, test_generator_group & group,
	const std::map<int, fixed_cand_equivalences> & candidate_equivalences) {

	update_results(functions_to_test, results_so_far, group.sample(
		desired_samples, candidate_equivalences));
}

// And then more testing? No. More features, because who has time for
// testing and cleanup? :p

int main(int argc, char ** argv) {

	size_t min_numcands = 3, max_numcands = 4;
	rng randomizer(RNG_ENTROPY);

	// Integrity test.

	gen_custom_function integrity_test(3);
	if (!integrity_test.test()) {
		throw std::runtime_error("Compositor: failed integrity test!");
	}

	// h4x0ring

	// Open some files to get back up to speed

	std::cout << "Reading files..." << std::endl;

	if (argc < 2) {
		std::cerr << "Usage: " << argv[0]
			<< " [general_rpn_tools config file] "
			<< std::endl;
		return(-1);
	}

	// Read configuration file.
	g_rpn_config settings;
	settings.load_from_file(argv[1]);

	std::vector<std::string> filename_cand_inputs = settings.source_files;

	size_t i;

	std::vector<std::vector<algo_t> > functions_to_test(max_numcands+1);

	for (i = min_numcands; i <= max_numcands; ++i) {
		std::ifstream sifter_file(filename_cand_inputs[i]);
		std::cout << filename_cand_inputs[i] << std::endl;

		if (!sifter_file) {
			std::cerr << "Could not open " << filename_cand_inputs[i]
				<< std::endl;
		}

		get_first_token_on_lines(sifter_file, functions_to_test[i]);

		sifter_file.close();

		std::cout << "... done " << i << " candidates. (read "
			<< functions_to_test[i].size() <<  " functions)."
			<< std::endl;
	}

	// h4x0ring end

	std::string out_prefix = argv[3];

	std::cout << "Initializing equivalences..." << std::endl;

	std::map<int, fixed_cand_equivalences> cand_equivs =
		get_cand_equivalences(max_numcands);

	std::vector<copeland_scenario> canonical_full_v;

	for (i = min_numcands; i <= max_numcands; ++i) {

		std::set<copeland_scenario> canonical_full = get_nonderived_scenarios(
			i, cand_equivs.find(i)->second);

		std::copy(canonical_full.begin(), canonical_full.end(),
			std::back_inserter(canonical_full_v));
	}

	std::cout << "... done." << std::endl;

	copeland_scenario example_desired_A, example_desired_B;

	// I don't know why references don't work here...
	for (const copeland_scenario x: canonical_full_v) {
		std::cout << "Smith set " << x.get_numcands() << " canonical: "
			<< x.to_string() << std::endl;
	}

	// Add some relative constraints. (Kinda ugly, but what can you do.)
	// TODO min_candidates, max
	std::vector<std::unique_ptr<relative_criterion_const> >
		relative_constraints = relative_criterion_producer().get_all(
			min_numcands, max_numcands, true);

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
					canonical_full_v, *constraint, cand_equivs,
					randomizer);

			for (test_instance_generator itgen : test_generators) {
				grps.insert(itgen);
			}
		}
	}

	std::cout << "Number of groups: " << grps.groups.size() << "\n";

	// Because the output file is linear, we need to allocate space for
	// the same number of functions no matter what the number of
	// candidates is. So allocate enough to always have room, i.e.
	// as many as the max i: functions_to_test[i].size();
	size_t max_num_functions = 0;
	for (i = min_numcands; i <= max_numcands; ++i) {
		max_num_functions = std::max(max_num_functions,
			functions_to_test[i].size());
	}

	for (size_t group_idx = 0; group_idx < grps.groups.size(); ++group_idx) {

		std::string fn_prefix = settings.test_storage_prefix + itos(i) + ".dat";

		test_generator_group grp = grps.groups[group_idx];

		grp.print_members();

		// Some stuff here
		int num_tests = settings.num_tests;

		test_results results(num_tests, max_num_functions);

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