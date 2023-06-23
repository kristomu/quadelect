#include "test.h"

#include "../tools/tools.h"
#include "../tools/ballot_tools.h"

#include <iostream>

void criterion_test::prepare_cache(test_cache & election_data,
	disproof & partial_disproof, size_t numcands) const {

	// If the cache is already populated, exit.
	if (!election_data.grouped_by_challenger.empty()) {
		// We should never be given a cache for another election.
		assert(election_data.grouped_by_challenger.size() == numcands);

		return;
	}

	// Group the ballots (who support the challenger and who support
	// the winner?)

	ordering honest_outcome = partial_disproof.before_outcome;

	if (ordering_tools::has_multiple_winners(honest_outcome)) {
		throw std::out_of_range("prepare_cache: Can't do ties.");
	}

	size_t winner = honest_outcome.begin()->get_candidate_num();

	election_data.grouped_by_challenger.resize(numcands,
		ballots_by_support(winner, winner));

	for (size_t challenger = 0; challenger < numcands; ++challenger) {
		election_data.grouped_by_challenger[challenger] =
			ballots_by_support(partial_disproof.before_election,
				winner, challenger);
	}
}

// Since we're dealing with strategies of the type "if a bunch of people who
// all prefer A to W change their ballots, then A shouldn't win", then we can
// do the same check for each, namely that the winner did actually change.
// NOTE: I don't yet do an exhaustive check that only the people who preferred
// the challenger to the winner changed their ballots, so you could construct
// a false proof that would show as true here. I also don't do any checks for
// the proof being in scope; that'll have to be done later when we add more
// test types.

bool criterion_test::is_disproof_valid(const disproof & disproof_to_verify)
const {
	size_t chosen_challenger = disproof_to_verify.data.find(
			"chosen_challenger")->second;

	if (!ordering_tools::is_winner(disproof_to_verify.after_outcome,
			chosen_challenger)) {
		return false;
	}

	if (ordering_tools::is_winner(disproof_to_verify.before_outcome,
			chosen_challenger)) {
		return false;
	}

	return true;
}

// This is not generalizable!!! TODO
void criterion_test::print_disproof(const disproof & disproof_to_print)
const {
	if (!is_disproof_valid(disproof_to_print)) {
		return;
	}

	size_t chosen_challenger = disproof_to_print.data.find(
			"chosen_challenger")->second;

	std::cout << "Strategy to elect " << chosen_challenger
		<< " worked!\n";
	std::cout << "Strategy executed: "
		<< disproof_to_print.disprover_name << "\n";

	std::cout << "Outcome before strategy: "
		<< ordering_tools().ordering_to_text(
			disproof_to_print.before_outcome, false) << "\n";
	std::cout << "Outcome after strategy: "
		<< ordering_tools().ordering_to_text(
			disproof_to_print.after_outcome, false) << "\n";

	std::cout << "Ballots before strategy: " << "\n";
	ballot_tools().print_ranked_ballots(
		disproof_to_print.before_election);

	std::cout << "Ballots after strategy: " << "\n";
	ballot_tools().print_ranked_ballots(
		disproof_to_print.after_election);
	std::cout << std::endl;
}