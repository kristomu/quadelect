#include <iterator>
#include <iostream>

#include <glpk.h>
#include <assert.h>

#include "gradual_c_b.h"


std::string gradual_cond_borda::determine_name() const {
	std::string base = "Gradual-[" + base_method->name() + "](";
	switch (completion) {
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
	if (cardinal) {
		base += ", cardinal";
	} else {
		base += ", ordinal";
	}

	return (base + ")");
}

std::pair<ordering, bool> gradual_cond_borda::elect_inner(const
	election_t & papers, const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache, bool winner_only) const {

	size_t num_hopefuls = 0;

	for (bool x: hopefuls) {
		if (x) {
			++num_hopefuls;
		}
	}

	cond_borda_matrix gcb(papers, num_candidates, CM_PAIRWISE_OPP,
		cardinal, completion);
	condmat gcond(papers, num_candidates, CM_PAIRWISE_OPP);

	bool debug = false;

	if (debug) {
		condmat gcond(papers, num_candidates, CM_PAIRWISE_OPP);

		std::cout << "Gradual Cond-Borda debug :" << std::endl;
		std::cout << "GCB (row against column): " << std::endl;

		int counter, sec;
		for (counter = 0; counter < num_candidates; ++counter) {
			for (sec = 0; sec < num_candidates; ++sec) {
				double favor = gcb.get_magnitude(counter, sec);
				if (favor > 0) {
					std::cout << "+";
				}
				if (favor == 0) {
					std::cout << " ";
				}

				std::cout << favor;
				if (gcb.get_magnitude(counter, sec) >
					gcb.get_magnitude(sec, counter)) {
					std::cout << "* ";
				} else {
					std::cout << "  ";
				}
			}
			std::cout << std::endl;
		}

		std::cout << std::endl << "Condorcet matrix (row against column): "
			<< std::endl;

		for (counter = 0; counter < num_candidates; ++counter) {
			for (sec = 0; sec < num_candidates; ++sec) {
				std::cout << gcond.get_magnitude(counter, sec);
				if (gcond.get_magnitude(counter, sec) > gcond.
					get_magnitude(sec, counter)) {
					std::cout << "* ";
				} else {
					std::cout << "  ";
				}
			}
			std::cout << std::endl;
		}
		std::cout << std::endl << std::endl;
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

	return (std::pair<ordering, bool>(base, false));
}

gradual_cond_borda::gradual_cond_borda(
	std::shared_ptr<const pairwise_method> base_method_in,
	bool cardinal_in, completion_type completion_in) {
	base_method = base_method_in;
	completion = completion_in;
	cardinal = cardinal_in;
	cached_name = determine_name();
}
