#include "equivalences.h"
#include "vector_ballot.h"

#include "tools/factoradic.h"
#include "tools/ballot_tools.h"

void inc_ballot_vector(const ballot_group & ballot_to_add,
	std::vector<double> & permutation_count_vector, size_t numcands) {

	// Get the ordering.

	std::vector<int> linear_ordering;

	// for detecting equal-rank (which we don't support)
	double last_score = INFINITY;

	// TODO: Fix. Error ultimately resides in twotests or monotonicity.
	/*if (!ballot_to_add.complete) {
		throw new std::runtime_error(
			"Trying to add incomplete ballot to ballot vector!");
	}*/

	for (candscore cur_cand_vote : ballot_to_add.contents) {
		if (cur_cand_vote.get_score() == last_score) {
			throw new std::runtime_error(
				"Trying to add ballot with equal-rank to ballot vector!");
		}
		linear_ordering.push_back(cur_cand_vote.get_candidate_num());
		last_score = cur_cand_vote.get_score();
	}

	if (linear_ordering.size() != numcands) {
		throw new std::runtime_error("Trying to add truncated ballot to ballot vector!");
	}

	// Increment the relevant index.
	permutation_count_vector[factoradic().permutation_number(linear_ordering,
			numcands, 0)] += ballot_to_add.get_weight();
}

std::vector<double> get_ballot_vector(const election_t &
	election,
	size_t numcands) {

	std::vector<double> ballot_vector(factorial(numcands));

	for (const ballot_group & ballot : election) {
		inc_ballot_vector(ballot, ballot_vector, numcands);
	}

	return ballot_vector;
}

std::vector<double> get_ballot_vector(const election_scenario_pair & esp) {

	return get_ballot_vector(esp.election, esp.scenario.get_numcands());
}



election_t relabel_election_candidates(
	const election_t & election_in,
	const std::vector<int> & candidate_relabeling,
	bool disallow_elimination) {

	// order = {3, 2, 0, 1} means
	// what will be the first candidate (A) in the output is D (#3) in the input

	// We first need to invert the permutation, so that inv_permutation[x]
	// is the new candidate number of the candidate that used to be x.
	// Fortunately, that is pretty easy.

	size_t numcands = candidate_relabeling.size();
	std::vector<int> inv_relabeling(numcands, -1);

	for (size_t i = 0; i < numcands; ++i) {
		inv_relabeling[candidate_relabeling[i]] = i;
	}

	election_t election_out;

	// Now just go through and relabel everything.
	for (const ballot_group & ballot_in: election_in) {
		ballot_group ballot_out = ballot_in;
		ballot_out.contents.clear();

		for (candscore cur_cand_vote: ballot_in.contents) {
			// Is it an eliminated candidate? If so, don't add to the
			// target.
			if (inv_relabeling[cur_cand_vote.get_candidate_num()] == -1) {
				if (disallow_elimination) {
					throw std::runtime_error(
						"Trying to eliminate when that's disallowed");
				}
				continue;
			}

			ballot_out.contents.insert(
				candscore(inv_relabeling[cur_cand_vote.get_candidate_num()],
					cur_cand_vote.get_score()));
		}

		election_out.push_back(ballot_out);
	}

	return election_out;

}

// A pure permutation is bijective, so no elimination allowed.
election_t permute_election_candidates(
	const election_t & election_in,
	const std::vector<int> & candidate_permutation) {

	return relabel_election_candidates(election_in, candidate_permutation,
			true);
}

