#include <iterator>
#include <iostream>

#include <glpk.h>
#include <assert.h>

#include "gradual_c_b.h"

using namespace std;

string gradual_cond_borda::determine_name() const {
	string base = "Gradual-[" + base_method->name() + "](";
	switch(completion) {
		case GF_NONE: base += "None";
			      break;
		case GF_LEAST: base += "Least";
			       break;
		case GF_GREATEST: base += "Greatest";
				  break;
		case GF_BOTH: base += "Both";
			      break;
		default: base = "???";
			 break;
	}
	if (cardinal) base += ", cardinal";
	else base += ", ordinal";

	return(base + ")");
}

pair<ordering, bool> gradual_cond_borda::elect_inner(const 
		list<ballot_group> & papers, const vector<bool> & hopefuls, 
		int num_candidates, cache_map * cache, bool winner_only) const {

	cond_borda_matrix gcb(papers, num_candidates, CM_PAIRWISE_OPP, 
			cardinal, completion);
	condmat gcond(papers, num_candidates, CM_PAIRWISE_OPP);

	bool debug = false;

	if (debug) {
		condmat gcond(papers, num_candidates, CM_PAIRWISE_OPP);

		cout << "Gradual Cond-Borda debug :" << endl;
		cout << "GCB (row against column): " << endl;

		int counter, sec;
		for (counter = 0; counter < num_candidates; ++counter) {
			for (sec = 0; sec < num_candidates; ++sec) {
				double favor = gcb.get_magnitude(counter, sec);
				if (favor > 0) cout << "+";
				if (favor == 0) cout << " ";

				cout << favor;
				if (gcb.get_magnitude(counter, sec) > 
						gcb.get_magnitude(sec, counter))
					cout << "* ";
				else cout << "  ";
			}
			cout << endl;
		}

		cout << endl << "Condorcet matrix (row against column): " 
			<< endl;

		for (counter = 0; counter < num_candidates; ++counter) {
			for (sec = 0; sec < num_candidates; ++sec) {
				cout << gcond.get_magnitude(counter, sec);
				if (gcond.get_magnitude(counter, sec) > gcond.
						get_magnitude(sec, counter)) 
					cout << "* ";
				else cout << "  ";
			}
			cout << endl;
		}
		cout << endl << endl;
	}

	// HACK HACK
	// Remember, comma is suspect (no longer). 
	// Also consider early abort, particularly if winner_only is true.
	ordering base = base_method->pair_elect(gcb, hopefuls, false).first,
		 current;
	bool can_advance = true;

	ordering_tools otools;

	while (can_advance && otools.has_equal_rank(base)) {
		can_advance = gcb.update();
		current = base_method->pair_elect(gcb, hopefuls, false).first;
		base = otools.ranked_tiebreak(base, current, num_candidates);
	}

	return(pair<ordering, bool>(base, false));
}

gradual_cond_borda::gradual_cond_borda(pairwise_method * base_method_in,
		bool cardinal_in, completion_type completion_in) {
	base_method = base_method_in;
	completion = completion_in;
	cardinal = cardinal_in;
	cached_name = determine_name();
}
