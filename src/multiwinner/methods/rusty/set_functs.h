#pragma once

// Set generation functions common to "whittle down by constraints" divisor-type
// methods.

#include <iostream>
#include <algorithm>
#include <set>
#include <list>
#include <iterator>
#include <vector>
#include <math.h>
#include <assert.h>

#include "auxiliary/dsc.h"
#include "multiwinner/methods/methods.h"
#include "tools/ballot_tools.h"
#include "setwise/coalition.h"
#include "tools/tools.h"

using namespace std;

class set_functions {

	public:
		solid_coalition construct_sc(string memberset,
			double support) const;
		string show_membership(const std::set<int> & x) const;
		string show_membership(const solid_coalition & x) const;
		void power_set(int numcands, int curcand, std::set<int> current,
			std::set<int> remaining,
			std::set<std::set<int> > & power_set_out) const;
		void choose_set(int numcands, int curcand, int how_many,
			int cur_chosen, std::set<int> & current,
			std::set<std::set<int> > & choose_out) const;
		int share_candidates(const std::set<int> & smaller,
			const std::set<int> & larger,
			int early_break) const;
		int share_candidates(const solid_coalition & check_against,
			const std::set<int> & proposed,
			int early_break) const;
};

// Perhaps these should be in coalition.cc

solid_coalition set_functions::construct_sc(string memberset,
	double support) const {
	solid_coalition output;

	for (size_t counter = 0; counter < memberset.size(); ++counter) {
		output.candidates.insert(memberset[counter]-'A');
	}

	output.support = support;
	return (output);
}

string set_functions::show_membership(const std::set<int> & x) const {
	string out;

	for (std::set<int>::const_iterator p = x.begin(); p != x.end(); ++p) {
		out += 'A' + *p;
	}

	return (out);
}

string set_functions::show_membership(const solid_coalition & x) const {
	return (show_membership(x.candidates));
}

void set_functions::power_set(int numcands, int curcand,
	std::set<int> current,
	std::set<int> remaining, std::set<std::set<int> > & power_set_out) const {

	if (curcand == numcands) {
		power_set_out.insert(current);
		return;
	} else {
		std::set<int>::iterator pos = remaining.begin(), bakpos;
		while (pos != remaining.end()) {
			std::set<int> newcurr = current, newrema = remaining;
			newcurr.insert(*pos);
			newrema.erase(newrema.find(*pos));
			power_set(numcands, curcand+1, newcurr, newrema,
				power_set_out);
			++pos;
		}
	}
}

void set_functions::choose_set(int numcands, int curcand, int how_many,
	int cur_chosen, std::set<int> & current,
	std::set<std::set<int> > & choose_out) const {

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

	// Recursion, dudes. You dig it? Might be possible to speed this up
	// even more by early lookahead stuff.

	// Perhaps if (how_many - cur_chosen > numcands - curcand) return.
	// Something like that. Experiment another time.
	if (cur_chosen == how_many) {
		choose_out.insert(current);
	}

	// If we have exhausted all candidates or there's no way of making
	// up the difference, bail outta here.
	if (how_many - cur_chosen > numcands - curcand || numcands == curcand) {
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

// Checks whether smaller shares any candidates with larger. The runtime should
// be (smaller.size() * ln(larger.size())) worst case.
// The early_break parameter means "stop once we've found this many matches".
// If -1, it doesn't stop until it's done.
int set_functions::share_candidates(const std::set<int> & smaller,
	const std::set<int> & larger, int early_break) const {

	// Make use of the fact that both are sorted to produce an algorithm
	// of this kind:
	// 	For each position in the smaller set
	// 		Advance pointer of the larger set until it's equal or
	// 		above, or we hit the end.
	// 		If equal, increment shares and check if we're done.
	// 		If we hit the end, return how many matches we got.
	// 	Next

	// If we just want a pass-fail, there are all sorts of inventive tricks
	// we could use to break if there's no way we can reach early_break.
	// However, that would ruin the elegance here.

	std::set<int>::const_iterator spos, lpos = larger.begin();
	int matches = 0;

	for (spos = smaller.begin(); spos != smaller.end(); ++spos) {
		// This is first so we return early for a match like >= 0.
		if (matches == early_break && early_break > -1) {
			return (matches);
		}

		while (lpos != larger.end() && *lpos < *spos) {
			++lpos;
		}

		if (lpos == larger.end()) {
			return (matches);
		}

		if (*spos == *lpos) {
			++matches;
		}
	}

	return (matches);
}

int set_functions::share_candidates(const solid_coalition & check_against,
	const std::set<int> & proposed, int early_break) const {

	if (check_against.candidates.size() < proposed.size())
		return (share_candidates(check_against.candidates, proposed,
					early_break));
	else	return (share_candidates(proposed, check_against.candidates,
					early_break));
}