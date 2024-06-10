#include <iostream>
#include <list>
#include <memory>
#include <vector>

#include "../tests/quick_dirty/monotonicity.h"

#include "../tools/ballot_tools.h"
#include "../ballots.h"

#include "../singlewinner/elimination/all.h"
#include "../singlewinner/experimental/all.h"
#include "../singlewinner/meta/all.h"
#include "../singlewinner/pairwise/simple_methods.h"
#include "../singlewinner/sets/all.h"
#include "../random/random.h"

#include "../generator/all.h"

int main() {

	int max_numvoters = 30, max_numcands = 4; // E.g.

	// TODO get seed from an entropy source, see quadelect proper
	std::shared_ptr<rng> rnd = std::make_shared<rng>(0);

	auto method_tested =
		std::make_shared<disqelim>();
	//std::make_shared<instant_runoff_voting>(PT_WHOLE, true);
	/*std::make_shared<slash>(std::make_shared<rmr1>(RMR_DEFEATING),
		std::make_shared<ext_minmax>(CM_WV, false));*/
	//std::make_shared<rmr1>(RMR_TWO_WAY);
	/*std::make_shared<comma>(std::make_shared<inner_burial_set>(),
		std::make_shared<rmr1>(RMR_TWO_WAY));*/
	//std::make_shared<ext_minmax>(CM_WV, false);

	std::shared_ptr<impartial> ballot_gen =
		std::make_shared<impartial>(false, false);

	std::cout << "Time to test " << method_tested->name()
		<< " for monotonicity failures!\n\n\n";

	monotone_check checker(ballot_gen, rnd, method_tested,
		max_numcands, max_numvoters);

	for (size_t i = 0;; ++i) {
		if ((i & 15) == 0) {
			std::cerr << "." << std::flush;
		}

		double monotone = checker.do_simulation();

		if (monotone == 0) {
			checker.print_counterexample();
		}

	}

	return 0;
}
