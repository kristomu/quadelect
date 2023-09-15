// A little snippet for indiv_ballot_generator.

#include "ballotgen.h"

std::list<ballot_group> indiv_ballot_generator::generate_ballots_int(
	int num_voters,
	int numcands, bool do_truncate, coordinate_gen & coord_source) const {

	std::list<ballot_group> toRet;
	ballot_group to_add;
	to_add.set_weight(1);

	for (int counter = 0; counter < num_voters; ++counter) {
		to_add.contents = generate_ordering(numcands, do_truncate,
				coord_source);
		toRet.push_back(to_add);
	}

	return (toRet);
}
