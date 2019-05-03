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

// Cut and paste code, fix later! But where should I put it?
std::vector<test_instance_generator> get_all_permitted_test_generators(
	double max_numvoters,
	const std::vector<copeland_scenario> canonical_scenarios,
	const relative_criterion_const & relative_criterion,
	const std::map<int, fixed_cand_equivalences> cand_equivs,
	rng & randomizer) {

	std::vector<test_instance_generator> out;

	// Must get before and after numcands for the relative criterion,
	// then go through canonical scenarios for those candidate numbers.
	// The inner loop should go through every (before cand, after cand)
	// pair consistent with after_as_before for that criterion.

	size_t numcands_before = relative_criterion.get_numcands_before(),
		numcands_after = relative_criterion.get_numcands_after();

	fixed_cand_equivalences before_cand_remapping =
		cand_equivs.find(numcands_before)->second, after_cand_remapping =
		cand_equivs.find(numcands_after)->second;

	for (copeland_scenario x: canonical_scenarios) {
		if (x.get_numcands() != numcands_before) { continue; }

		for (copeland_scenario y: canonical_scenarios) {
			if (y.get_numcands() != numcands_after) { continue; }
			test_generator cur_test(randomizer.long_rand());

			std::cout << relative_criterion.name() << "\tCombination "
				<< x.to_string() << ", " << y.to_string() << ":";
			if (!cur_test.set_scenarios(x, y, max_numvoters,
				relative_criterion)) {
				std::cout << "not permitted\n";
				continue;
			}

			std::cout << "permitted\n";

			// Some candidates might be eliminated, which would make
			// equality fail. Still, we should check that the relative
			// criterion doesn't ask for more candidates than we have.
			assert (numcands_before >= relative_criterion.
				get_candidate_reordering().num_source_candidates());

			// For every allowed pair of candidates for this criterion...

			for (const auto & pair: relative_criterion.
				get_candidate_reordering()) {

				// except A, since we're looking for some other candidate
				// to compare A to...

				if (pair.first == 0) { continue; }

				// Later, we'll set the "evaluation" of any nonexistent
				// candidate's score to -infinity. (See [file] for more
				// about that.) Thus, we don't care about nonexistent
				// before candidates unless the method is no_harm, because
				// A can't possibly be helped by the nonexistent candidate's
				// score going from -infinity to something finite.
				// Analogous reasoning holds when we don't care about
				// nonexistent after candidates unless the method is
				// no_help.

				if (pair.first == CP_NONEXISTENT &&
					!relative_criterion.no_harm()) {
					std::cout << "Ignoring nonexistent first without" <<
						" no-harm." << std::endl;
					continue;
				}
				if (pair.second == CP_NONEXISTENT &&
					!relative_criterion.no_help()) {
					std::cout << "Ignoring nonexistent second without" <<
						" no-help." << std::endl;
					continue;
				}

				// Don't deal with eliminations yet.
				if (pair.first == CP_NONEXISTENT ||
					pair.second == CP_NONEXISTENT) {
					std::cout << "Ignoring eliminated pair." << std::endl;
					continue;
				}

				// Make sure it's safe to cast to unsigned. We should have
				// handled every special case by the point we get here.
				assert(pair.first >= 0 && pair.second >= 0);

				size_t before_cand_idx = pair.first,
					after_cand_idx = pair.second;
			
				assert(after_cand_idx < numcands_after);

				test_instance_generator to_add(cur_test);
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
