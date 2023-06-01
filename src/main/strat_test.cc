#include "strat_test.h"

// Refactoring scaffolding. Since we're trying to engineer
// a strategy in favor of the challenger, we divide the ballots
// into those that explicitly support the challenger and everybody
// else. Those that are indifferent to the winner and challenger
// thus don't count as supporting the challenger.
class ballots_by_support {
	public:
		std::list<ballot_group> supporting_challenger,
			others;

		double challenger_support, other_support;
		size_t winner, challenger;

		ballots_by_support(size_t winner_in, size_t challenger_in) {
			challenger_support = 0;
			other_support = 0;
			winner = winner_in;
			challenger = challenger_in;
		}
};

ballots_by_support group_by_support(
	const std::list<ballot_group> & ballots,
	size_t winner, size_t challenger) {

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

ballot_group bury(ballot_group ballot, size_t winner,
	size_t challenger) {

	ballot.replace_score(winner, ballot.get_min_score()-1);
	return ballot;
}

ballot_group compromise(ballot_group ballot, size_t winner,
	size_t challenger) {

	ballot.replace_score(challenger, ballot.get_max_score()+1);
	return ballot;
}

// Both burial and compromising at once.
ballot_group two_sided_strat(ballot_group ballot, size_t winner,
	size_t challenger) {

	return bury(compromise(ballot, winner, challenger),
		winner, challenger);
}

std::list<ballot_group> do_simple_strat(const ballots_by_support &
	grouped_ballots, ballot_group (*strat)(ballot_group, size_t,size_t)) {

	// Apply the given strategy to every strategizer's ballot.

	std::list<ballot_group> strategic_election = grouped_ballots.others;

	for (ballot_group ballot:
		grouped_ballots.supporting_challenger) {

		strategic_election.push_back(strat(ballot,
			grouped_ballots.winner, grouped_ballots.challenger));
	}

	return strategic_election;
}

// Reverse the honest outcome and place the challenger first and
// the winner last.
// E.g. if the social outcome was W=A>B>C>D=E, and the winner is W and
// challenger C, the strategic faction all vote C>D=E>B>A>W.
std::list<ballot_group> two_sided_reverse(const ballots_by_support &
	grouped_ballots, const ordering & honest_outcome) {

	std::list<ballot_group> strategic_election = grouped_ballots.others;

	ballot_group strategic_ballot(grouped_ballots.challenger_support,
		ordering_tools().reverse(honest_outcome), true, false);

	strategic_election.push_back(two_sided_strat(strategic_ballot,
		grouped_ballots.winner, grouped_ballots.challenger));

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
std::list<ballot_group> coalitional_strategy(const ballots_by_support &
	grouped_ballots, size_t numcands, size_t num_coalitions,
	pure_ballot_generator * ballot_generator, rng * randomizer) {

	std::list<ballot_group> strategic_election = grouped_ballots.others;
	double unassigned_weight = grouped_ballots.challenger_support;
	double max_support_per_coalition = grouped_ballots.challenger_support/
		num_coalitions;

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

basic_strategy get_applicable_strategy(int iteration) {
	switch(iteration) {
		case 0: return ST_BURIAL;
		case 1: return ST_COMPROMISING;
		case 2: return ST_TWOSIDED;
		case 3: return ST_REVERSE;
		default: return ST_OTHER;
	}
}

basic_strategy StrategyTest::strategize_for_election(
	const std::list<ballot_group> & ballots,
	ordering honest_outcome, size_t numcands, bool verbose) const {

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

	size_t challenger;

	std::vector<ballots_by_support> grouped_ballots(numcands,
		ballots_by_support(winner, winner));

	for (challenger = 0; challenger < numcands; ++challenger) {
		// Group the ballots (who support the challenger and who support
		// the winner?)
		grouped_ballots[challenger] = group_by_support(
				ballots, winner, challenger);
	}

	for (int iteration = 0; iteration < strategy_attempts_per_try; ++iteration) {

		basic_strategy strategy_type = get_applicable_strategy(iteration);

		// How many distinct ballots (coalitions) the strategists submit
		// for coalitional strategy, if that's what we're going to use.
		int num_coalitions = 1 + iteration % 3;

		for (challenger = 0; challenger < numcands; ++challenger) {
			if (challenger == winner) {
				continue;
			}

			if (grouped_ballots[challenger].challenger_support == 0) {
				continue;
			}
			assert(grouped_ballots[challenger].challenger_support > 0);

			std::list<ballot_group> strategic_election;

			// Choose a strategy.
			switch (strategy_type) {
				case ST_BURIAL: // Burial
					strategic_election = do_simple_strat(
						grouped_ballots[challenger], bury);
					break;
				case ST_COMPROMISING: // Compromise
					strategic_election = do_simple_strat(
						grouped_ballots[challenger], compromise);
					break;
				case ST_TWOSIDED: // Two-sided
					strategic_election = do_simple_strat(
						grouped_ballots[challenger], two_sided_strat);
					break;
				case ST_REVERSE:
					// Reverse social order with two-sided strategy
					strategic_election = two_sided_reverse(
						grouped_ballots[challenger], honest_outcome);
					break;
				default:
					strategic_election = coalitional_strategy(
						grouped_ballots[challenger], numcands, num_coalitions,
						strat_generator, randomizer);
					break;
			}

			if (verbose) {
				std::string strat_name[] = {"burial", "compromise", "two-sided",
					"reverse", "coalitional"};
				std::cout << "DEBUG: Trying strategy " << strat_name[strategy_type]
					<< std::endl;
				std::cout << "After transformation, the ballot set "
					"looks like this:" << std::endl;
				ballot_tools().print_ranked_ballots(strategic_election);
			}

			// Determine the winner again! A tie counts if our man
			// is at top rank, because he wasn't, before.
			ordering strat_result = method->elect(strategic_election,
					numcands, true);

			// Check if our candidate is now at top rank.
			for (pos = strat_result.begin(); pos !=
				strat_result.end() &&
				pos->get_score() == strat_result.
				begin()->get_score(); ++pos) {
				if (pos->get_candidate_num() == challenger) {
					strategy_worked = true;
				}
			}

			if (verbose && strategy_worked) {
				std::cout << "Strategy to elect " << challenger
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
				return strategy_type;
			}

		}
	}

	if (verbose && !strategy_worked) {
		std::cout << "Strategy didn't work!" << std::endl;
	}

	return ST_NONE;
}

// Err, should probably not use camelcase... Fix later. TODO
// Mega method, fix later TODO
strategy_result StrategyTest::attempt_execute_strategy() {
	ordering honest_outcome;

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

	honest_outcome = method->elect(ballots, numcands, true);
	++total_generation_attempts;

	if (ordering_tools::has_multiple_winners(honest_outcome)) {
		return STRAT_TIE;
	}

	if (strategize_for_election(ballots, honest_outcome,
			numcands, false) != ST_NONE) {
		return STRAT_SUCCESS;
	}

	return STRAT_FAILED;
}
