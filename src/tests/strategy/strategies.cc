#include "strategies.h"

// Maybe some map/accumulate trickery could be done here?
// I feel like I'm duplicating stuff a lot... Maybe this is better?

void per_ballot_strat::add_strategic_election_inner(
	disproof & partial_disproof, int64_t instance_index,
	const test_cache & cache, size_t /*numcands*/,
	pure_ballot_generator * ballot_generator, rng * randomizer) const {

	partial_disproof.data.clear();
	partial_disproof.disprover_name = name();

	// Transform each strategizer's ballot according to the strategy.

	// First turn the index into a challenger number and get the
	// relevant election grouped by challenger from the cache.

	size_t winner = cache.grouped_by_challenger[0].winner;
	size_t chosen_challenger = skip_number(instance_index, winner);
	if (chosen_challenger >= cache.grouped_by_challenger.size()) {
		throw std::runtime_error("per_ballot_strat: invalid index " +
			itos(instance_index));
	}

	const ballots_by_support * grouped_ballots =
		&cache.grouped_by_challenger[chosen_challenger];

	partial_disproof.data["chosen_challenger"] = chosen_challenger;
	partial_disproof.after_election = grouped_ballots->others;

	for (ballot_group ballot:
		grouped_ballots->supporting_challenger) {

		partial_disproof.after_election.push_back(modify_ballots(ballot,
				winner, grouped_ballots->challenger));
	}
}

ballot_group burial::modify_ballots(ballot_group ballot, size_t winner,
	size_t /*challenger*/) const {

	ballot.replace_score(winner, ballot.get_min_score()-1);
	return ballot;
}

ballot_group compromising::modify_ballots(ballot_group ballot,
	size_t /*winner*/, size_t challenger) const {

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
void two_sided_reverse::add_strategic_election_inner(
	disproof & partial_disproof, int64_t instance_index,
	const test_cache & cache, size_t numcands,
	pure_ballot_generator * ballot_generator, rng * randomizer) const {

	partial_disproof.data.clear();
	partial_disproof.disprover_name = name();

	size_t winner = cache.grouped_by_challenger[0].winner;
	size_t chosen_challenger = skip_number(instance_index, winner);
	if (chosen_challenger >= cache.grouped_by_challenger.size()) {
		throw std::runtime_error("per_ballot_strat: invalid index " +
			itos(instance_index));
	}
	const ballots_by_support * grouped_ballots =
		&cache.grouped_by_challenger[chosen_challenger];

	partial_disproof.data["chosen_challenger"] = chosen_challenger;
	partial_disproof.after_election = grouped_ballots->others;

	// Create a ballot with weight equal to the number of voters preferring
	// the challenger to the winner, with ordering equal to the reverse
	// of the honest outcome.
	ordering honest_outcome = partial_disproof.before_outcome;
	ballot_group strategic_ballot(grouped_ballots->challenger_support,
		ordering_tools().reverse(honest_outcome), true, false);

	// Place the winner uniquely last and the challenger uniquely first.
	strategic_ballot.replace_score(winner,
		strategic_ballot.get_min_score()-1);
	strategic_ballot.replace_score(grouped_ballots->challenger,
		strategic_ballot.get_max_score()+1);

	partial_disproof.after_election.push_back(strategic_ballot);
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
void coalitional_strategy::add_strategic_election_inner(
	disproof & partial_disproof, int64_t instance_index,
	const test_cache & cache, size_t numcands,
	pure_ballot_generator * ballot_generator, rng * randomizer) const {

	// We currently only support instance index = -1, i.e. a fully
	// randomized election strategy. I know, we shouldn't mush together
	// two parameters into one this way. Fix later, TODO.

	if (instance_index != -1) {
		throw std::runtime_error("Coalitional strategy only "
			"supports random");
	}

	partial_disproof.data.clear();
	partial_disproof.disprover_name = name();

	size_t num_coalitions = randomizer->next_int(1, 4);
	size_t winner = cache.grouped_by_challenger[0].winner;
	size_t chosen_challenger = skip_number(randomizer->next_int(numcands-1),
			winner);

	const ballots_by_support * grouped_ballots =
		&cache.grouped_by_challenger[chosen_challenger];

	partial_disproof.data["chosen_challenger"] = chosen_challenger;
	partial_disproof.after_election = grouped_ballots->others;

	double unassigned_weight = grouped_ballots->challenger_support;
	double max_support_per_coalition =
		grouped_ballots->challenger_support/num_coalitions;

	for (size_t i = 0; i < num_coalitions && unassigned_weight > 0; ++i) {

		ballot_group strategic_ballot = ballot_generator->
			generate_ballot(numcands, *randomizer);

		// We're not allowed to set ballot weights of zero. Thus, if we
		// are dealing with a situation where the current support would
		// be rounded off to zero or something higher than the
		// given max support, just spend the remaining voting power on
		// a single strategic ballot.

		// This is also the reason why the next_double has a 0.5 minimum;
		// that's so that the rounded weight will always be at least 1.

		if (i == num_coalitions-1 || max_support_per_coalition < 1) {
			strategic_ballot.set_weight(unassigned_weight);
		} else {
			strategic_ballot.set_weight(round(
					randomizer->next_double(0.5,max_support_per_coalition)));
		}

		strategic_ballot.set_weight(std::min(
				strategic_ballot.get_weight(), unassigned_weight));
		unassigned_weight -= strategic_ballot.get_weight();

		partial_disproof.after_election.push_back(strategic_ballot);
	}
}