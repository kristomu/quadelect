#include "subelections.h"

// TODO: Ignore exhausted ballots (as the definition states) instead of
// fractionally attributing them to every candidate.

void subelections::count_subelections(
	const election_t & papers,
	const std::vector<bool> & hopefuls,
	bool tiebreak) {

	included_candidates = hopefuls;
	num_candidates = hopefuls.size();
	hopeful_power_set =	power_set(hopefuls);

	// Create a vector of first preference scores for every possible
	// way to eliminate hopeful candidates. The accompanying num_candidates
	// gives the cardinality of the set, i.e. the number of remaining
	// hopefuls, so we can calculate the 1/n threshold. We also have a sum
	// of first preferences to deal with equal rank/exhausted ballots.

	first_pref_scores.clear();
	num_remaining_candidates.clear();
	num_remaining_voters.clear();

	for (const std::vector<bool> & candidate_selection: hopeful_power_set) {

		int num_remaining_candidates_here = 0;

		for (size_t i = 0; i < candidate_selection.size(); ++i) {
			if (candidate_selection[i]) {
				++num_remaining_candidates_here;
			}
		}

		ordering plur_outcome;

		if (num_remaining_candidates_here > 0) {
			plur_outcome = plurality_method.elect(papers,
					candidate_selection, num_candidates, NULL, false);
		}

		std::vector<double> first_prefs_this_selection(
			num_candidates, 0);
		double num_remaining_voters_here = 0;

		for (const candscore & cand_and_score: plur_outcome) {
			assert(cand_and_score.get_candidate_num() < (size_t)num_candidates);

			double score = cand_and_score.get_score();

			if (tiebreak) {
				score += 1e-6 * score * cand_and_score.get_candidate_num() *
					num_candidates;
			}

			first_prefs_this_selection[cand_and_score.get_candidate_num()] =
				score;
			num_remaining_voters_here += score;
		}

		first_pref_scores.push_back(first_prefs_this_selection);
		num_remaining_candidates.push_back(num_remaining_candidates_here);
		num_remaining_voters.push_back(num_remaining_voters_here);
	}
}

// Determine the disqualification relation for each level of set
// cardinality. disqualifies[i][a][b] gives whether a ~> b when
// considering subelections of i members or fewer.

// TODO? Handle higher levels with hopefuls... e.g. suppose
// A is hopeful and everybody else is not, then we don't need
// n levels. Needs testing. XXX
disqual_tensor subelections::get_level_disqualifications(
	const std::vector<bool> & hopefuls,
	bool cumulative) const {

	size_t numcands = hopefuls.size(), num_levels = numcands + 1;

	std::vector<std::vector<std::vector<bool> > > disqualifies(
		num_levels, std::vector<std::vector<bool> >(numcands,
			std::vector<bool>(numcands, true)));

	// For zero and one-candidate sets, it's impossible to get
	// two candidates into the set, so the disqualification relation
	// makes no sense. To make things easier on ourselves, we consider
	// A~>B vacuously true when there are no sets containing both A
	// and B. This makes every pair on level zero (even non-hopefuls)
	// true, and every pair except A~>A true on level one.

	size_t level, cand, against;
	for (level = 1; level < num_levels; ++level) {
		for (cand = 0; cand < numcands; ++cand) {
			if (!hopefuls[cand]) {
				for (against = 0; against < numcands; ++against) {
					disqualifies[level][cand][against] = false;
				}
				continue;
			}
			disqualifies[level][cand][cand] = false;
		}
	}

	// Go through every subelection and candidate pair, and see if the
	// former disqualifies the latter. If not, set disqualifies

	for (size_t subelection = 0; subelection < first_pref_scores.size();
		++subelection) {

		for (cand = 0; cand < numcands; ++cand) {
			if (!hopeful_power_set[subelection][cand]) {
				continue;
			}

			// The candidate's first prefs
			size_t numcands_here = num_remaining_candidates[subelection];
			double cand_fpp = first_pref_scores[subelection][cand];

			// For A ~> B to still hold, we need
			// fpA_S > numvoters/numcands_here, i.e.
			// numcands_here * fpA_S > numvoters.
			if (numcands_here * cand_fpp > num_remaining_voters[subelection]) {
				continue;
			}

			for (against = 0; against < numcands; ++against) {
				if (cand == against) {
					continue;
				}

				if (!hopeful_power_set[subelection][against]) {
					continue;
				}

				disqualifies[numcands_here][cand][against] = false;
			}
		}
	}

	// We now have each disqualifies giving the disqualification relation
	// for *exactly* that cardinality level. If that's what we were asked
	// for, return it...

	if (!cumulative) {
		return disqualifies;
	}

	// Otherwise use the prior level results to make each level's pair
	// true only if it's true for that level and every level below.

	for (cand = 0; cand < numcands; ++cand) {
		if (!hopefuls[cand]) {
			continue;
		}

		for (against = 0; against < numcands; ++against) {
			if (!hopefuls[against] || cand == against) {
				continue;
			}

			for (level = 1; level < num_levels; ++level) {
				if (!disqualifies[level-1][cand][against]) {
					disqualifies[level][cand][against] = false;
				}
			}
		}
	}

	return disqualifies;
}

void subelections::print_subelection_counts() const {

	size_t num_subelections = hopeful_power_set.size();

	for (size_t i = 0; i < num_subelections; ++i) {
		std::cout << "Subelection " << i << " contains { ";
		size_t j;

		for (j = 0; j < hopeful_power_set[i].size(); ++j) {
			if (hopeful_power_set[i][j]) {
				std::cout << j << " ";
			}
		}
		std::cout << "}\n";

		for (j = 0; j < hopeful_power_set[i].size(); ++j) {
			if (!hopeful_power_set[i][j]) {
				continue;
			}
			std::cout << "fp(" << j <<"): " <<
				first_pref_scores[i][j] << "\n";
		}
		std::cout << "\n";
	}
}

condmat subelection_tools::get_defeating_matrix(
	const disqual_tensor & beats,
	const std::vector<bool> & hopefuls, size_t level) {

	size_t num_candidates = hopefuls.size();

	// Define a Condorcet matrix for the weak defeats relation.
	// A weakly defeats B if the highest level k where either A~(k)~>B
	// or B~(k)~>A, we have A~(k)~>B.

	condmat matrix(num_candidates, 1, CM_PAIRWISE_OPP);

	for (size_t candidate = 0; candidate < num_candidates; ++candidate) {
		if (!hopefuls[candidate]) {
			continue;
		}

		for (size_t challenger = 0; challenger < candidate; ++challenger) {
			if (!hopefuls[challenger]) {
				continue;
			}

			if (beats[level][candidate][challenger]) {
				matrix.add(candidate, challenger, 1);
				matrix.add(challenger, candidate, 0);
			}

			if (beats[level][challenger][candidate]) {
				matrix.add(candidate, challenger, 0);
				matrix.add(challenger, candidate, 1);
			}
		}
	}

	return matrix;
}
