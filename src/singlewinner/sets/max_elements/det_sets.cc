
#include "det_sets.h"

// "limit" is the length limit of the path, used for calculating the Landau
// (Fishburn) set. A limit of (number of candidates) is equal to one of
// infinity.

ordering det_sets_relation::nested_sets(const abstract_condmat & input,
	const std::vector<bool> & hopefuls, size_t limit) const {

	// N^3 (Floyd-Warshall)
	// If we only need the top set, we can do in N^2 (not implemented)
	// Now added a kludge to handle eliminated candidates.

	size_t extent = 0;

	std::vector<int> hopeful;
	for (size_t cand = 0; cand < input.get_num_candidates(); ++cand) {
		if (hopefuls[cand]) {
			++extent;
			hopeful.push_back(cand);
		}
	}

	// is extent if there's no path between x and y
	std::vector<std::vector<size_t> > path_len(extent,
		std::vector<size_t>(extent, extent));

	// Set up for paths of length 1
	size_t i, j, k;

	for (i = 0; i < extent; ++i)
		for (j = 0; j < extent; ++j)
			if (i != j && relation(input, hopeful[i], hopeful[j], hopefuls)) {
				path_len[i][j] = 1;
			}

	// Handle intermediate paths
	for (k = 0; k < extent; ++k)
		for (i = 0; i < extent; ++i) {
			if (k == i) {
				continue;
			}
			for (j = 0; j < extent; ++j) {
				if (k == j || i == j) {
					continue;
				}
				if (path_len[i][j] > path_len[i][k] +
					path_len[k][j])
					path_len[i][j] = path_len[i][k] +
						path_len[k][j];
			}
		}

	// Count.
	std::vector<int> path_score(extent, 0);

	for (i = 0; i < extent; ++i)
		for (j = 0; j < extent; ++j)
			if (i != j)
				if (path_len[i][j] >= std::min(extent, limit+1)) {
					--path_score[i];
				}

	// Finally, turn the count into an ordering.
	ordering toRet;

	for (i = 0; i < extent; ++i) {
		toRet.insert(candscore(hopeful[i], path_score[i]));
	}

	return (toRet);
}
