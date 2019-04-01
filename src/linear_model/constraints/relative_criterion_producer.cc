#include "relative_criterion_producer.h"
#include "relative_criteria/mono-add-top.h"
#include "relative_criteria/mono-raise.h"
#include "relative_criteria/clones.h"

std::vector<std::unique_ptr<relative_criterion_const> >
	relative_criterion_producer::get_all(int min_num_cands,
	int max_num_cands, bool different_scenarios_only) const {

	// Note: I make use of knowledge that's not encoded in the classes
	// themselves, e.g. that clone independence takes the number of
	// candidates before cloning as the before_numcands, and after
	// cloning as the after_numcands.

	std::vector<std::unique_ptr<relative_criterion_const> >
		relative_constraints;

	int i;

	for (i = min_num_cands; i < max_num_cands; ++i) {
		// Teaming and vote_splitting
		relative_constraints.push_back(
			std::make_unique<clone_const>(i, i+1));
		// Crowding
		relative_constraints.push_back(
			std::make_unique<clone_const>(i, i+1, 1));
	}

	// HACK. FIX LATER
	// For some reason, the ordinary clone constraint above fails
	// to set up this crowding constraint. I have to find out how
	// to get every crowding constraint so that I can be sure the
	// tests cover everything. (Do that later.)
	std::vector<int> clone_spec = {0, 1, 1, 2};
	relative_constraints.push_back(
		std::make_unique<clone_const>(clone_spec));

	for (i = min_num_cands; i <= max_num_cands; ++i) {
		if (different_scenarios_only && i < 4) {
			continue;
		}
		relative_constraints.push_back(
			std::make_unique<mono_raise_const>(i));
		relative_constraints.push_back(
			std::make_unique<mono_add_top_const>(i));
	}

	return relative_constraints;
}