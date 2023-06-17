#include "strat_test.h"

ballots_by_support strategy_test::group_by_support(
	const std::list<ballot_group> & ballots,
	size_t winner, size_t challenger) const {

	ballots_by_support grouped_ballots(winner, challenger);

	for (const ballot_group & b_group : ballots) {
		bool seen_winner = false, seen_challenger = false;

		// Scores are initialized to -infinity so that ranked
		// candidates always beat unranked ones.
		double winner_score = -std::numeric_limits<double>::infinity(),
			   challenger_score = -std::numeric_limits<double>::infinity();

		// Find the rank for the challenger and the winner.
		// Note that these count from the top, so challenger < winner
		// means that the challenger is ranked higher than the
		// winner.

		for (auto pos = b_group.contents.begin(); pos !=
			b_group.contents.end() &&
			(!seen_winner || !seen_challenger); ++pos) {

			if (pos->get_candidate_num() == winner) {
				winner_score = pos->get_score();
				seen_winner = true;
			}
			if (pos->get_candidate_num() == challenger) {
				challenger_score = pos->get_score();
				seen_challenger = true;
			}
		}

		// We consider equal-ranks to be in favor of the winner,
		// thus we only explicitly check if the challenger was
		// rated above the winner.
		if (challenger_score > winner_score) {
			grouped_ballots.challenger_support += b_group.weight;
			grouped_ballots.supporting_challenger.push_back(b_group);
		} else {
			grouped_ballots.other_support += b_group.weight;
			grouped_ballots.others.push_back(b_group);
		}
	}

	return grouped_ballots;
}

// Maybe some map/accumulate trickery could be done here?
// I feel like I'm duplicating stuff a lot... Maybe this is better?

std::list<ballot_group> per_ballot_strat::get_strategic_election(
	const ordering & honest_outcome,
	int64_t instance_index,
	const test_cache & cache, size_t numcands,
	pure_ballot_generator * ballot_generator, rng * randomizer) const {

	// Transform each strategizer's ballot according to the strategy.

	// First turn the index into a challenger number and get the
	// relevant election grouped by challenger from the cache.

	size_t winner = cache.grouped_by_challenger[0].winner;
	chosen_challenger = skip_number(instance_index, winner);
	if (chosen_challenger >= cache.grouped_by_challenger.size()) {
		throw std::runtime_error("per_ballot_strat: invalid index " +
			itos(instance_index));
	}
	const ballots_by_support * grouped_ballots =
		&cache.grouped_by_challenger[chosen_challenger];

	std::list<ballot_group> strategic_election =
		grouped_ballots->others;

	for (ballot_group ballot:
		grouped_ballots->supporting_challenger) {

		strategic_election.push_back(modify_ballots(ballot,
				winner, grouped_ballots->challenger));
	}

	return strategic_election;
}

ballot_group burial::modify_ballots(ballot_group ballot, size_t winner,
	size_t challenger) const {

	ballot.replace_score(winner, ballot.get_min_score()-1);
	return ballot;
}

ballot_group compromising::modify_ballots(ballot_group ballot,
	size_t winner, size_t challenger) const {

	ballot.replace_score(challenger, ballot.get_max_score()+1);
	return ballot;
}

// Both burial and compromising at once.
ballot_group two_sided_strat::modify_ballots(ballot_group ballot,
	size_t winner, size_t challenger) const {

	ballot.replace_score(winner, ballot.get_min_score()-1);
	ballot.replace_score(challenger, ballot.get_max_score()+1);

	return ballot;
}

