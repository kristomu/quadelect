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
// I feel like I'm duplicating stuff a lot...

std::list<ballot_group> bury_winner(const ballots_by_support &
	grouped_ballots) {

	// Just make a copy of everybody not supporting the challenger.
	// Then add the people supporting the challenger, but change their
	// scores of the winner to the minimum score, minus one.

	std::list<ballot_group> strategic_election = grouped_ballots.others;

	for (ballot_group strategic_ballot:
		grouped_ballots.supporting_challenger) {
		strategic_ballot.replace_score(grouped_ballots.winner,
			strategic_ballot.get_min_score()-1);
		strategic_election.push_back(strategic_ballot);
	}

	return strategic_election;
}

std::list<ballot_group> compromise(const ballots_by_support &
	grouped_ballots) {

	// The same as burial, but we give the challenger max score instead.

	std::list<ballot_group> strategic_election = grouped_ballots.others;

	for (ballot_group strategic_ballot:
		grouped_ballots.supporting_challenger) {
		strategic_ballot.replace_score(grouped_ballots.challenger,
			strategic_ballot.get_max_score()+1);
		strategic_election.push_back(strategic_ballot);
	}

	return strategic_election;
}

std::list<ballot_group> two_sided_strat(const ballots_by_support &
	grouped_ballots) {

	// Burial and compromise at the same time.

	std::list<ballot_group> strategic_election = grouped_ballots.others;

	for (ballot_group strategic_ballot:
		grouped_ballots.supporting_challenger) {
		strategic_ballot.replace_score(grouped_ballots.winner,
			strategic_ballot.get_min_score()-1);
		strategic_ballot.replace_score(grouped_ballots.challenger,
			strategic_ballot.get_max_score()+1);
		strategic_election.push_back(strategic_ballot);
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
	strategic_ballot.replace_score(grouped_ballots.challenger,
		strategic_ballot.get_max_score()+1);
	strategic_ballot.replace_score(grouped_ballots.winner,
		strategic_ballot.get_min_score()-1);

	strategic_election.push_back(strategic_ballot);

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

	for (size_t challenger = 0; challenger < numcands && !strategy_worked;
		++challenger) {
		if (challenger == winner) {
			continue;
		}

		if (verbose) {
			std::cout << "Trying to rig in favor of " << challenger << std::endl;
		}

		// Group the ballots (who support the challenger and who support
		// the winner?)
		ballots_by_support grouped_ballots = group_by_support(
				ballots, winner, challenger);

		if (grouped_ballots.challenger_support == 0) {
			continue;
		}
		assert(grouped_ballots.challenger_support > 0);

		for (int tries = 0; tries < strategy_attempts_per_try && !strategy_worked;
			++tries) {

			std::list<ballot_group> prefers_winner = grouped_ballots.others;

			// How many distinct ballots (coalitions) the strategists submit
			// for coalitional strategy.
			int num_coalitions = 1 + tries % 3;

			// Choose a strategy.
			switch (tries) {
				case ST_BURIAL: // Burial
					prefers_winner = bury_winner(grouped_ballots);
					break;
				case ST_COMPROMISING: // Compromise
					prefers_winner = compromise(grouped_ballots);
					break;
				case ST_TWOSIDED: // Two-sided
					prefers_winner = two_sided_strat(grouped_ballots);
					break;
				case ST_REVERSE:
					// Reverse social order with two-sided strategy
					prefers_winner = two_sided_reverse(grouped_ballots,
							honest_outcome);
					break;
				default:
					prefers_winner = coalitional_strategy(grouped_ballots,
							numcands, num_coalitions, strat_generator, randomizer);
					break;
			}

			if (verbose && tries <= ST_REVERSE) {
				std::string strategies[] = {"burial", "compromise", "two-sided", "reverse"};
				std::cout << "DEBUG: Trying strategy " << strategies[tries] << std::endl;
				std::cout << "After transformation, the ballot set looks like this:" <<
					std::endl;
				ballot_tools().print_ranked_ballots(prefers_winner);
			}

			// Determine the winner again! A tie counts if our man
			// is at top rank, because he wasn't, before.
			ordering strat_result = method->elect(prefers_winner,
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
					ordering_tools().ordering_to_text(strat_result, false) << std::endl;
				std::cout << "After strategy: " << std::endl;
				ballot_tools().print_ranked_ballots(prefers_winner);
			}

			if (strategy_worked) {
				if (tries < ST_OTHER) {
					return (basic_strategy)tries;
				}
				return ST_OTHER;
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
	ordering::const_iterator pos;
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

	return (STRAT_FAILED);
}
