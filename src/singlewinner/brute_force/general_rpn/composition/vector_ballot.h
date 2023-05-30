#pragma once

#include <vector>

#include "../../../../ballots.h"
#include "scenario.h"
#include "equivalences.h"

// Some functions involving vector ballot groups. Vector ballot
// groups/elections may get their own class eventually, but not yet.

// What's next?
// ballot_group --> factorial format vector
// std::list<ballot_group> --> factorial format vector
// std::list<ballot_group> --> std::list<ballot_group> permuting candidate names

// We may need the inverse of kth_permutation for this.

// Those give us a way of generating a random election and getting the
// vote vectors for all candidates (this will necessarily be a part of
// any future election_method wrapper anyway).

void inc_ballot_vector(const ballot_group & ballot_to_add,
	std::vector<double> & permutation_count_vector, size_t numcands);

std::vector<double> get_ballot_vector(const std::list<ballot_group> &
	election,
	size_t numcands);

std::vector<double> get_ballot_vector(const election_scenario_pair & esp);

std::list<ballot_group> relabel_election_candidates(
	const std::list<ballot_group> & election_in,
	const std::vector<int> & candidate_relabeling,
	bool disallow_elimination);

std::list<ballot_group> permute_election_candidates(
	const std::list<ballot_group> & election_in,
	const std::vector<int> & candidate_permutation);