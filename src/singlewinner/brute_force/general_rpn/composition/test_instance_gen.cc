#include "test_instance_gen.h"

// Get a test instance from a generator and candidate equivalences.
relative_test_instance test_instance_generator::get_test_instance(
	const std::map<int, fixed_cand_equivalences> candidate_equivalences) {

	int before_numcands = before_A.get_numcands(),
		after_numcands = after_A.get_numcands();

	relative_test_instance ti = tgen.sample_instance(
		cand_B_idx_before, cand_B_idx_after,
		candidate_equivalences.find(before_numcands)->second,
		candidate_equivalences.find(after_numcands)->second);

	return ti;
}

// Where should I put it?
std::vector<test_instance_generator> get_all_permitted_test_generators(
	double max_numvoters,
	const std::vector<copeland_scenario> canonical_scenarios,
	const relative_criterion_const & relative_criterion,
	const std::map<int, fixed_cand_equivalences> cand_equivs,
	rng & randomizer) {

	std::vector<test_instance_generator> out;

	// There are two steps to this function. First, we create all the test
	// generators based on what the relative criterion will permit. During
	// creation, the test generator might invoke ISDA wrappers or similar,
	// so we don't know what the scenarios will actually *be*.

	// Then, we create every allowed test instance generator based on those
	// test generators.

	//

	// Must get before and after numcands for the relative criterion,
	// then go through canonical scenarios for those candidate numbers.
	// The inner loop should go through every (before cand, after cand)
	// pair consistent with after_as_before for that criterion.

	size_t numcands_before = relative_criterion.get_numcands_before(),
		numcands_after = relative_criterion.get_numcands_after();

	std::vector<test_generator> generators;

	for (copeland_scenario x: canonical_scenarios) {
		if (x.get_numcands() != numcands_before) { continue; }

		for (copeland_scenario y: canonical_scenarios) {
			if (y.get_numcands() != numcands_after) { continue; }
			test_generator cur_test(randomizer.long_rand());

			std::cout << "Combination " << relative_criterion.name()
				<< ", " << x.to_string() << ", " << y.to_string() << ": ";
			if (!cur_test.set_scenarios(x, y, max_numvoters,
				relative_criterion)) {
				std::cout << "not permitted\n";
				continue;
			}

			std::cout << "permitted\n";
			generators.push_back(cur_test);
		}
	}

	for (test_generator & gen: generators) {

		// Very ugly hack. We'll fix this later by making it possible to
		// enumerate the permitted candidate pairs. (??)
		size_t gen_numcands_before = gen.numcands_before,
				gen_numcands_after = gen.numcands_after;

		fixed_cand_equivalences before_cand_remapping = cand_equivs.find(
			gen_numcands_before)->second, after_cand_remapping =
			cand_equivs.find(gen_numcands_after)->second;

		for (size_t before_cand_idx = 1;
			before_cand_idx < gen_numcands_before; ++before_cand_idx) {

			for (size_t after_cand_idx = 0;
				after_cand_idx < gen_numcands_after; ++after_cand_idx) {

				// Only continue with the permitted pairs.
				if (!gen.permitted_b_pair(before_cand_idx,
					after_cand_idx, relative_criterion)) {
					continue;
				}

				test_instance_generator to_add(gen);
				// Set a different seed but use the same sampler and
				// polytope as we created earlier.
				to_add.tgen.set_rng_seed(randomizer.long_rand());

				// Set what kind of test this group should perform,
				// depending on what the relative criterion says.

				to_add.no_help = relative_criterion.no_help();
				to_add.no_harm = relative_criterion.no_harm();

				// Get the scenarios by sampling once.
				relative_test_instance ti = to_add.tgen.sample_instance(
					before_cand_idx, after_cand_idx,
					before_cand_remapping, after_cand_remapping);

				to_add.before_A = ti.before_A.scenario;
				to_add.before_B = ti.before_B.scenario;
				to_add.after_A = ti.after_A.scenario;
				to_add.after_B = ti.after_B.scenario;

				to_add.cand_B_idx_before = before_cand_idx;
				to_add.cand_B_idx_after = after_cand_idx;
				out.push_back(to_add);
			}
		}
	}
	return out;
}
