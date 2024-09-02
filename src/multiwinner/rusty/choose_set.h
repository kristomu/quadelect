#include <set>
#include <list>
#include <iostream>
#include <assert.h>

using namespace std;

void choose_set(int numcands, int curcand, int how_many,
	int cur_chosen, std::set<int> & current,
	std::set<std::set<int> > & choose_out) {

	cout << "[c: " << curcand << " of " << numcands << "] [h: " << cur_chosen
		<< " of " << how_many << "]" << endl;

	// Some sanity checks. Pick k out of n with k > n doesn't make sense.
	if (curcand == 0) {
		assert(how_many <= numcands);
	}

	// If cur_chosen = how_many, insert set into output.
	// If numcands = curcand, return. Else...

	// Add a candidate to the set. As we do this in ascending order,
	// we know this candidate has to be at the end. Recurse with
	// cur_chosen +1, curcand+1. Then remove and recurse with
	// cur_chosen, curcand + 1.

	// Recursion, dudes. You dig it?

	// Perhaps if (how_many - cur_chosen > numcands - curcand) return.
	// Something like that. Experiment another time.
	if (cur_chosen == how_many) {
		choose_out.insert(current);
	}

	// If we have exhausted all candidates or there's no way of making
	// up the difference, bail outta here.
	if (how_many - cur_chosen > numcands - curcand || numcands == curcand) {
		cout << "Breaking." << endl;
		return;
	}

	// Hint that the next candidate will be added at the end (because it
	// will). Removed.
	current.insert(curcand);
	choose_set(numcands, curcand+1, how_many, cur_chosen+1, current,
		choose_out);
	// Remove the one we added.
	current.erase(--current.end());
	choose_set(numcands, curcand+1, how_many, cur_chosen, current,
		choose_out);
}

main() {
	std::set<int> foo;
	std::set<std::set<int> > choose_set_out;

	int numcands = 4;
	int how_many = 2;

	choose_set(numcands, 0, how_many, 0, foo, choose_set_out);

	cout << *(choose_set_out.rbegin()->begin()) << endl;
}
