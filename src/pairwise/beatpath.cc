#include "beatpath.h"


///////////////////////////////////////////////////////////////////////////
// Beatpath matrix.

void beatpath::make_beatpaths(const abstract_condmat & input,
	const std::vector<bool> & hopefuls) {

	size_t num_candidates = input.get_num_candidates();
	set_num_voters(input.get_num_voters());

	contents = std::vector<std::vector<double> > (num_candidates,
			std::vector<double>(
				num_candidates, 0));

	// Copy it over.
	size_t i, j, k;

	for (i = 0; i < num_candidates; ++i)
		if (hopefuls[i])
			for (j = 0; j < num_candidates; ++j)
				if (hopefuls[j])
					contents[i][j] = input.get_magnitude(i,
							j, hopefuls);

	// Calculate beatpaths by Floyd-Warshall.

	for (i = 0; i < num_candidates; ++i)
		for (j = 0; j < num_candidates; ++j) {
			if (i == j) {
				continue;
			}
			for (k = 0; k < num_candidates; ++k) {
				if (i == k || j == k) {
					continue;
				}

				contents[j][k] = std::max(contents[j][k],
						std::min(contents[j][i],
							contents[i][k]));
			}
		}

	// All done!
}

double beatpath::get_internal(size_t candidate, size_t against,
	bool raw) const {
	// Same as in pairwise_matrix.

	assert(candidate < contents.size());
	assert(against < contents.size());

	if (raw) {
		return (contents[candidate][against]);
	} else	return (type.transform(contents[candidate][against],
					contents[against][candidate], num_voters));
}

beatpath::beatpath(const abstract_condmat & input, pairwise_type
	type_in) : abstract_condmat(type_in) {
	make_beatpaths(input, std::vector<bool>(input.get_num_candidates(), true));
}

beatpath::beatpath(const abstract_condmat & input, pairwise_type type_in,
	const std::vector<bool> & hopefuls) : abstract_condmat(type_in) {
	make_beatpaths(input, hopefuls);
}

// ... I think?
beatpath::beatpath(const election_t & scores,
	size_t num_candidates,
	pairwise_type type_in) : abstract_condmat(CM_PAIRWISE_OPP) {
	make_beatpaths(condmat(scores, num_candidates, type_in),
		std::vector<bool>(num_candidates, true));
}
