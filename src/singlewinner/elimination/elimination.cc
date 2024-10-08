#include "elimination.h"
#include <list>

// Loser-elimination meta-method. This method takes a base method and
// repeatedly disqualifies the loser (if loser-elimination) or those with a
// below-mean score (if average loser elimination).

ordering loser_elimination::break_tie(const ordering & original_ordering,
	const std::list<ordering> & past_ordering,
	int num_candidates) const {

	assert(!original_ordering.empty());

	ordering fixed_ordering = original_ordering;

	// While more than one candidate is ranked last, go down the
	// past_ordering list, breaking ties.

	ordering_tools otools;

	for (std::list<ordering>::const_iterator vpos = past_ordering.begin();
		vpos != past_ordering.end() &&
		ordering_tools::has_multiple_losers(fixed_ordering); ++vpos) {

		fixed_ordering = otools.tiebreak(fixed_ordering, *vpos,
				num_candidates);
	}

	// TODO: If still a tie, break by first ballot?

	return (fixed_ordering);
}

// Loser-elimination doesn't do anything special when winner_only is set to
// true, since to determine the winner, one has to eliminate all the other
// candidates anyway (early majority in IRV notwithstanding). Perhaps find a
// generalization of early majority, eventually.

std::pair<ordering, bool> loser_elimination::elect_inner(const
	election_t & papers, const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache, bool winner_only) const {

	std::list<ordering> base_method_tiebreaks;
	std::vector<bool> base_hopefuls = hopefuls;

	std::vector<int> losers;
	std::vector<int> almost_losers;

	std::pair<ordering, bool> output;
	output.second = false;

	// Determine the number of hopefuls we have.
	size_t num_hopefuls = 0, counter;

	for (counter = 0; counter < (size_t)num_candidates; ++counter) {
		if (hopefuls[counter]) {
			++num_hopefuls;
		}
	}

	if (bottom_two_runoff) {
		pairwise = condmat(CM_PAIRWISE_OPP);
		pairwise.count_ballots(papers, (size_t)num_candidates);
	}

	int rank = 0;

	bool debug = false;
	std::map<size_t, std::string> fakecand;
	ordering_tools otools;

	if (debug) {
		std::string f = "!";

		for (counter = 0; counter < 26; ++counter) {
			f[0] = (char)('A' + counter);
			fakecand[counter] = f;
		}
	}

	// While we have candidates left to add...
	while (output.first.size() < num_hopefuls) {
		if (debug) {
			std::cout << "Loop de loop, rank is " << rank << std::endl;
		}

		// Get the output for the base method.
		ordering this_round = base->elect(papers, base_hopefuls,
				num_candidates, cache, false);

		// Then eliminate and add candidates. If we're using average
		// elimination, calculate the average and dump all the
		// candidates below or at the average to our output, otherwise
		// add the loser, breaking ties if there is more than one.

		ordering::const_iterator pos;
		ordering::const_reverse_iterator rpos;

		if (average_loser_elim) {
			double total = 0;
			int inner_num_cands = 0;

			// Orderings where candidates have been eliminated may
			// still have those candidates listed; in that case,
			// the scores are undefined and should not be
			// considered.
			for (pos = this_round.begin(); pos != this_round.end();
				++pos)
				if (base_hopefuls[pos->get_candidate_num()]) {
					total += pos->get_score();
					++inner_num_cands;
				}

			int elim_this_round = 0;

			// Add them to the output order and actually eliminate
			// them by setting them as no longer hopeful.
			// Do the mean comparison as (num cands * score) <= total
			// instead of dividing through by num_cands - the former has
			// better numerical precision.
			for (rpos = this_round.rbegin(); rpos !=
				this_round.rend() && (inner_num_cands * rpos->get_score())
				<= total; ++rpos) {
				if (!base_hopefuls[rpos->get_candidate_num()]) {
					continue;
				}

				// TODO: Make ties tied instead of arbitrarily tiebroken.
				// e.g. all candidates have equal first preference votes,
				// then Carey should put them all as tied instead of
				// ordering them in some non-neutral order.

				// Currently doesn't work so...
				output.first.insert(candscore(rpos->
						get_candidate_num(),
						rank++));
				base_hopefuls[rpos->get_candidate_num()] =
					false;
				++elim_this_round;
			}

			// This shouldn't happen, but might if there's numerical
			// imprecision. In that case, every remaining candidate ties
			// for first.

			assert(elim_this_round != 0);

			if (elim_this_round == 0) {
				for (rpos = this_round.rbegin(); rpos !=
					this_round.rend(); ++rpos) {

					output.first.insert(candscore(rpos->get_candidate_num(),
							rank));
					base_hopefuls[rpos->get_candidate_num()] = false;
					++elim_this_round;
				}
			}
		} else {
			// DEBUG
			if (debug) {
				std::cout << "this_round: " << otools.
					ordering_to_text(this_round, fakecand,
						true) << std::endl;
			}

			// Otherwise, break any potential ties, then get
			// the loser. Note that the tiebreak function will
			// return immediately if there are no ties, and that
			// we assume there is only one loser after the tiebreak
			// is done (which means we may have to break by ballot
			// in there later, e.g. if it's an exact tie).

			ordering mod_this_round = break_tie(this_round,
					base_method_tiebreaks, num_candidates);

			if (debug) {
				std::cout << "mod_this_round: " << otools.
					ordering_to_text(mod_this_round,
						fakecand, true) << std::endl;
			}

			// Determine loser and add him to the elimination list,
			// then actually eliminate him.

			// This is an ugly hack: what we really need to do is recurse
			// on every choice of loser to determine who can actually
			// win (or become second, third, etc). TODO: Do that later.

			losers.clear();
			almost_losers.clear();

			auto pos = mod_this_round.rbegin(),
				 almost_loser_start = mod_this_round.rbegin();

			while (pos != mod_this_round.rend() &&
				pos->get_score() == mod_this_round.rbegin()->get_score()) {
				losers.push_back(pos->get_candidate_num());
				++pos;
			}

			// Get the almost losers if there are any.

			if (pos != mod_this_round.rend()) {
				almost_loser_start = pos;

				while (pos != mod_this_round.rend() &&
					pos->get_score() == almost_loser_start->get_score()) {
					almost_losers.push_back(pos->get_candidate_num());
					++pos;
				}

			}

			// YUCK! In essence this does a tiebreak based on the candidate
			// number! FIX LATER!

			int loser = losers[0];

			// If we're doing bottom-two runoff, then if there's more than
			// one loser, use the other loser as the candidate to compare
			// against; otherwise use the first almost-loser.

			if (bottom_two_runoff) {
				int challenger = -1;
				if (losers.size() > 1) {
					challenger = losers[1];
				}
				if (challenger == -1 && !almost_losers.empty()) {
					challenger = almost_losers[0];
				}

				if (challenger != -1 && pairwise.beats(loser, challenger)) {
					loser = challenger;
				}
			}

			output.first.insert(candscore(loser, rank++));

			assert(base_hopefuls[loser]);
			base_hopefuls[loser] = false;
		}

		// Add the base output to list of previous outputs (for tiebreak
		// purposes). It's really just needed when not doing average
		// elimination, but for aesthetics, we add it no matter what.
		// If first_differences is true, we should add the new ballot
		// to the end - otherwise, we should add it to the beginning.
		if (first_differences) {
			base_method_tiebreaks.push_back(this_round);
		} else	{
			base_method_tiebreaks.push_front(this_round);
		}
	}

	return (output);
}