// Reverse the honest outcome and place the challenger first and
// the winner last.
// E.g. if the social outcome was W=A>B>C>D=E, and the winner is W and
// challenger C, the strategic faction all vote C>D=E>B>A>W.
std::list<ballot_group> two_sided_reverse::get_strategic_election(
	const ordering & honest_outcome,
	int64_t instance_index,
	const test_cache & cache, size_t numcands,
	pure_ballot_generator * ballot_generator, rng * randomizer) const {

	size_t winner = cache.grouped_by_challenger[0].winner;
	chosen_challenger = skip_number(instance_index, winner);
	if (chosen_challenger >= cache.grouped_by_challenger.size()) {
		throw std::runtime_error("per_ballot_strat: invalid index " +
			itos(instance_index));
	}
	const ballots_by_support * grouped_ballots =
		&cache.grouped_by_challenger[chosen_challenger];

	std::list<ballot_group> strategic_election = grouped_ballots->others;

	// Create a ballot with weight equal to the number of voters preferring
	// the challenger to the winner, with ordering equal to the reverse
	// of the honest outcome.
	ballot_group strategic_ballot(grouped_ballots->challenger_support,
		ordering_tools().reverse(honest_outcome), true, false);

	// Place the winner uniquely last and the challenger uniquely first.
	strategic_ballot.replace_score(winner,
		strategic_ballot.get_min_score()-1);
	strategic_ballot.replace_score(grouped_ballots->challenger,
		strategic_ballot.get_max_score()+1);

	return strategic_election;
}

// This produces one ballot per coalition; the members of the coalition
// all vote the same (random) way. Note that all but one of the weights
// are assumed to be discrete, and there may be fewer coalitions than
// specified if it's impossible to pass this constraint otherwise.
// This may cause problems for Gaussians; fix later (probably by a parameter
// deciding whether it's discrete or not, and then Webster's method to make
// fair rounding when not.)

// The assignment of coalition weights is not entirely uniform, but it
// seems to let the method find strategies more easily, at least for IRV.
std::list<ballot_group> coalitional_strategy::get_strategic_election(
	const ordering & honest_outcome,
	int64_t instance_index,
	const test_cache & cache, size_t numcands,
	pure_ballot_generator * ballot_generator, rng * randomizer) const {

	// We currently only support instance index = -1, i.e. a fully
	// randomized election strategy. I know, we shouldn't mush together
	// two parameters into one this way. Fix later, TODO.

	if (instance_index != -1) {
		throw std::runtime_error("Coalitional strategy only "
			"supports random");
	}

	size_t num_coalitions = randomizer->irand(1, 4);
	size_t winner = cache.grouped_by_challenger[0].winner;
	chosen_challenger = skip_number(randomizer->irand(0, numcands-1),
			winner);

	const ballots_by_support * grouped_ballots =
		&cache.grouped_by_challenger[chosen_challenger];

	std::list<ballot_group> strategic_election = grouped_ballots->others;
	double unassigned_weight = grouped_ballots->challenger_support;
	double max_support_per_coalition =
		grouped_ballots->challenger_support/num_coalitions;

	for (size_t i = 0; i < num_coalitions && unassigned_weight > 0; ++i) {

		ballot_group strategic_ballot = ballot_generator->
			generate_ballot(numcands, *randomizer);

		if (i == num_coalitions-1) {
			strategic_ballot.weight = unassigned_weight;
		} else {
			strategic_ballot.weight = round(
					randomizer->drand(0,max_support_per_coalition));
		}

		strategic_ballot.weight = std::min(strategic_ballot.weight,
				unassigned_weight);
		unassigned_weight -= strategic_ballot.weight;

		strategic_election.push_back(strategic_ballot);
	}

	return strategic_election;
}

// This should really be replaced with some kind of iteration through
// a vector of strategies, where it tries each a certain number of times
// before going to the next, skipping those that are exhausted... phew.
// things are gonna get so very messy here. I need to take a step back
// and consider how to simplify things, later.

strategy * strategy_test::get_applicable_strategy(int iteration) {
	switch (iteration) {
		case 0: return strategies[0].get(); // burial
		case 1: return strategies[1].get(); // compromising
		case 2: return strategies[2].get(); // two-sided
		case 3: return strategies[3].get(); // two-sided reverse
		default: return &coalstrat;
	}
}

