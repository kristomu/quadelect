#ifndef _VOTE_COMPAT_QBUCK

#include <list>
#include <iostream>
#include "methods.cc"

using namespace std;

// QLTD-PR as copied from secprelect (where it gets extremely good scores).

class old_qltd_pr : public multiwinner_method {
	private:
		std::vector<vector<int> > construct_old_ballots(
			const election_t & ballots, int num_candidates) const;

		std::vector<int> QuotaBucklin(
			const std::vector<vector<int> > & rankings,
			int num_candidates, int council_size, bool hare,
			bool use_card, const std::vector<vector<double> > *
			rated_ballots) const;

	public:
		std::list<int> get_council(int council_size, int num_candidates,
			const election_t & ballots) const;

		string name() const {
			return ("Compat-QLTD-PR");
		}

};

// This will fail on assert if the ballots are weighted, since the old-style
// format doesn't have a parameter for that. Besides, this is only intended to
// be scaffolding. Needs CLEANUP.
std::vector<vector<int> > old_qltd_pr::construct_old_ballots(
	const election_t & ballots, int num_candidates) const {

	std::vector<vector<int> > out_ballots;

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
	const std::vector<vector<int> > & rankings,
	int num_candidates, int council_size, bool hare, bool use_card,
	const std::vector<vector<double> > * rated_ballots) const {

	// Of those that aren't elected, count the p first candidates into a
	// tally.
	// If any candidate is above quota, that candidate is chosen and the
	// voters are deweighted by surplus/quota.
	// Rerun until we're done.

	// To generalize, one'd want to deweight the voters just so that the
	// candidate's no longer above quota.

	double quota;
	if (hare) {
		quota = rankings.size()/(double)(council_size);
	} else	{
		quota = (rankings.size())/(council_size + 1) + 1;
	}

	std::vector<bool> elected(num_candidates, false);
	std::vector<int> allocated;
	std::vector<double> tally(num_candidates);
	std::vector<double> weighting(rankings.size(), 1);
	int counter, sec, last_elected = -1;
	double round = 0;
	double surplus;
	bool failsafe = false;

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
		for (int a = 0; a < rated_ballots->size(); ++a) {
			for (int b = 0; b< (*rated_ballots)[a].size(); ++b)
				if ((*rated_ballots)[a][b] > card_divisor) {
					card_divisor = (*rated_ballots)[a][b];
				}
		}
	}

	if (card_divisor == 0) {
		card_divisor = 1;
	}

	while (allocated.size() < council_size) {
		//cout << "Iter: " << round << ", " << fraction << endl;
		//cout << round << "\t" << allocated.size() << "\t" << council_size << endl;
		// Wasn't there something about that Hare some times fail to
		// fill the council?
		if ((round > (council_size + 1)))
			if (hare) {
				cout << "Damn, a timeout. Allocated size: " << allocated.size() <<
					". Switching to Droop "<<
					"quota." << endl;
				return (QuotaBucklin(rankings, num_candidates,
							council_size, !hare,
							use_card, rated_ballots));
			} else {
				cout << "This shouldn't happen. Allocated size: " << allocated.size() <<
					", now using "<<
					"previous tally." << endl;
				failsafe = true;
			}

		int step = 256;
		int low = (round - floor(round)) * step, high = step, mid;
		double behind_score = -1, behind_fract = -1;
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
						tally[cand] +=
							weighting[counter] *
							addtlwt *
							max(0.0, this_round);
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

			//		cout << floor(round) + fraction << "\t" << record_i << "\t " << quota << endl;

			/*	if (record_i == (quota + 1e-3)) {
					cout << "Found with " << record_i << endl;
					break;
				}*/
			if (record_i < quota) {
				low = mid + 1;
			} else {
				behind_score = record_i;
				behind_fract = fraction;
				high = mid - 1;
			}
		}

		//cout << "Mid is " << mid << endl;
		//cout << "Best: " << behind_score << endl;
		//assert (1 != 1);

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
							max(0.0, this_round);
						--this_round;
					}
				}
			}
		}

		int found = -1, found_count = 0;
		double found_record = -1;
		double irrespec_record = -1;
		std::list<int> candidates_past_quota;
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

		//cout << "How many above quota: " << found_count << endl;
		// If there are more than one, break by Borda.
		// Should probably be a better system, but for now, Borda'll
		// work.
		if (found_count > 1) {
			// TODO: Detect multiple with same Borda score. If
			// there are any, tiebreak randomly.

			// Doesn't seem to matter in the long run, but of course
			// matters in the short run.

			// Removed because we would have to port positional too,
			// which is too much of a bother.

			/*vector<double> borda_tally(num_candidates, 0);
			for (counter = 0; counter < rankings.size(); ++counter)
				positional_one(weighting[counter],
						rankings[counter],
						borda_tally, nauru_positional);
						//borda_positional);

			double borda_record = -1;
			int recordholder = -1;
			for (std::list<int>::const_iterator lpos =
					candidates_past_quota.begin();
					lpos != candidates_past_quota.end();
					++lpos)
				if (borda_tally[*lpos] > borda_record) {
					borda_record = borda_tally[*lpos];
					recordholder = *lpos;
				}
			found = recordholder;*/
			//		cout << "Borda winner was " << found << endl;
		}

		//	assert (found_count < 2);

		if (found != -1) {
			//		cout << "Found" << endl;
			surplus = max(0.0, tally[found] - quota);

			allocated.push_back(found);
			last_elected = found;
			// Reweight

			for (counter = 0; counter < rankings.size(); ++counter) {
				int this_round = round;
				for (sec = 0; sec < rankings[counter].size() &&
					this_round >= 0; ++sec) {
					int cand = rankings[counter][sec];

					if (!elected[cand]) {
						/*the 1e-6 is so that we never divide by zero. It's a hack, but should help against the "round 119" problem until I figure out why it's happening.*/
						if (cand == last_elected) {
							weighting[counter] *= surplus / quota;
						}
						--this_round;
					}
				}
			}
			elected[found] = true;
		} else {
			//		fraction += 0.01;
			if (fraction >= 1) {
				round++;
				fraction = 0;
			}
			//		0.01; // Meek-type adjustment. Very slow. Make adaptive later.
		}
	}

	return (allocated);
}

std::list<int> old_qltd_pr::get_council(int council_size,
	int num_candidates,
	const election_t & ballots) const {

	std::vector<vector<int> > translated = construct_old_ballots(ballots,
			num_candidates);

	std::vector<int> retval = QuotaBucklin(translated, num_candidates,
			council_size, false, false, NULL);

	std::list<int> council;

	copy(retval.begin(), retval.end(), inserter(council, council.begin()));

	return (council);
}

#endif
