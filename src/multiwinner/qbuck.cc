
#include "qbuck.h"
#include "quotas.h"
#include <limits>

std::pair<bool, candscore> qltd_pr::get_first_above_quota(const
	std::vector<std::vector<double> > &
	positional_matrix, const std::vector<bool> & hopefuls,
	const double quota, double start_at, double & surplus) const {

	// This is a reimplementation of QLTD, where we abort after reaching
	// a certain quota. Maybe this is code replication, but we'll see later.
	// In any case, starting at first place, we sum up votes for that place,
	// then check if any is above quota. If someone is, then we abort and
	// return the name and the round number (round number negated because
	// higher score is better).
	//
	// If nobody's above quota, even with all votes counted, we
	// return false for the first operand. The second is undefined.

	double recordholder = -1;
	size_t num_candidates = positional_matrix.size();

	std::vector<double> tally(num_candidates, 0), old_tally = tally;

	double adj_start_at = start_at;

	size_t counter = 0;
	for (counter = 0; counter < positional_matrix[0].size() &&
		recordholder == -1; ++counter) {
		// Sum up this row
		size_t sec;
		old_tally = tally;
		for (sec = 0; sec < num_candidates; ++sec) {
			tally[sec] += positional_matrix[sec][counter];
		}

		// Check if anyone's past quota
		if (adj_start_at <= 0) {
			for (sec = 0; sec < num_candidates; ++sec) {
				if (!hopefuls[sec]) {
					continue;
				}
				if (tally[sec] <= quota) {
					continue;
				}
				if (recordholder == -1) {
					recordholder = sec;
				}
				// Beware non-neutral tiebreak!
				if (tally[sec] > tally[recordholder]) {
					recordholder = sec;
				}
			}
		}

		--adj_start_at;
	}

	// Okay, at this point we've either exhausted all the ballots, or
	// someone's above quota. Deal with the exhausted case first.
	if (recordholder == -1) {
		return {false, candscore(0, 0)};
	}

	// If we're in Bucklin mode, then the tally of the recordholder
	// gives his current support, and the surplus is just that minus
	// the quota.

	if (bucklin) {
		surplus = tally[recordholder] - quota;
		assert(surplus >= 0);

		// Set the candscore return value.
		size_t current_round = counter;
		return {true, candscore(recordholder, -current_round)};
	}

	// Otherwise, we're in QLTD mode.

	// There are two situations here: either we started at 0 (or at a point
	// where start_at was below or at the quota), or we didn't and start_at
	// was above quota even before.

	// In the former case, old_tally won't be above quota and we can figure
	// out how much the candidate is above by determining the fractional
	// contribution of (tally - old_tally) required to make it just above
	// quota, so surplus is zero. In the latter case, we reached quota at the
	// original start_at, and surplus is however much we were above the quota
	// then.

	// To handle this properly, we'll first calculate the round at which
	// we got above quota (fractional rounds included), and then we sum up
	// to verify. If start_at is 0 and surplus is significantly nonzero,
	// then we have a bug.

	// XXX: This should actually use the same logic as Bucklin, because
	// having the surplus always be zero doesn't make any sense (and we
	// can infer the Bucklin surplus rule anyway by assuming a continuum
	// of ranks with a uniform distribution of votes on the continuum between
	// two adjacent ranks). Instead, what *should* change is who gets elected.
	// Suppose the quota is 100, and A has 80 votes on rank 1, B has 90.
	// However, A has 150 on rank 2, B has 101. Then with QLTD, A should be
	// elected, because it takes a lower fraction of votes from rank 2 to elect
	// A than B. Now suppose that A has 80 votes on rank 1 and B has 70.
	// On rank 2, A has 1002 and B has 1003. Then A should be elected,
	// because 80 + 0.025 * (1002-80) > 100, 70 + 0.025 * (1003-70) < 100.
	// So the QLTD rule is slightly more complex than straight Bucklin.

	double reached_quota = 0;
	if (old_tally[recordholder] <= quota) {
		double current_round_fraction = renorm(
				old_tally[recordholder], tally[recordholder],
				quota, 0.0, 1.0);

		reached_quota = (counter - 1) + current_round_fraction;
		surplus = 0;

		return {true, candscore(recordholder, -reached_quota)};
	}

	// The surplus is given by start_at, so count up to it.
	// I'm not sure about this code... I guess it's used when
	// we have multiple winners at the same rank ???

	assert(start_at != 0);
	reached_quota = start_at;
	// Calculate surplus.
	tally[recordholder] = 0;
	for (size_t sec = 0; sec < counter; ++sec) {
		tally[recordholder] += std::min(1.0, reached_quota - sec) *
			positional_matrix[recordholder][sec];
	}

	surplus = tally[recordholder] - quota;

	assert(surplus >= 0);

	// Set the candscore return value.
	return {true, candscore(recordholder, -reached_quota)};
}

bool qltd_pr::is_contributing(const ballot_group & ballot,
	const std::vector<bool> & hopefuls, size_t to_this_candidate,
	double num_preferences) const {

	// Simply check whether the candidate referenced is within the
	// ceil(num_preferences) first preferences. Equal rank is considered
	// a single preference.

	// TODO: Check for off-by-one here.

	int cur_pref = 0, end_pref = ceil(num_preferences);

	ordering::const_iterator cur_pos = ballot.contents.begin();
	double old_pref = cur_pos->get_score();

	while (cur_pref < end_pref && cur_pos != ballot.contents.end()) {
		// If it has been elected, just skip ahead - since we're
		// supposed to act if it was never included.
		if (!hopefuls[cur_pos->get_candidate_num()]) {
			++cur_pos;
			continue;
		}
		if (cur_pos->get_candidate_num() == to_this_candidate) {
			return (true);
		}
		++cur_pos;
		// Handle equal ranks
		if (cur_pos != ballot.contents.end())
			if (old_pref != cur_pos->get_score()) {
				++cur_pref;
			}
	}

	return false;
}