bool strategy_test::strategize_for_election(
	const std::list<ballot_group> & ballots,
	ordering honest_outcome, size_t numcands, bool verbose) {

	// Ties are a problem. We could consider the strategy a success
	// if any non-winner can become a winner, but for now, just don't
	// deal with ties.
	if (ordering_tools::has_multiple_winners(honest_outcome)) {
		throw std::out_of_range("strategize_for_election: Can't do ties.");
	}

	size_t winner = honest_outcome.begin()->get_candidate_num();

	bool strategy_worked = false;
	ordering::const_iterator pos;

	if (verbose) {
		std::cout << "Trying to strategize for this election: " << std::endl;
		ballot_tools().print_ranked_ballots(ballots);
		std::cout << "Honest outcome is " << ordering_tools().ordering_to_text(
				honest_outcome, true) << std::endl;
	}

	test_cache election_data;
	std::list<ballot_group> strategic_election;

	election_data.grouped_by_challenger.resize(numcands,
		ballots_by_support(winner, winner));

	// Create an array per strategizer so that we can determine when
	// we've exhausted all the options. TODO: for strategizers with
	// extremely large ranges, just do -1 instead or draw from a random
	// permutation...

	std::vector<int64_t> instances_tried(strategies.size(), 0);

	for (size_t challenger = 0; challenger < numcands; ++challenger) {
		// Group the ballots (who support the challenger and who support
		// the winner?)
		election_data.grouped_by_challenger[challenger] =
			group_by_support(ballots, winner, challenger);
	}

	for (int iteration = 0; iteration < strategy_attempts_per_try;
		++iteration) {

		for (size_t i = 0; i < strategies.size(); ++i) {

			bool requested_strat = false;

			strategy * strategizer = strategies[i].get();

			if (strategizer->get_num_tries(numcands) < 0) {
				strategic_election = strategizer->get_strategic_election(
						honest_outcome, -1, election_data, numcands,
						ballot_gen, randomizer);
				requested_strat = true;
			} else {
				// If we've tried every instance for this strategy,
				// skip.
				if (instances_tried[i] >=
					strategizer->get_num_tries(numcands)) {
					continue;
				}
				strategic_election = strategizer->get_strategic_election(
						honest_outcome, instances_tried[i], election_data,
						numcands, ballot_gen, randomizer);

				++instances_tried[i];
				requested_strat = true;
			}

			if (!requested_strat) {
				continue;
			}

			if (verbose) {
				std::cout << "DEBUG: Trying strategy "
					<< strategizer->name() << std::endl;
				std::cout << "After transformation, the ballot set "
					"looks like this:" << std::endl;
				ballot_tools().print_ranked_ballots(strategic_election);
			}

			// Determine the winner again! A tie counts if our man
			// is at top rank, because he wasn't, before.
			ordering strat_result = method->elect(strategic_election,
					numcands, true);

			// Check if our candidate is now at top rank.
			// ??? HOW DO WE DO THIS??? Not like this, obviously!
			// Some kind of incremental construction of a disproof?
			// Or just straight-out construction of one???
			if (ordering_tools::is_winner(strat_result,
					strategizer->chosen_challenger)) {
				strategy_worked = true;
			}

			if (verbose && strategy_worked) {
				std::cout << "Strategy to elect " << strategizer->chosen_challenger
					<< " worked!" << std::endl;
				std::cout << "Outcome after strategy: " <<
					ordering_tools().ordering_to_text(strat_result,
						false) << std::endl;
				std::cout << "After strategy: " << std::endl;
				ballot_tools().print_ranked_ballots(strategic_election);
			}

			if (strategy_worked) {
				// TODO: Return a disproof instead.
				// Requires a redesign of both two-tests and this...
				return true;
			}

		}
	}

	if (verbose && !strategy_worked) {
		std::cout << "Strategy didn't work!" << std::endl;
	}

	return false;
}

strategy_result strategy_test::attempt_execute_strategy() {

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
