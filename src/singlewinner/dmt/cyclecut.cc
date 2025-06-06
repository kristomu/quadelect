#include "cyclecut.h"
#include "../pairwise/simple_methods.h"

#include <iostream>

// BEWARE: This does not handle equal rank or truncation!
// TODO: FIX LATER.
subelect_count_t get_triple_counts(const election_t & scores,
	const std::vector<bool> & hopefuls) {

	subelect_count_t counts;

	std::vector<size_t> involved_candidates(3);

	//std::cout << "Constructing triple counts...\n";

	for (const ballot_group & bg: scores) {
		//std::cout << "New ballot, weight " << bg.get_weight() << "\n";
		ordering::const_iterator cursor_a, cursor_b, cursor_c,
				 end = bg.contents.end();

		for (cursor_a = bg.contents.begin(); cursor_a != end;
			++cursor_a) {

			if (!hopefuls[cursor_a->get_candidate_num()]) {
				continue;
			}

			for (cursor_b = cursor_a; cursor_b != end; ++cursor_b) {
				if (cursor_a == cursor_b) {
					continue;
				}
				if (!hopefuls[cursor_b->get_candidate_num()]) {
					continue;
				}
				if (cursor_a->get_score() == cursor_b->get_score()) {
					continue;
				}

				for (cursor_c = cursor_b; cursor_c != end; ++cursor_c) {
					if (cursor_b == cursor_c) {
						continue;
					}
					if (!hopefuls[cursor_c->get_candidate_num()]) {
						continue;
					}
					if (cursor_b->get_score() == cursor_c->get_score()) {
						continue;
					}

					involved_candidates[0] = cursor_a->get_candidate_num();
					involved_candidates[1] = cursor_b->get_candidate_num();
					involved_candidates[2] = cursor_c->get_candidate_num();
					/*std::cout << "Updating ";
					std::copy(involved_candidates.begin(), involved_candidates.end(),
						std::ostream_iterator<size_t>(std::cout, " "));
					std::cout << "\n";*/
					std::sort(involved_candidates.begin(),
						involved_candidates.end());

					// Set a_idx to the index of the highest ranked candidate
					// in the involved candidates vector after sorting.
					// (Just go through the vector to find the index.
					// Quick and dirty.)
					size_t a_idx = 0;
					while (involved_candidates[a_idx] !=
						cursor_a->get_candidate_num()) {
						++a_idx;
					}

					subelect_count_t::iterator sc_pos =
						counts.find(involved_candidates);

					if (sc_pos == counts.end()) {
						std::vector<double> unit_count(3, 0);
						unit_count[a_idx] = bg.get_weight();

						counts[involved_candidates] = unit_count;
					} else {
						sc_pos->second[a_idx] += bg.get_weight();
					}
				}
			}
		}
	}

	return counts;
}

// cycle_fpa_fpc

cycle_fpa_fpc::cycle_fpa_fpc(const std::vector<size_t> & cycle_in,
	const subelect_count_t & subelect_count, bool verbose) {

	cycle = cycle_in;
	std::vector<size_t> sorted_cycle = cycle;
	std::sort(sorted_cycle.begin(), sorted_cycle.end());

	std::vector<double> first_prefs = subelect_count.find(
			sorted_cycle)->second;

	// Now descramble the count-order so it's in cycle order
	// instead of sorted candidate order. (Yeah, this is ugly.)

	std::vector<double> cycle_order_fpps(3);
	for (size_t i = 0; i < 3; ++i) {
		for (size_t j = 0; j < 3; ++j) {
			if (sorted_cycle[i] == cycle[j]) {
				cycle_order_fpps[j] = first_prefs[i];
			}
		}
	}

	if (verbose) {
		std::cout << "Registering a cycle ";
		std::copy(cycle_in.begin(), cycle_in.end(),
			std::ostream_iterator<size_t>(std::cout, " "));
		std::cout << " with first prefs ";
		std::copy(cycle_order_fpps.begin(), cycle_order_fpps.end(),
			std::ostream_iterator<double>(std::cout, " "));
		std::cout << "\n";
	}

	fpA_fpC_score = cycle_order_fpps[0] - cycle_order_fpps[2];
}

// Cycle-cutting

bool cycle_cutting::is_cycle(const condmat & matrix,
	const std::vector<size_t> & c) const {

	return matrix.beats(c[0], c[1]) && matrix.beats(c[1], c[2]) &&
		matrix.beats(c[2], c[0]);

}