loser_elimination::loser_elimination(
	std::shared_ptr<const election_method> base_method,
	bool average_loser, bool use_first_diff) : pairwise(CM_PAIRWISE_OPP) {

	assert(base_method != NULL);
	base = base_method;
	average_loser_elim = average_loser;
	first_differences = use_first_diff;
	bottom_two_runoff = false;

	cached_name = determine_name();
}

loser_elimination::loser_elimination(
	std::shared_ptr<const election_method> base_method,
	bool average_loser, bool use_first_diff,
	bool btr_in) : pairwise(CM_PAIRWISE_OPP) {

	assert(base_method != NULL);

	// Can't do bottom-two runoff with average loser
	assert(!(average_loser && btr_in));

	base = base_method;
	average_loser_elim = average_loser;
	first_differences = use_first_diff;
	bottom_two_runoff = btr_in;

	cached_name = determine_name();
}


std::string loser_elimination::determine_name() const {

	std::string ref;

	if (average_loser_elim) {
		ref = "AVGEliminate-[" + base->name() + "]";
	} else	{
		if (bottom_two_runoff) {
			ref = "BTREliminate-[";
		} else {
			ref = "Eliminate-[";
		}

		ref += base->name() + "]";
	}

	if (first_differences) {
		ref += "/fd";
	} else	{
		ref += "/ld";
	}

	return (ref);
}
