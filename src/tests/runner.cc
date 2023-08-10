#include "runner.h"

size_t test_runner::get_num_failed_criteria(
	const std::list<ballot_group> & ballots, ordering honest_outcome,
	size_t numcands, bool only_one, bool verbose) {

	// Ties are a problem. We could consider the test to be disproven
	// if any non-winner can become a winner (for e.g. monotonicity,
	// strategy); but for now, just don't handle ties.
	if (ordering_tools::has_multiple_winners(honest_outcome)) {
		throw std::out_of_range("get_num_failed_criteria: Can't do ties.");
	}

	if (disproof_attempts_per_election < tests.size()) {
		throw std::out_of_range("get_num_failed_criteria: Not enough attempts "
			"to try every test.");
	}

	if (verbose) {
		std::cout << "Trying to strategize for this election: " << std::endl;
		ballot_tools().print_ranked_ballots(ballots);
		std::cout << "Honest outcome is " << ordering_tools().ordering_to_text(
				honest_outcome, true) << std::endl;
	}

	test_cache election_data;

	// Create the disproof (evidence) that we'll be building on.
	disproof failure_instance;
	failure_instance.before_outcome = honest_outcome;
	failure_instance.before_election = ballots;

	// Create an array per tester so that we can determine when
	// we've exhausted all the options. For testers with extremely
	// large ranges, do -1 to draw from a random permutation...
	// TODO? Better design?

	std::vector<int64_t> instances_tried(tests.size(), 0);
	std::fill(failed_criteria.begin(), failed_criteria.end(), false);

	size_t iteration = 0;
	bool exhausted_every_test = false;

	size_t num_failures = 0;
	size_t max_failures = tests.size();

	if (only_one) {
		max_failures = 1;
	}

	last_run_tried_all_tests = !only_one;

	while (iteration < disproof_attempts_per_election
		&& !exhausted_every_test
		&& num_failures < max_failures) {

		exhausted_every_test = true;

		// A possible cleanup for this is to use a list or something
		// similar and evict strategies that have been exhausted. Then
		// we've exhausted every test (counterexample generator)
		// once the list is empty.

		// TODO: Relabel "strategy" stuff here to make more sense with
		// tests, but without obscuring what's happening.

		for (size_t i = 0; i < tests.size() &&
			iteration < disproof_attempts_per_election; ++i) {

			bool got_strategic_election = false;

			criterion_test * tester = tests[i].get();

			// No need to run this test if we already found a failure.
			if (failed_criteria[i]) {
				continue;
			}

			// This part feels a bit ugly. We go through different
			// strategies testing if we've already exhausted them or
			// if they're still usable.

			// TODO: Fix get() here - I just can't stand doing more
			// cleanup right now.

			if (tester->get_num_tries(numcands) < 0) {
				tester->add_strategic_election(
					failure_instance, -1, election_data, numcands,
					ballot_gen.get(), randomizer);
				got_strategic_election = true;
			} else {
				// If we've tried every instance for this test, skip.
				if (instances_tried[i] >=
					tester->get_num_tries(numcands)) {
					continue;
				}
				tester->add_strategic_election(failure_instance,
					instances_tried[i], election_data,
					numcands, ballot_gen.get(), randomizer);

				++instances_tried[i];
				got_strategic_election = true;
			}

			if (got_strategic_election) {
				exhausted_every_test = false;
				++iteration;
			} else {
				continue;
			}

			if (verbose) {
				std::cout << "DEBUG: Trying test "
					<< tester->name() << std::endl;
			}

			// Determine the winner again! A tie counts if our man
			// is at top rank, because he wasn't, before.
			failure_instance.after_outcome = method->elect(
					failure_instance.after_election,
					numcands, true);

			// Check if we found a failure (in which case failure_instance
			// is now a valid disproof of strategy immunity).
			if (tester->is_disproof_valid(failure_instance)) {
				if (verbose) {
					tester->print_disproof(failure_instance);
				}

				++num_failures;
				failed_criteria[i] = true;
				if (num_failures >= max_failures) {
					return num_failures;
				}
			}

		}
	}

	return num_failures;
}

test_result test_runner::attempt_finding_failure(bool only_one) {

	// Ties don't count because any method that's decisive will let a voter
	// break the tie.
	// Err, a method may not always be decisive! TODO: Fix that. A tie
	// should count as a method with many equal-winners and be considered
	// susceptible to strategy if someone else can be made a winner, or one
	// of the equal-winners turned into a sole winner.

	size_t numcands = numcands_min;
	if (numcands_max > numcands_min) {
		numcands = randomizer->irand(numcands_min, numcands_max+1);
	}

	ballots = ballot_gen->generate_ballots(numvoters,
			numcands, *randomizer);

	// First, get the honest winner. If it's a tie, report as such.
	// Otherwise, check if it's possible to execute a strategy that
	// makes someone else win.

	ordering honest_outcome = method->elect(ballots, numcands, true);
	++total_generation_attempts;

	if (ordering_tools::has_multiple_winners(honest_outcome)) {
		return TEST_TIE;
	}

	if (get_num_failed_criteria(ballots,
			honest_outcome, numcands, only_one, false) > 0) {

		return TEST_DISPROVEN;
	}

	return TEST_NO_DISPROOFS;
}

std::map<std::string, bool> test_runner::get_failure_pattern() const {

	if (!last_run_tried_all_tests) {
		throw std::runtime_error("test_runner: Tried to get failure pattern "
			"without having run tests!");
	}

	std::map<std::string, bool> failure_pattern;

	for (size_t i = 0; i < tests.size(); ++i) {
		failure_pattern[tests[i]->name()] = failed_criteria[i];
	}

	return failure_pattern;
}