#include "relative_criterion_producer.h"
#include "relative_criteria/mono-add-top.h"
#include "relative_criteria/mono-raise.h"
#include "relative_criteria/strong_um.h"
#include "relative_criteria/clones.h"
#include "relative_criteria/isda.h"

#include <set>

std::vector<std::shared_ptr<relative_criterion_const> >
	relative_criterion_producer::get_all(int min_num_cands,
	int max_num_cands, bool different_scenarios_only) const {

	// Note: I make use of knowledge that's not encoded in the classes
	// themselves, e.g. that clone independence takes a cloning
	// specification, and ISDA takes an elimination specification.

	// Clones. This may cover too much (i.e. create redundant criteria)
	// but should be sufficient (not miss any).

	std::vector<std::shared_ptr<relative_criterion_const> >
		monotonicity, clones, strategy, base_constraints,
		relative_constraints;

	int numcands;

	std::set<std::vector<int> > clone_specs;

	for (numcands = min_num_cands; numcands < max_num_cands; ++numcands) {
		std::vector<int> cand_permutation(numcands);
		std::iota(cand_permutation.begin(), cand_permutation.end(), 0);

		// Get all possible clone specs that keeps candidate A at the
		// beginning.
		do {
			for (int to_clone = 0; to_clone < numcands; ++to_clone) {
				for (int pos = 0; pos < numcands; ++pos) {
					std::vector<int> clone_spec = cand_permutation;
					clone_spec.insert(clone_spec.begin()+pos, to_clone);
					clone_specs.insert(clone_spec);
				}
			}
		} while (std::next_permutation(cand_permutation.begin()+1,
			cand_permutation.end()));
	}

	for (const std::vector<int> & clone_spec: clone_specs) {
		clones.push_back(std::make_shared<clone_const>(clone_spec));
	}

	// Monotonicity criteria
	for (numcands = min_num_cands; numcands <= max_num_cands; ++numcands) {
		if (different_scenarios_only && numcands < 4) {
			continue;
		}
		monotonicity.push_back(
			std::make_shared<mono_raise_const>(numcands));
		monotonicity.push_back(
			std::make_shared<mono_add_top_const>(numcands));
	}

	// Strategy criteria
	for (numcands = min_num_cands; numcands <= max_num_cands; ++numcands) {
		for (int cand = 1; cand < numcands; ++cand) {
			strategy.push_back(
				std::make_shared<strong_um>(numcands, cand));
		}
	}

	// Combine to base constraints.
	std::copy(clones.begin(), clones.end(),
		std::back_inserter(base_constraints));
	std::copy(monotonicity.begin(), monotonicity.end(),
		std::back_inserter(base_constraints));
	std::copy(strategy.begin(), strategy.end(),
		std::back_inserter(base_constraints));

	// Create ISDA constraints. This is very brute force, but we can
	// afford it due to the small number of candidates.
	// For each number of candidates above 3, it creates a possible
	// elimination schedule. For all of these schedules, for all of the
	// base constraints, it attempts to create ISDA with this constraint
	// and schedule. It may fail (if so, ISDA throws an exception).

	// The reason we start at 4 is because eliminating someone from a
	// beginning of three candidates will produce two, which is a perfect
	// tie and thus not subject to any algorithm.

	std::set<std::vector<int> > elim_schedules;

	for (numcands = std::max(4, min_num_cands); numcands <= max_num_cands;
		++numcands) {

		std::vector<int> cand_permutation(numcands), elim_schedule;
		std::iota(cand_permutation.begin(), cand_permutation.end(), 0);

		do {
			elim_schedule = cand_permutation;

			// Eliminate someone who isn't A.
			for (int cand_to_elim = 1; cand_to_elim < numcands;
				++cand_to_elim) {
				elim_schedule[cand_to_elim] = -1;
				elim_schedules.insert(elim_schedule);
				elim_schedule[cand_to_elim] = cand_permutation[
					cand_to_elim];
			}
		} while (std::next_permutation(cand_permutation.begin()+1,
			cand_permutation.end()));
	}

	for (const std::vector<int> & elim_sched : elim_schedules) {
		for (std::shared_ptr<relative_criterion_const> c: base_constraints) {
			try {
				relative_constraints.push_back(
					std::make_shared<isda_relative_const>(false, c,
						elim_sched));
			} catch (std::runtime_error & e) {

			}
			try {
				relative_constraints.push_back(
					std::make_shared<isda_relative_const>(true, c,
						elim_sched));
			} catch (std::runtime_error & e) {

			}
		}
	}

	// Add base constraints to relative ones.
	std::copy(base_constraints.begin(), base_constraints.end(),
		std::back_inserter(relative_constraints));

	return relative_constraints;
}

std::vector<std::shared_ptr<relative_criterion_const> >
	relative_criterion_producer::get_criteria(int min_num_cands,
	int max_num_cands, bool different_scenarios_only,
	const std::vector<std::string> & desired_criteria) const {

	// First get every relative criterion.

	std::vector<std::shared_ptr<relative_criterion_const> > out =
		get_all(min_num_cands, max_num_cands, different_scenarios_only);

	// If there are no filtering specs, return everything
	if (desired_criteria.empty()) { return out; }

	// Otherwise remove the undesired ones. Note erase is O(n) and
	// so the whole thing is O(n^ 2). This could be optimized with
	// cleverness, but there's no reason thus far.

	std::set<std::string> desired_criteria_set(desired_criteria.begin(),
		desired_criteria.end());

	auto cur = out.begin();

	while (cur != out.end()) {
		if (desired_criteria_set.find((*cur)->name()) ==
			desired_criteria_set.end()) {
			cur = out.erase(cur);
		} else {
			++cur;
		}
	}

	return out;
}
