#include "topological.h"


std::vector<int> topological_set::find_indegree(const abstract_condmat &
	input,
	const std::vector<bool> & hopefuls) const {

	// Set all to 0.
	std::vector<int> indegree(input.get_num_candidates(), 0);

	// Then count incoming edges.
	for (int counter = 0; counter < input.get_num_candidates(); ++counter)
		for (int sec = 0; sec < input.get_num_candidates(); ++sec)
			if (counter != sec && relation(input, hopefuls,
					counter, sec)) {
				++indegree[sec];
			}

	return (indegree);
}

ordering topological_set::topo_sort(const abstract_condmat & input,
	const std::vector<bool> & hopefuls, std::vector<int> indegree) const {

	// First of all, insert all with no incoming edges. The std::pair<int, int>
	// has the vertex (candidate) number as the first element, and a "count"
	// as the second. This count is incremented for each new element we add,
	// and thus is analogous to the "depth" of the breadth-first search.

	std::list<std::pair<int, int> > running_list;

	size_t counter;
	ordering toRet;

	for (counter = 0; counter < indegree.size(); ++counter)
		if (indegree[counter] == 0) {
			running_list.push_back(std::pair<int, int>(counter, 0));
		}

	// While there are candidates left...
	while (!running_list.empty()) {
		// Get one from the left.
		std::pair<int, int> cur = *running_list.begin();

		// Remove it.
		running_list.erase(running_list.begin());

		// Decrement the edge count of all vertices connected to this
		// one, and if the indegree is then zero, add it with the
		// current entry's depth incremented by one.
		for (counter = 0; counter < indegree.size(); ++counter)
			if (relation(input, hopefuls, cur.first, counter)) {
				--indegree[counter];
				if (indegree[counter] == 0)
					running_list.push_back(std::pair<int, int>(
							counter,
							cur.second+1));
			}

		// Finally, add the current vertex to the output ordering.
		toRet.insert(candscore(cur.first, -cur.second));
	}

	// Check if there are any nonzero degrees left. If so, cycle, so bork.
	for (counter = 0; counter < indegree.size(); ++counter)
		if (indegree[counter] != 0) {
			return (ordering());
		}

	// No? Return the ordering and we're done.
	return (toRet);
}

