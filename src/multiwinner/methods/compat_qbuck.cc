#include "compat_qbuck.h"
#include "quotas.h"

#include "singlewinner/positional/simple_methods.h"
#include "hack/msvc_random.h"

#include <iostream>

// This will fail on assert if the ballots are weighted, since the old-style
// format doesn't have a parameter for that. Besides, this is only intended to
// be scaffolding. Needs CLEANUP.
std::vector<std::vector<int> > old_qltd_pr::construct_old_ballots(
	const election_t & ballots, size_t num_candidates) const {

	std::vector<std::vector<int> > out_ballots;

	for (election_t::const_iterator ballot = ballots.begin();
		ballot != ballots.end(); ++ballot) {
		// Bah deltas
		assert(fabs(ballot->get_weight() - 1) < 1e-5);

		std::vector<int> this_ballot(num_candidates, num_candidates + 1);
		int place_count = 0;
		double old_value = ballot->contents.begin()->get_score();

		for (ordering::const_iterator pos = ballot->contents.begin();
			pos != ballot->contents.end(); ++pos) {
			if (pos->get_score() < old_value) {
				++place_count;
				old_value = pos->get_score();
			}
			this_ballot[pos->get_candidate_num()] = place_count;
		}

		out_ballots.push_back(this_ballot);
	}

	return (out_ballots);
}


