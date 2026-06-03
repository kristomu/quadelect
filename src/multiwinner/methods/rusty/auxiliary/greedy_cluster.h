#pragma once

#include <algorithm>
#include <iostream>
#include <assert.h>
#include <vector>
#include <math.h>

// Determine the best "equal size" clustering. This is a hack. It's
// also a greedy algorithm, and if this thing is NPC, that means we
// won't be optimal; but let's see how bad it is!

class es_sorter {

	public:
		std::vector<bool> full;

		int get_minimum(const std::vector<int>::const_iterator & begin,
			const std::vector<int>::const_iterator & end) const;

		bool operator()(
			const std::vector<std::vector<int> >::const_iterator & a,
			const std::vector<std::vector<int> >::const_iterator & b)
		const;

		static int get_clustering_error(
			const std::vector<std::vector<int> > & penalties,
			int num_clusters);
};