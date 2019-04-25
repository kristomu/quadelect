#include "isda.h"

size_t isda_relative_const::calc_num_before_cands(bool elimination_first,
	std::shared_ptr<relative_criterion_const> & inner_criterion_in,
	const std::vector<int> & elimination_spec_in) const {

	// If elimination happens first, then we start with the output number of
	// candidates to the elimination constraint "criterion".

	if (elimination_first) {
		return elimination_util_const(elimination_spec_in).
			get_numcands_after();
	}

	// If elimination happens last, then we start with the inner criterion's
	// input number of candidates.

	return inner_criterion_in->get_numcands_before();
}

size_t isda_relative_const::calc_num_after_cands(bool elimination_first,
	std::shared_ptr<relative_criterion_const> & inner_criterion_in,
	const std::vector<int> & elimination_spec_in) const {

	// If elimination happens first, then we end with the inner criterion's
	// output number of candidates.

	if (elimination_first) {
		return inner_criterion_in->get_numcands_after();
	}

	// If elimination happens last, then we end with the number of
	// uneliminated candidates, i.e. the elimination constraint's number
	// of candidates after.

	return elimination_util_const(elimination_spec_in).get_numcands_after();
}

// DEBUG
// Perhaps all of this should be pushed into a class and then I could
// write some tests...
void isda_relative_const::print_cand_pairs(const cand_pairs & in) const {
	for (size_t x = 0; x < in.size(); ++x) {
		std::cout << x << " -> ";
		std::copy(in[x].begin(), in[x].end(),
			std::ostream_iterator<size_t>(std::cout, " "));
		std::cout << std::endl;
	}
}

// Compose two candidate pairs. This works as follows: if (x, y) is a
// valid pair according to first, and (y, z) is a valid pair according
// to second, then the composition makes (x, z) into a valid pair. It's
// used when the ISDA layer remaps x to y, and the inner criterion
// remaps y to z.
cand_pairs isda_relative_const::compose(const cand_pairs & first,
	const cand_pairs & second) const {

	// First use maps to avoid having to resize (and check uniqueness) while
	// composing.
	std::map<size_t, std::set<size_t> > compose_temp;

	for (size_t x = 0; x < first.size(); ++x) {
		for (size_t y : first[x]) {
			if (second.size() <= y) { continue; }
			for (size_t z: second[y]) {
				// Now (x, z) is a valid combination.
				compose_temp[x].insert(z);
			}
		}
	}

	// A cand_pairs variable isn't a map, it's a vector of vectors, so
	// spool it all over before returning.
	cand_pairs out_pair(first.size());

	for (const auto & compose_cand_list : compose_temp) {
		size_t x = compose_cand_list.first;

		std::copy(compose_cand_list.second.begin(),
			compose_cand_list.second.end(),
			std::back_inserter(out_pair[x]));
	}

	return out_pair;
}

// If to_reverse says (x, y) is a valid pair, then the output says (y, x)
// is a valid pair.
cand_pairs isda_relative_const::reverse(
	const cand_pairs & to_reverse) const {

	size_t max_y = 0;

	for (const std::vector<size_t> & ys: to_reverse) {
		if (ys.empty()) { continue; }

		max_y = std::max(max_y,
			*std::max_element(ys.begin(), ys.end()));
	}

	cand_pairs reversed(max_y+1);

	for (size_t x = 0; x < to_reverse.size(); ++x) {
		for (size_t y: to_reverse[x]) {
			reversed[y].push_back(x);
		}
	}

	return reversed;
}

std::string isda_relative_const::name() const {
	return "ISDA(" + inner_criterion->name() + ")";
}