std::list<size_t> qltd_pr::get_council(size_t council_size,
	size_t num_candidates,
	const election_t & ballots) const {

	// All start off as hopeful. As in usual Bucklin, count until some
	// candidate is above the quota. Then determine the fractional vote
	// required to make the candidate that's furthermost above the quota
	// be just above the quota. Then those who elected this person are
	// deweighted completely (since the surplus, by definition, is zero),
	// the candidate eliminated, and we go on.
	//
	// In some situations, this will give the result of a council with too
	// few members. In that case, we fill the rest according to the Borda
	// count of the ballots that remain.
	//
	// I don't think this satisfies Droop proportionality, and not just in
	// the case where we have to use the completion hack.

	// Possible adjustment: once we've elected someone, start at the
	// beginning, but without the elected person. That's like multi-round
	// QPQ, and would be really easy to implement with the "qltd" half-
	// positional class.

	election_t reweighted_ballots = ballots;
	std::vector<bool> hopefuls(num_candidates, true),
		all_hopefuls = hopefuls;

	std::list<size_t> council;
	size_t num_elected = 0;

	// First construct positional array. Then, find the round at which
	// some candidate go above quota. Once that is done and the candidate
	// is elected, reweight and reconstruct the array, and start the
	// process either at zero or at the round (including fractional) where
	// we left off (depending on some parameter?). That's slow, and would
	// be quicker if the ballot rank structure was a vector, but bah.

	// TODO: Fix that later. Make it a vector with an encapsulating class
	// that autosorts when required or after each insertion. The former
	// would be better but we'd run into const problems. We could hack that
	// with an assert but it'd be really ugly. The truth is that C++ doesn't
	// do lazy evaluation very well.

	positional_aggregator aggregator;
	borda borda_count(PT_WHOLE);

	double quota, start_at = 0;

	// 1e-13 required because of double precision number inaccuracy.
	if (hare_quota) {
		quota = get_hare_quota(reweighted_ballots, council_size);
	} else	{
		quota = get_droop_quota(reweighted_ballots, council_size);
	}

	while (num_elected < council_size) {
		// Get positional aggregation data.

		// Using all_hopefuls here gives a very bad result and
		// exhausts the method, so elimination is important.
		std::vector<std::vector<double> > positional_matrix = aggregator.
			get_positional_matrix(reweighted_ballots,
				num_candidates, num_candidates -
				num_elected, hopefuls, PT_WHOLE,
				-1);

		double surplus = 0;
		std::pair<bool, candscore> quota_result = get_first_above_quota(
				positional_matrix, hopefuls, quota, start_at, surplus);

		// TODO: Complete later.
		//assert (winner.get_candidate_num() != -1);

		// If we didn't find anybody above the quota...
		if (!quota_result.first) {
			ordering borda_result = borda_count.pos_elect(
					positional_matrix, num_candidates -
					num_elected, hopefuls);
			// This majority gets a superproportional say because
			// it's not obvious how one would handle "winners"
			// with less than a quota. It might be that QLTD-PR is
			// fundamentally flawed.
			for (ordering::const_iterator bpos = borda_result.
					begin(); bpos != borda_result.end() &&
				num_elected < council_size; ++bpos) {
				if (!hopefuls[bpos->get_candidate_num()]) {
					continue;
				}
				council.push_back(bpos->get_candidate_num());
				++num_elected;
			}

			assert(num_elected == council_size);
			return (council);
		}


		// So now we have a winner. Admit him to the council, remove
		// hopeful status, reweight, and increase num_elected.
		candscore winner = quota_result.second;
		assert(hopefuls[winner.get_candidate_num()]);
		council.push_back(winner.get_candidate_num());
		++num_elected;

		election_t::iterator lpos = reweighted_ballots.begin();

		while (lpos != reweighted_ballots.end()) {
			if (!is_contributing(*lpos, hopefuls, winner.
					get_candidate_num(), -winner.get_score())) {
				++lpos;
				continue;
			}

			// If there's no surplus, every contributing vote
			// is completely used up.
			if (surplus == 0) {
				lpos = reweighted_ballots.erase(lpos);
			} else {
				lpos->set_weight(lpos->get_weight() *
					surplus/(quota + surplus));
				++lpos;
			}
		}

		hopefuls[winner.get_candidate_num()] = false;

		if (!restart_at_zero) {
			start_at = -winner.get_score();
		}

		// We shouldn't need to recalculate the quota, but
		// numerical imprecision means we have to.
		if (council_size == num_elected) {
			continue;
		}

		if (hare_quota) {
			quota = get_hare_quota(reweighted_ballots,
					council_size - num_elected);
		} else	{
			quota = get_droop_quota(reweighted_ballots,
					council_size - num_elected);
		}
	}

	return council;
}

std::string qltd_pr::name() const {
	std::string init;

	if (bucklin) {
		init = "Quota Bucklin";
	} else	{
		init = "QLTD-PR";
	}

	if (restart_at_zero) {
		init += "(restart, ";
	} else	{
		init += "(static, ";
	}

	if (hare_quota) {
		init += "Hare)";
	} else	{
		init += "Droop)";
	}

	return init;
}
