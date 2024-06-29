#include "runtime.h"
#include "../tools/time_tools.h"

double runtime_sim::do_simulation() {

	election_t election = ballot_gen->generate_ballots(numvoters,
			numcands, *entropy_source);

	time_pt start = get_now();

	method->elect(election, numcands, true);

	double elapsed = secs_elapsed(start, get_now());

	return std::min(max_num_seconds, elapsed);
}