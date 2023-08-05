#include "../generator/all.h"
#include "../singlewinner/all.h"

#include "../tools/ballot_tools.h"
#include "../ballots.h"
#include "../tools/tools.h"

#include "../random/random.h"

#include <iostream>
#include <chrono>

int main(int argc, const char ** argv) {

	condorcet_set cond;
	schwartz_set xd;
	smith_set sm;

	std::vector<election_method *> methods;

	ballot_tools btools;
	ordering_tools otools;

	impartial ballot_gen(true, false);
	size_t counter;

	methods.push_back(new landau_set());
	methods.push_back(new venzke_landau_set());

	rng randomizer(0);

	ordering methodA, methodB, smith;

	std::map<size_t, std::string> fakecand;
	std::string f = "Q";

	for (counter = 0; counter < 26; ++counter) {
		f[0] = (char)('A' + counter);
		fakecand[counter] = f;
	}

	std::cout << "Trying to find discrepancies..." << std::endl;

	std::chrono::steady_clock clock;
	std::chrono::time_point<std::chrono::steady_clock> last, now;

	now = clock.now();

	for (size_t i = 0;; ++i) {

		// Regularly show how many test have been performed.
		if ((clock.now() - now) > std::chrono::seconds(5)) {
			last = now;
			now = clock.now();

			std::cout << "Testing: test #" << i << "     \r" << std::flush;
		}


		// Randomly select some number of candidates and voters so
		// as to sample a large region of election space where the
		// discrepancies (if any) might be hidden.

		int numcands = randomizer.lrand(4, 10);
		int numvoters = randomizer.lrand(1, 127);


		std::list<ballot_group> ballots = ballot_gen.generate_ballots(numvoters,
				numcands, randomizer);

		methodA = methods[0]->elect(ballots, numcands, true);
		methodB = methods[1]->elect(ballots, numcands, true);

		smith = sm.elect(ballots, numcands, true);

		if (methodA != methodB && methodA.begin()->get_candidate_num() !=
			methodB.begin()->get_candidate_num()) {
			std::string method_a_out = otools.ordering_to_text(methodA,
					false);
			std::string method_b_out = otools.ordering_to_text(methodB,
					false);
			std::string smith_out = otools.ordering_to_text(smith,
					false);

			std::cout << "\nWe have found a discrepancy" << std::endl;
			std::cout << "\t" << methods[0]->name() << "\t outcome: "
				<< method_a_out << std::endl;
			std::cout << "\t" << methods[1]->name() << "\t outcome: "
				<< method_b_out << std::endl;
			std::cout << "\tSmith set: " << sm.name() << "\t" << smith_out <<
				std::endl;

			std::vector<std::string> ballots_out = btools.ballots_to_text(
					ballots, fakecand, false);

			copy(ballots_out.begin(), ballots_out.end(),
				std::ostream_iterator<std::string>(std::cout, "\n\t"));
			std::cout << std::endl;
		}
	}
}
