#include "runner.h"

bool test_runner::strategize_for_election(
	const std::list<ballot_group> & ballots,
	ordering honest_outcome, size_t numcands, bool verbose) {

	// Ties are a problem. We could consider the strategy a success
	// if any non-winner can become a winner, but for now, just don't
	// deal with ties.
	if (ordering_tools::has_multiple_winners(honest_outcome)) {
		throw std::out_of_range("strategize_for_election: Can't do ties.");
	}

	if (verbose) {
		std::cout << "Trying to strategize for this election: " << std::endl;
		ballot_tools().print_ranked_ballots(ballots);
		std::cout << "Honest outcome is " << ordering_tools().ordering_to_text(
				honest_outcome, true) << std::endl;
	}

	test_cache election_data;

	// Create the disproof (evidence) that we'll be building on.
	disproof strategy_instance;
	strategy_instance.before_outcome = honest_outcome;
	strategy_instance.before_election = ballots;

	// Create an array per strategizer so that we can determine when
	// we've exhausted all the options. For strategizers with extremely
	// large ranges, do -1 to draw from a random permutation...
	// TODO? Better design?

	std::vector<int64_t> instances_tried(strategies.size(), 0);

	int iteration = 0;
	bool exhausted_every_strategy = false;

	while (iteration < strategy_attempts_per_try
		&& !exhausted_every_strategy) {

		exhausted_every_strategy = true;

		// A possible cleanup for this is to use a list or something
		// similar and evict strategies that have been exhausted. Then
		// we've exhausted every test (counterexample generator)
		// once the list is empty.

		for (size_t i = 0; i < strategies.size() &&
			iteration < strategy_attempts_per_try; ++i) {

			bool got_strategic_election = false;

			criterion_test * strategizer = strategies[i].get();

			// This part feels a bit ugly. We go through different
			// strategies testing if we've already exhausted them or
			// if they're still usable.

			if (strategizer->get_num_tries(numcands) < 0) {
				strategizer->add_strategic_election(
					strategy_instance, -1, election_data, numcands,
					ballot_gen, randomizer);
				got_strategic_election = true;
			} else {
				// If we've tried every instance for this test, skip.
				if (instances_tried[i] >=
					strategizer->get_num_tries(numcands)) {
					continue;
				}
				strategizer->add_strategic_election(strategy_instance,
					instances_tried[i], election_data,
					numcands, ballot_gen, randomizer);

				++instances_tried[i];
				got_strategic_election = true;
			}

			if (got_strategic_election) {
				exhausted_every_strategy = false;
				++iteration;
			} else {
				continue;
			}

			if (verbose) {
				std::cout << "DEBUG: Trying strategy "
					<< strategizer->name() << std::endl;
			}

			// Determine the winner again! A tie counts if our man
			// is at top rank, because he wasn't, before.
			strategy_instance.after_outcome = method->elect(
					strategy_instance.after_election,
					numcands, true);

			// Check if the strategy worked (in which case strategy_instance
			// is now a valid disproof of strategy immunity).
			if (strategizer->is_disproof_valid(strategy_instance)) {
				if (verbose) {
					strategizer->print_disproof(strategy_instance);
				}

				// Maybe we should return a disproof instead? TODO?
				return true;
			}

		}
	}

	if (verbose) {
		std::cout << "Strategy didn't work!" << std::endl;
	}

	return false;
}

strategy_result test_runner::attempt_execute_strategy() {

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
		return STRAT_TIE;
	}

	if (strategize_for_election(ballots,
			honest_outcome, numcands, false)) {

		return STRAT_SUCCESS;
	}

	return STRAT_FAILED;
}
