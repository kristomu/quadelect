#include "strat_ifpp.h"

#include "pairwise/matrix.h"


// Recursive function for getting the first preferences of every
// candidate for any choice of other candidates eliminated. It's not
// written with efficiency in mind, just to get something done.
void ifpp_method_x::find_eliminated_plurality_counts(
	std::map<std::vector<bool>, std::vector<double> > & first_pref_counts,
	const election_t & election,
	std::vector<bool> & remaining_candidates,
	int num_remaining) const {

	if (num_remaining == 0) {
		return;
	}

	// If we've already covered this selection of candidates,
	// exit.
	if (first_pref_counts.find(remaining_candidates) !=
		first_pref_counts.end()) {
		return;
	}

	// Get the outcome for the current choice of remaining
	// candidates. -1 means that candidate was eliminated.
	size_t num_candidates = remaining_candidates.size();

	std::vector<double> first_prefs(num_candidates, -1);

	ordering plur_outcome = plurality_method.elect(election,
			remaining_candidates, num_candidates, NULL, false);

	for (const candscore & cand_and_score: plur_outcome) {
		assert(cand_and_score.get_candidate_num() < num_candidates);
		first_prefs[cand_and_score.get_candidate_num()] =
			cand_and_score.get_score();
	}

	first_pref_counts[remaining_candidates] = first_prefs;

	// Then recurse by removing one of each candidate.
	for (size_t i = 0; i < num_candidates; ++i) {
		if (!remaining_candidates[i]) {
			continue;
		}

		assert(first_prefs[i] >= 0);

		remaining_candidates[i] = false;
		find_eliminated_plurality_counts(first_pref_counts,
			election, remaining_candidates, num_remaining-1);
		remaining_candidates[i] = true;
	}
}

std::map<std::vector<bool>, std::vector<double> >
ifpp_method_x::get_eliminated_plurality_counts(
	const election_t & election,
	std::vector<bool> hopefuls) const {

	int num_hopefuls = 0;
	for (size_t i = 0; i < hopefuls.size(); ++i) {
		if (hopefuls[i]) {
			++num_hopefuls;
		}
	}

	std::map<std::vector<bool>, std::vector<double> > first_pref_counts;
	find_eliminated_plurality_counts(first_pref_counts,
		election, hopefuls, num_hopefuls);

	return first_pref_counts;
}

// Recursively determine the candidates' scores.

std::vector<double> ifpp_method_x::find_candidate_scores(
	const std::map<std::vector<bool>, std::vector<double> > &
	first_pref_counts,
	std::map<std::vector<bool>, std::vector<double> > & score_cache,
	std::vector<bool> & remaining_candidates) const {

	// If we've already covered this selection of candidates,
	// exit.
	if (score_cache.find(remaining_candidates) !=
		score_cache.end()) {
		return score_cache.find(remaining_candidates)->second;
	}

	// Count the number of remaining candidates and the number
	// of non-exhausted voters.

	auto fp_counts_ref = first_pref_counts.find(remaining_candidates);

	size_t i, num_candidates = remaining_candidates.size(),
			  num_remaining = 0;
	double num_voters = 0;
	std::vector<int> remaining_indices;

	for (i = 0; i < num_candidates; ++i) {
		if (!remaining_candidates[i]) {
			continue;
		}

		++num_remaining;
		assert(fp_counts_ref->second[i] >= 0);

		num_voters += fp_counts_ref->second[i];
		remaining_indices.push_back(i);
	}

	// Candidates who are not in the running will be skipped; their
	// score is zero.
	std::vector<double> scores(num_candidates, 0);

	// If there are two candidates, then one's score is A>B and the other's
	// is B>A. This is a hack: it effectively does tied at the top, which I'm
	// not sure if I want. But for now... TODO FIX LATER
	if (num_remaining == 2) {
		for (int cand_idx: remaining_indices) {
			scores[cand_idx] = fp_counts_ref->second[cand_idx];
		}
		score_cache[remaining_candidates] = scores;
		return scores;
	}

	// If there's only one candidate, that candidate has score |V|.
	// Note that truncated voters don't count.
	if (num_remaining == 1) {
		scores[i] = num_voters;
		score_cache[remaining_candidates] = scores;
		return scores;
	}


	for (i = 0; i < num_candidates; ++i) {
		if (!remaining_candidates[i]) {
			continue;
		}

		// If the given candidate has <= 1/n first preferences, recurse
		// by eliminating him.
		if (num_remaining * fp_counts_ref->second[i] > num_voters) {
			continue;
		}

		remaining_candidates[i] = false;

		std::vector<double> inherited_scores = find_candidate_scores(
				first_pref_counts, score_cache, remaining_candidates);

		remaining_candidates[i] = true;

		// Each candidate's score is the maximum obtainable, so max over the
		// inherited score array given to us.
		for (size_t j = 0; j < num_candidates; ++j) {
			scores[j] = std::max(scores[j], inherited_scores[j]);
		}
	}

	score_cache[remaining_candidates] = scores;
	return scores;
}

std::vector<double> ifpp_method_x::get_candidate_scores(
	const election_t & election,
	std::vector<bool> hopefuls, int num_candidates) const {

	std::map<std::vector<bool>, std::vector<double> >
	first_pref_counts = get_eliminated_plurality_counts(
			election, hopefuls);

	std::map<std::vector<bool>, std::vector<double> > score_cache;

	return find_candidate_scores(
			first_pref_counts, score_cache, hopefuls);
}

std::pair<ordering, bool> ifpp_method_x::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls, int num_candidates,
	cache_map * cache, bool winner_only) const {

	std::vector<double> scores = get_candidate_scores(papers,
			hopefuls, num_candidates);

	ordering outcome;

	for (int i = 0; i < num_candidates; ++i) {
		if (hopefuls[i]) {
			outcome.insert(candscore(i, scores[i]));
		}
	}

	// Break ties by Plurality score.
	// Apparently not very good, skipping for now...
	// It would make more sense to break ties leximax, but
	// I can't be arsed.
	/*ordering plur_outcome = plurality_method.elect(papers,
		hopefuls, num_candidates, NULL, false);

	outcome = ordering_tools::ranked_tiebreak(outcome,
		plur_outcome, num_candidates);*/

	return std::pair<ordering, bool>(outcome, false);
}