std::pair<ordering, bool> cycle_cutting::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache,
	bool winner_only) const {

	size_t numcands = num_candidates; // HACK

	bool verbose = false; // For debugging.

	double num_voters = 0;
	for (const ballot_group & b: papers) {
		num_voters += b.get_weight();
	}

	// First get the Condorcet matrix and triple subelection count.
	// The Condorcet matrix needs to be in pairwise opposition (raw) mode
	// because we're going to modify its values later.
	condmat matrix(papers, num_candidates, initial_type);
	subelect_count_t subelect_count = get_triple_counts(
			papers, hopefuls);

	if (verbose) {
		std::cout << "Raw pairwise values before modifying the matrix:\n";
		matrix.debug_print_raw_values();
	}

	// Get the Smith set before we do anything to the matrix, so we can be
	// sure that the method passes Smith no matter what kind of "bugs" it
	// may have.
	std::pair<ordering, bool> smith_result = smith.pair_elect(
			matrix, hopefuls, cache, false);

	size_t a, b;
	// Make it non-neutral so we don't have to deal with lots of ties.
	// HACK
	for (a = 0; a < numcands; ++a) {
		for (b = a+1; b < numcands; ++b) {
			if (matrix.get_magnitude(a, b) == matrix.get_magnitude(b, a)) {
				matrix.add(a, b, 1e-5);
			}
		}
	}

	if (verbose) {
		std::cout << "Raw pairwise values after modifying the matrix:\n";
		matrix.debug_print_raw_values();
	}

	// Then determine what three-candidate cycles exist, and for each
	// of these cycles, insert the cycle, and the fpA-fpC score of the
	// first candidate in that cycle for the subelection containing
	// only the cycle members.

	std::vector<size_t> cycle(3);
	std::vector<cycle_fpa_fpc> cycle_scores;

	for (cycle[0] = 0; cycle[0] < numcands; ++cycle[0]) {
		if (!hopefuls[cycle[0]]) {
			continue;
		}

		for (cycle[1] = 0; cycle[1] < numcands; ++cycle[1]) {
			if (cycle[0] == cycle[1] || !hopefuls[cycle[1]]) {
				continue;
			}

			for (cycle[2] = 0; cycle[2] < numcands; ++cycle[2]) {
				if (cycle[0] == cycle[2] || cycle[1] == cycle[2] ||
					!hopefuls[cycle[2]]) {
					continue;
				}

				/*std::cout << "Checking if this is a cycle: ";
				std::copy(cycle.begin(), cycle.end(), std::ostream_iterator<size_t>(std::cout, " "));
				std::cout << "\n";*/

				// We only care about cycles.
				if (!is_cycle(matrix, cycle)) {
					continue;
				}

				if (verbose) {
					std::cout << (char)(cycle[0] + 'A') << " > " << (char)(
							cycle[1] + 'A') << ":" << matrix.get_magnitude(cycle[0], cycle[1]) << "\n";
					std::cout << (char)(cycle[1] + 'A') << " > " << (char)(
							cycle[2] + 'A') << ":" << matrix.get_magnitude(cycle[1], cycle[2]) << "\n";
					std::cout << (char)(cycle[2] + 'A') << " > " << (char)(
							cycle[0] + 'A') << ":" << matrix.get_magnitude(cycle[2], cycle[0]) << "\n";
				}

				cycle_scores.push_back(cycle_fpa_fpc(
						cycle, subelect_count, verbose));
			}
		}
	}

	// Sort the list in descending order, and for each member of the list,
	// check if there's still a cycle. If so, break the cycle at the edge
	// pointing to the fpA-fpC winner.

	std::sort(cycle_scores.begin(), cycle_scores.end(), std::greater<>());
	size_t priority = 0;

	for (const cycle_fpa_fpc & cur_cycle: cycle_scores) {
		if (verbose) {
			std::cout << "Checking earlier cycle ";
			std::copy(cur_cycle.cycle.begin(), cur_cycle.cycle.end(),
				std::ostream_iterator<size_t>(std::cout, " "));
			std::cout << " with score " << cur_cycle.fpA_fpC_score << "\n";
		}

		// Check if it's still a cycle, since earlier cycle-breaking
		// might have broken this cycle too.
		if (!is_cycle(matrix, cur_cycle.cycle)) {
			if (verbose) {
				std::cout << "\tBroken already, skip.\n";
			}
			continue;
		}

		if (verbose) {
			std::cout << "\tDropping defeat " << (char)('A' + cur_cycle.cycle[2])
				<< " > " << (char)('A' + cur_cycle.cycle[0]) << "\n";
		}

		// Break the cycle between the last and first candidate.
		// We should set the magnitude to maximum so that any method
		// like ranked pairs will process it first. But somehow that
		// makes the method more manipulable.
		double involved_voters = num_voters;

		// Hence this replicates the old behavior.
		if (original) {
			involved_voters = matrix.get_magnitude(
					cur_cycle.cycle[2], cur_cycle.cycle[0]) +
				matrix.get_magnitude(
					cur_cycle.cycle[0], cur_cycle.cycle[2]) - 1e-5;
		}

		matrix.set(cur_cycle.cycle[2], cur_cycle.cycle[0], priority * 1e-6);
		matrix.set(cur_cycle.cycle[0], cur_cycle.cycle[2],
			involved_voters - priority * 1e-6);
		// Re-establish tiebreak
		if (cur_cycle.cycle[0] < cur_cycle.cycle[2]) {
			matrix.add(cur_cycle.cycle[0], cur_cycle.cycle[2], 1e-5);
		} else {
			matrix.add(cur_cycle.cycle[2], cur_cycle.cycle[0], 1e-5);
		}

		++priority;
	}

	if (verbose) {
		std::cout << "Raw pairwise values after cutting cycles:\n";
		matrix.debug_print_raw_values();
	}

	// Once all of that is done, hand the modified matrix to a Condorcet
	// method and return its outcome. Ensure Smith compliance by placing
	// all Smith members first in the ranking.

	matrix.set_type(CM_WV);

	std::pair<ordering, bool> minmax_result = ext_minmax(CM_WV,
			false).pair_elect(matrix, hopefuls, cache, winner_only);

	ordering_tools otools;

	minmax_result.first = otools.ranked_tiebreak(smith_result.first,
			minmax_result.first, num_candidates);

	return minmax_result;

}