constraint_set isda_relative_const::smith_loser_constraints(
	std::string ballot_suffix, std::string description_suffix) const {

	// Use the standard name for the margin of victory parameter; this
	// fixed parameter is set by test_generator.
	pairwise_constraints pwconst("min_victory_margin");

	constraint_set loser_constraints;

	size_t numcands = elimination_spec.size();
	std::vector<size_t> losers, non_losers;

	for (size_t i = 0; i < elimination_spec.size(); ++i) {
		if (elimination_spec[i] == -1) {
			losers.push_back(i);
		} else {
			// Note that the pairwise constraints are linked to the
			// situation before elimination, thus we push the index,
			// not the number at that index (which would be the candidate
			// number after elimination).
			non_losers.push_back(i);
		}
	}

	for (size_t winner: non_losers) {
		for (size_t loser: losers) {
			loser_constraints.add(pwconst.beat_constraint(true,
				winner, loser, ballot_suffix, description_suffix,
				numcands));
		}
	}

	return loser_constraints;
}

constraint_set isda_relative_const::relative_constraints(
	std::string before_suffix, std::string after_suffix) const {

	constraint_set isda_linkage;

	if (isda_before) {
		// inner criterion's before ballot -> pairwise constraints ->
		// elimination by the given schedule -> before ballot
		// (fewer candidates)

		// inner criterion's before ballot -> inner criterion ->
		// after ballot (more candidates)

		// Pairwise constraints on before_suffix
		isda_linkage.add(smith_loser_constraints(
			"inner_" + before_suffix,
			"inner_" + before_suffix + "_isda_loser"));

		// elimination
		isda_linkage.add(eliminator.relative_constraints(
			"inner_" + before_suffix, before_suffix));

		// Inner criterion
		isda_linkage.add(inner_criterion->relative_constraints(
			"inner_" + before_suffix, after_suffix));
	} else {
		// before ballot -> inner criterion -> pairwise constraints ->
		// elimination by the given schedule -> after ballot.

		// Inner criterion
		isda_linkage.add(inner_criterion->relative_constraints(
			before_suffix, before_suffix + "_isda"));

		// Pairwise constraints on before_suffix + _isda go here
		isda_linkage.add(smith_loser_constraints(before_suffix + "_isda",
			before_suffix + "_isda_isda_loser"));

		// Elimination
		isda_linkage.add(eliminator.relative_constraints(
			before_suffix + "_isda", after_suffix));
	}

	return isda_linkage; // Now wasn't that easy?
}

// This is also rather ugly.

isda_relative_const::isda_relative_const(bool elimination_first,
	std::shared_ptr<relative_criterion_const> inner_criterion_in,
	std::vector<int> elimination_spec_in) :
	relative_criterion_const(
		calc_num_before_cands(elimination_first, inner_criterion_in,
			elimination_spec_in),
		calc_num_after_cands(elimination_first, inner_criterion_in,
			elimination_spec_in)
	), eliminator(elimination_spec_in) {

	isda_before = elimination_first;
	inner_criterion = inner_criterion_in;
	elimination_spec = elimination_spec_in;

	// Verify that the number of candidates prior to elimination match.
	if (isda_before) {
		assert(numcands_before == eliminator.get_numcands_after());
		assert(numcands_after == inner_criterion->get_numcands_after());

		// If the composition is impossible due to the inner criterion
		// requiring a different number of candidates than ISDA will
		// provide, throw an exception.
		if (inner_criterion_in->get_numcands_before() !=
			elimination_spec.size()) {
			throw std::runtime_error("ISDA: candidate number mismatch"
				" in criterion composition!");
		}

		// Since the input ballot is "un-eliminated" to produce a larger
		// ballot, and this is then fed to the inner criterion, the proper
		// candidate index matching between B and B' is the composition of
		// the elimination, reversed, and the inner criterion.
		candidate_reordering = compose(reverse(
			eliminator.get_candidate_reordering()),
			inner_criterion_in->get_candidate_reordering());
	} else {
		assert(numcands_before == inner_criterion->get_numcands_before());
		assert(numcands_after == eliminator.get_numcands_after());

		if (inner_criterion_in->get_numcands_after() !=
			elimination_spec.size()) {

			throw std::runtime_error("ISDA: candidate number mismatch"
				" in criterion composition!");	
		}

		// The proper candidate index matching between B and B' is the
		// composition of the rearrangement performed by the inner
		// criterion (since it is called first),and the elimination.
		candidate_reordering = compose(
			inner_criterion_in->get_candidate_reordering(),
			eliminator.get_candidate_reordering());
	}
}