std::vector<int> old_qltd_pr::QuotaBucklin(
	const election_t & ballots,
	const std::vector<std::vector<int> > & rankings,
	size_t num_candidates, size_t council_size, bool hare, bool use_card,
	const std::vector<std::vector<double> > * rated_ballots) const {

	// Of those that aren't elected, count the p first candidates into a
	// tally.
	// If any candidate is above quota, that candidate is chosen and the
	// voters are deweighted by surplus/quota.
	// Rerun until we're done.

	// To generalize, one'd want to deweight the voters just so that the
	// candidate's no longer above quota.

	// NOTE: Now uses the common quota functions, but these quotas are
	// significantly different! It may cause the method's performance to
	// change.

	double quota;
	if (hare) {
		quota = get_hare_quota(ballots, council_size);
	} else	{
		quota = get_droop_quota(ballots, council_size);
	}

	std::vector<bool> elected(num_candidates, false);
	std::vector<int> allocated;
	std::vector<double> tally(num_candidates);
	std::vector<double> weighting(rankings.size(), 1);
	size_t counter, sec;
	int last_elected = -1;
	double round = 0;
	double surplus;
	bool failsafe = false;

	size_t elected_candidates = 0;

	// Do Meek-type binary search to find the round value where a vote goes
	// just above the quota. (Actually, that would be a sort of bisection
	// search.) We can do that by having two tallies, one for all full-
	// votes, and one for what's not a full-vote, and then adjusting the
	// weight for that.

	// Meek isn't in the binary search, it's what happens when you have to
	// "skip" a candidate (because he's already been elected). Ordinarily
	// the skip just goes on to the next candidate, but it happens a bit
	// differently in Meek (redistribution proportional to how many voted
	// for the candidate with iteration to handle recursive loops where X
	// votes for Y next and Y votes for X next.

	// It'd be interesting to preface this by majority-defeat-
	// disqualification. If X > Y by greater than the quota, then
	// disqualify Y. Or maybe greater than 1 - quota.
	// Nah, must be 1-quota or otherwise minorities would suffer. Limit to
	// Smith? Could also lead to trouble if the Smith set
	// is just one. Is there an analogy of Smith for quota instead of
	// majority? That's not good, either.

	double fraction = 0;
	double card_divisor = 0;

	if (use_card) {
		for (size_t a = 0; a < rated_ballots->size(); ++a) {
			for (size_t b = 0; b< (*rated_ballots)[a].size(); ++b)
				if ((*rated_ballots)[a][b] > card_divisor) {
					card_divisor = (*rated_ballots)[a][b];
				}
		}
	}

	if (card_divisor == 0) {
		card_divisor = 1;
	}

	while (allocated.size() < (size_t)council_size) {
		// Wasn't there something about that Hare some times fail to
		// fill the council?
		if ((round > (council_size + 1))) {
			if (hare) {
				std::cout << "Damn, a timeout. Allocated size: "
					<< allocated.size() << ". Switching to Droop "<<
					"quota." << std::endl;
				return (QuotaBucklin(ballots, rankings,
							num_candidates, council_size, !hare,
							use_card, rated_ballots));
			} else {
				std::cout << "This shouldn't happen. Allocated size: "
					<< allocated.size() << ", now using "<<
					"previous tally." << std::endl;
				failsafe = true;
			}
		}

		int step = 256;
		int low = (round - floor(round)) * step, high = step, mid;
		double behind_fract = -1;
		while (low <= high) {
			mid = (low + high) / 2;
			fraction = mid / (double)step;

			for (counter = 0; counter < tally.size(); ++counter) {
				tally[counter] = 0;
			}

			for (counter = 0; counter < rankings.size();
				++counter) {
				double this_round = floor(round) + fraction;

				for (sec = 0; sec < rankings[counter].size() &&
					this_round >= 0; ++sec) {
					int cand = rankings[counter][sec];
					double addtlwt;
					if (use_card) {
						addtlwt = (*rated_ballots)[counter][cand] / card_divisor;
					} else	{
						addtlwt = 1;
					}

					if (!elected[cand]) {
						tally[cand] += weighting[counter] *
							addtlwt * std::max(0.0, this_round);
						--this_round;
					}
				}
			}

			double record_i = -1;
			for (counter = 0; counter < tally.size(); ++counter) {
				if (elected[counter]) {
					continue;
				}
				if (tally[counter] >= record_i) {
					record_i = tally[counter];
				}
			}

			if (record_i < quota) {
				low = mid + 1;
			} else {
				behind_fract = fraction;
				high = mid - 1;
			}
		}

		// Perhaps elect all of them at once? Check that later.
		// No, that's bad.
		if (behind_fract > 0) {
			fraction = behind_fract;
			for (counter = 0; counter < tally.size(); ++counter) {
				tally[counter] = 0;
			}

			for (counter = 0; counter < rankings.size();
				++counter) {
				double this_round = floor(round) +
					behind_fract;

				for (sec = 0; sec < rankings[counter].size()
					&& this_round >= 0; ++sec) {
					int cand = rankings[counter][sec];
					double addtlwt;
					if (use_card) {
						addtlwt = (*rated_ballots)[counter][cand] / card_divisor;
					} else	{
						addtlwt = 1;
					}

					if (!elected[cand]) {
						tally[cand] +=
							weighting[counter] *
							addtlwt *
							std::max(0.0, this_round);
						--this_round;
					}
				}
			}
		}

		int found = -1, found_count = 0;
		double found_record = -1;
		double irrespec_record = -1;
		council_t candidates_past_quota;
		for (counter = 0; counter < tally.size(); ++counter) {
			if (elected[counter]) {
				continue;
			}
			if (tally[counter] > irrespec_record) {
				irrespec_record = tally[counter];
			}

			if (failsafe || tally[counter] >= quota) {
				candidates_past_quota.push_back(counter);
				found_count++;
				if (tally[counter] > found_record || found_record == -1) {
					found = counter;
					found_record = tally[counter];
				}
			}
		}

		// If there are more than one, break by Borda.
		// Should probably be a better system, but for now, Borda'll
		// work.

		// This actually didn't work, and I'm patching something in here
		// that might sorta work.
		if (found_count > 1) {
			std::vector<bool> hopefuls(num_candidates, false);

			for (council_t::const_iterator lpos =
					candidates_past_quota.begin();
				lpos != candidates_past_quota.end();
				++lpos) {

				hopefuls[*lpos] = true;
			}

			ordering social_order = borda(PT_WHOLE).elect(ballots,
					hopefuls, num_candidates);

			std::vector<size_t> winners = ordering_tools::get_winners(
					social_order);

			// Pick a random winner.
			found = winners[random() % winners.size()];
		}

		if (found != -1) {
			assert(tally[found] >= quota);
			surplus = tally[found] - quota;

			allocated.push_back(found);
			last_elected = found;
			// Reweight

			double total_weight = 0;

			for (counter = 0; counter < rankings.size(); ++counter) {
				int this_round = round;
				for (sec = 0; sec < rankings[counter].size() &&
					this_round >= 0; ++sec) {
					int cand = rankings[counter][sec];

					if (!elected[cand]) {
						if (cand == last_elected) {
							weighting[counter] *= surplus / quota;
						}
						--this_round;
					}
				}

				total_weight += weighting[counter];
			}
			assert(!elected[found]);
			elected[found] = true;
			++elected_candidates;

			// Recalculate the quota to handle numerical imprecision
			if (elected_candidates < council_size) {
				if (hare) {
					quota = total_weight / (double)(council_size - elected_candidates);
				} else {
					quota = total_weight / (double)(council_size - elected_candidates + 1.0);
				}
			}
		} else {
			// If fraction == 0, seems like we're stuck. Just pick some hopefuls
			// and bail. I don't understand my own code :-) From 2008.
			if (fraction == 0) {
				for (size_t j = 0; j < elected.size()
					&& elected_candidates < council_size; ++j) {
					if (!elected[j]) {
						allocated.push_back(j);
						++elected_candidates;
					}
				}
				return allocated;
			}

			if (fraction >= 1) {
				round++;
				fraction = 0;
			}
		}
	}

	return (allocated);
}

council_t old_qltd_pr::get_council(size_t council_size,
	size_t num_candidates, const election_t & ballots) const {

	std::vector<std::vector<int> > translated = construct_old_ballots(ballots,
			num_candidates);

	std::vector<int> retval = QuotaBucklin(ballots, translated,
			num_candidates, council_size, false, false, NULL);

	council_t council;

	copy(retval.begin(), retval.end(), inserter(council, council.begin()));

	return (council);
}