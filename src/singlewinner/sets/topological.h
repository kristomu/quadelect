// These are sets defined by topologically sorting vertices (candidates) with 
// respect to edges (a certain relation). Note that these will bomb and return
// nothing if the relation is cyclical.

// We'll use a variant of Kahn's topological sort. This consists of maintaining
// a list of vertices with no incoming edges. Then we remove a vertex on the
// left of the list, add [all nodes that vertex pointed to but that now have
// no incoming edges] to the right end of the list, and repeat. If there are
// any edges left when we're done, there's a cycle somewhere and we bomb.

// That is in essence a breadth-first-search with an indegree count.
// Children are only admitted if the indegree is one, and in either case, the
// indegree count is decremented by one every time we see a link to it.

// Thus we first determine the indegree of all nodes (candidates), then start 
// the sort by admitting those with indegree zero. If there are none, it's
// cyclical, otherwise continue till the end and check if any are nonzero.
// If that's the case, then again there's a cycle.

#ifndef _SET_TOPOLOGICAL
#define _SET_TOPOLOGICAL

#include <vector>

#include "../method.h"
#include "../pairwise/method.h"

using namespace std;

class topological_set {

	private:
		vector<int> find_indegree(const abstract_condmat & input,
				const vector<bool> & hopefuls) const;

		ordering topo_sort(const abstract_condmat & input,
				const vector<bool> & hopefuls,
				vector<int> indegree) const;

	protected:
		virtual bool relation(const abstract_condmat & input,
				const vector<bool> & hopefuls, 
				int from, int to) const = 0;
};

#endif
