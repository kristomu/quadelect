#ifndef _VOTE_P_SMETHODS
#define _VOTE_P_SMETHODS

#include "../method.h"
#include "../../pairwise/matrix.h"
#include "../../pairwise/beatpath.h"
#include "method.h"

#include <complex>

using namespace std;

// Some simple (low K-complexity) Condorcet methods follow.
// Minmax, "maxmin", and LR are really just all variants on:
// 	+  sum (for all j) d[i][j]^p  for some p
// or
// 	- (sum (for all j) d[j][i]^p) for some p
//
// with minmax/maxmin having p = inf, LR having p = 1. Why not implement that?
//
// With an additional parameter, we can emulate simple Copeland, but nah.

// Minmax
// Pick the candidate for which the greatest opponent magnitude is the least.
// This produces a social ordering according to the greatest opponent magnitude,
// negated (so as to keep with the convention that higher score is better).

class ord_minmax : public pairwise_method {

	public:
		pair<ordering, bool> pair_elect(const abstract_condmat & input,
				const vector<bool> & hopefuls, 
				cache_map & cache, bool winner_only) const;

		string pw_name() const { return("Minmax"); }
		ord_minmax(pairwise_type def_type_in) : 
			pairwise_method(def_type_in) { update_name(); }

};


// Perhaps clean up the code here.
// Leximax version of minmax. In case of a tie, break by next-to-greatest, etc.
// Perhaps I should call minmax first and then only tiebreak. Minmax is often
// conclusive, after all.

// Also implements "minmin", which picks the candidate that's the closest to
// reversing a defeat, i.e. the one for which the minimal defeat is the least.
// Minmin doesn't pass mono-add-top. Find out why, later. It fails a lot when
// given (margins) and the possibility of truncation, but I expected that.
class ext_minmax : public pairwise_method {
	private:
		bool minmin;
	public:
		pair<ordering, bool> pair_elect(const abstract_condmat & input,
				const vector<bool> & hopefuls,
				cache_map & cache, bool winner_only) const;

		string pw_name() const { 
			if (minmin)
				return("Ext-Minmin");
			else	return("Ext-Minmax"); }

		ext_minmax(pairwise_type def_type_in, bool minmin_in) : pairwise_method(def_type_in) {minmin = minmin_in; update_name(); }
};

// "Maxmin"
// The opposite of minmax: the winner is the one for whose least victory is
// the strongest.
// This is kinda iffy and not even Condorcet!
class maxmin : public pairwise_method {

	public:
		pair<ordering, bool> pair_elect(const abstract_condmat & input,
				const vector<bool> & hopefuls,
				cache_map & cache, bool winner_only) const;

		string pw_name() const { return("Maximin"); }
		maxmin(pairwise_type def_type_in) : pairwise_method(
				def_type_in) { update_name(); }
};

// Copeland and n-th order Copeland. WV, Margins, PO doesn't matter.
// Also may be extended to sports versions (3-1 and 2-1).
class copeland : public pairwise_method {
	private:
		double win, tie;
		unsigned int order;

	public:
		// Used for n-th order Copeland
		vector<double> get_copeland(const abstract_condmat & input,
				const vector<bool> & hopefuls, 
				const vector<double> & counterscores) const;
		pair<ordering, bool> pair_elect(const abstract_condmat & input,
				const vector<bool> & hopefuls,
				cache_map & cache, bool winner_only) const;

		string pw_name() const;
		copeland(pairwise_type def_type_in) : pairwise_method(def_type_in) { win = 1; tie = 0; order = 1; type_matters = false; update_name(); }
		copeland(pairwise_type def_type_in, unsigned int order_in, double win_in, double tie_in) : pairwise_method(def_type_in) { win = win_in; tie = tie_in; order = order_in; type_matters = false; update_name(); }

};

// The Schulze method. Uses the beatpath matrix defined elsewhere; without it,
// the implementation wouldn't be nearly as simple.

class schulze : public pairwise_method {
	public:
		pair<ordering, bool> pair_elect(const abstract_condmat & input,
				const vector<bool> & hopefuls,
				cache_map & cache, bool winner_only) const;

		string pw_name() const { return("Schulze"); }
		
		schulze(pairwise_type def_type_in) : pairwise_method(
				def_type_in) { update_name(); }
};

#endif
