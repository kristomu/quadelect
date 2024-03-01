#include <iostream>
#include <list>
#include <memory>
#include <vector>

#include "../tools/ballot_tools.h"
#include "../ballots.h"

#include "../singlewinner/elimination.h"
#include "../singlewinner/experimental/rmr1.h"
#include "../singlewinner/meta/all.h"
#include "../singlewinner/pairwise/simple_methods.h"
#include "../random/random.h"

#include "../generator/all.h"

// Quick and dirty monotonicity checker. Something is up with the way I've
// structured my strategy tests and other classes, and it makes it pretty
// much impossible to write new test types. I really need to refactor and
// clean it up. Until then, I have to find out if my experimental methods
// are monotone, hence this hack.

int main() {

	// For a really long time:

	// Generate a random election with the given number of candidates,
	// voters and ballot distribution. Do this uncompressed.

	// Run the election through the method of choice and record the winners.
	// Create a copy for the "after" election.

	// Then lots of times:
	// Choose a winner to uprank.
	// Create a copy for the "after" election. Shuffle them and choose a random
	// number of ballots to up or downrank.
	// Go down the list increasing the score of the candidate to be raised until
	// enough ballots have been raised.
	// Get the outcome.
	// If the candidate we raised is no longer in the set, scream blue murder.

	size_t inner_iters = 10000;

	int max_numvoters = 15, max_numcands = 4; // E.g.

	rng rnd(0); // TODO get seed from an entropy source, see quadelect proper

	auto method_tested =
		//std::make_shared<instant_runoff_voting>(PT_WHOLE, true);
		std::make_shared<comma>(std::make_shared<rmr1>(),
			std::make_shared<ext_minmax>(CM_WV, false));
	//std::make_shared<rmr1>();
	//std::make_shared<ext_minmax>(CM_WV, false);

	std::shared_ptr<impartial> ballot_gen =
		std::make_shared<impartial>(false, false);

	std::cout << "Time to test " << method_tested->name()
		<< " for monotonicity failures!\n\n\n";

	std::vector<int> shuffle_indices(max_numvoters+1);

	for (size_t i = 0;; ++i) {
		if ((i & 15) == 0) {
			std::cerr << "." << std::flush;
		}

		int numvoters = rnd.next_int(3, max_numvoters+1),
			numcands = rnd.next_int(3, max_numcands+1);

		auto ballots_before = ballot_gen->generate_ballots(
				numvoters, numcands, rnd);

		// Sort the ballots to make reading them easier.
		ballots_before.sort();

		ordering before_ordering = method_tested->elect(
				ballots_before, numcands, true);

		std::vector<int> winners = ordering_tools::get_winners(
				before_ordering);

		for (size_t j = 0; j < inner_iters; ++j) {
			int winner = winners[j % winners.size()];

			// Some vector conversion shenanigans for shuffling.
			std::vector<ballot_group> after_vector(ballots_before.begin(),
				ballots_before.end());

			std::iota(shuffle_indices.begin(),
				shuffle_indices.begin() + numvoters,
				0);

			std::random_shuffle(shuffle_indices.begin(),
				shuffle_indices.begin() + numvoters);

			size_t votes_to_alter = rnd.next_int(numvoters);

			for (size_t k = 0; k < votes_to_alter; ++k) {

				// Get the candidate's score, then alter it to
				// a random value between its current score and
				// the max score plus one.

				// Strictly speaking we should do something between
				// the maximum and the score of the candidate ranked
				// just above the current candidate so that it's
				// raised at least one step, but eh, TODO.

				// We use shuffle indices so that the order of the
				// ballots does not change, making it easier to see
				// what was changed.
				int idx = shuffle_indices[k];

				candscore cur_cs = after_vector[idx].get_candidate(
						winner);
				after_vector[idx].replace_score(cur_cs,
					rnd.next_double(cur_cs.get_score(),
						after_vector[idx].get_max_score()+1));
			}

			std::list<ballot_group> ballots_after(after_vector.begin(),
				after_vector.end());

			ordering after_ordering = method_tested->elect(
					ballots_after, numcands, true);

			std::vector<int> after_winners = ordering_tools::get_winners(
					after_ordering);

			bool old_winner_present = false;
			bool weakly_monotone = after_winners.size() <= winners.size();

			for (int new_winner: after_winners) {
				old_winner_present |= (new_winner == winner);
			}

			if (!old_winner_present || !weakly_monotone) {
				if (!old_winner_present) {
					std::cout << "Damn! Not monotone!" << std::endl <<
						"[strong fail] ";
				}
				if (!weakly_monotone) {
					std::cout << "Oops! Weakly nonmonotone - the "
						"number of winners increased." << std::endl <<
						"[weak fail] ";
				}
				std::cout << numvoters << " voters, " << numcands
					<< " candidates\n";

				std::cout << "First election: " << std::endl;
				ballot_tools().print_ranked_ballots(ballots_before);

				char w = 'A' + winner;

				std::cout << "\nSecond election, after raising "
					<< w << ": " << std::endl;

				ballot_tools().print_ranked_ballots(ballots_after);

				std::cout << "\n\n";
				std::cout << "Social order for the first election: " <<
					ordering_tools::ordering_to_text(before_ordering,
						true) << "\n";
				std::cout << "Social order after raising " << w << ": " <<
					ordering_tools::ordering_to_text(after_ordering,
						true) << "\n";
			}

		}

	}

	return 0;
}