// Monotonicity: Implements variants on the mono-raise criterion. These are:

// - Mono-raise: a method fails it if altering some ballots by raising X makes
//               X lose.

// - Mono-raise-delete: a method fails it if raising X and truncating the ballot
//               after X, makes X lose.

// - Mono-raise-random: a method fails it if altering some ballots by raising X
//               and replacing all ranks lower than X's new rank with candidates
//               in random order, makes X lose.

#include "mono_raise.h"

///// Mono-raise
////////////////

bool mono_raise::alter_ballot(const ordering & input,
	ordering & norm, size_t numcands,
	const std::vector<size_t> & data,
	rng & randomizer) const {

	// Let's raise/lower the candidate to raise (unless he's already
	// in first/last and not a tie).

	size_t cand_to_change = data[0];
	norm = input;

	// Raise the candidate's score ever so slightly, which will push him one
	// rank up (unless he's already in first).

	int highest_score = norm.begin()->get_score();

	bool did_something = false;

	for (ordering::iterator spos = norm.begin(); spos != norm.end() &&
		!did_something; ++spos) {

		if (spos->get_candidate_num() == cand_to_change &&
			spos->get_score() != highest_score) {
			double cur_score = spos->get_score();
			double rand_displace = 1.01;

			// Remove the old and insert the new.
			int cn = spos->get_candidate_num();

			// So that we don't saw off the branch on which we're
			// sitting.
			ordering::iterator backup = spos;
			--backup;
			norm.erase(spos);
			spos = backup;
			--spos;
			norm.insert(candscore(cn, cur_score * rand_displace));

			// Now that we're all done, let the loop know.
			did_something = true;
		}
	}

	return (did_something);
}

///// Mono-raise-delete
///////////////////////

bool mono_raise_delete::alter_ballot(const ordering & input,
	ordering & output, size_t numcands, const std::vector<size_t> & data,
	rng & randomizer) const {

	// As in mono-raise, but we delete everything after the change.
	// Cut n paste too much?

	output.clear();

	int highest_score = output.begin()->get_score();
	size_t cand_to_change = data[0];
	bool raise = (data[1] == 1);

	// Hack because "lowering" doesn't make sense for add-delete.
	if (!raise) {
		output = input;
		return (false);
	}

	bool did_something = false;
	double rand_displace = -1;

	candscore to_add(-1, -1);

	for (ordering::const_iterator spos = input.begin();
		spos != input.end() && !did_something; ++spos) {

		if (spos->get_candidate_num() == cand_to_change) {
			int cur_score = spos->get_score();
			rand_displace = cur_score;
			if (highest_score != cur_score) {
				rand_displace = randomizer.next_int(
						cur_score,
						highest_score);
				// Always raise by at least one, unless
				// we're already at top.
				rand_displace = std::min((double)highest_score,
						rand_displace + 1);
			}

			// Set what we want to add
			to_add = candscore(spos->get_candidate_num(),
					rand_displace);

			if (rand_displace != cur_score) {
				did_something = true;
			}
		}
	}

	// Now mop up possible lower ranked.
	// TODO: Check that!
	//std::cout << std::endl;
	/*std::cout << "A: " << input.size() << std::endl;
	std::cout << "B: " << output.size() << std::endl;
	std::cout << "RD: " << rand_displace << std::endl;*/
	/*ordering::iterator limit_pos = output.begin();

	while (limit_pos->get_score() > rand_displace
			&& limit_pos != output.end())
		++limit_pos;

	std::cout << "B: " << output.size() << std::endl;
	output.erase(limit_pos, output.end());*/
	for (ordering::const_iterator xpos = input.begin(); xpos != input.end()
		&& xpos->get_score() > rand_displace; ++xpos) {
		output.insert(*xpos);
	}

	//std::cout << "BA: " << output.size() << std::endl;

	// Add ours and return.
	output.insert(to_add);

	//assert(output.size() != input.size());

	return (output.size() != input.size());
}

///// Mono-raise-random
///////////////////////

bool mono_raise_random::alter_ballot(const ordering & input,
	ordering & output, size_t numcands, const std::vector<size_t> & data,
	rng & randomizer) const {

	// Lost of cut-n-paste code from MRD here. See mono-raise-delete for
	// what this does. May have to refactor later.

	output.clear();

	int highest_score = output.begin()->get_score();
	size_t cand_to_change = data[0];
	bool raise = (data[1] == 1);

	// Could be done, after a fashion, but that wouldn't be symmetric.
	// (I.e. lower candidate and randomize everything after his new
	// position)
	if (!raise) {
		output = input;
		return (false);
	}

	bool did_something = false;
	double rand_displace = -1;

	candscore to_add(-1, -1);

	for (ordering::const_iterator spos = input.begin(); spos != input.end()
		&& !did_something; ++spos) {

		if (spos->get_candidate_num() == cand_to_change) {
			int cur_score = spos->get_score();
			rand_displace = cur_score;
			if (highest_score != cur_score)
				rand_displace = std::min((double)randomizer.
						next_int(cur_score, highest_score),
						rand_displace + 1);

			to_add = candscore(spos->get_candidate_num(),
					rand_displace);

			if (rand_displace != cur_score) {
				did_something = true;
			}
		}
	}

	// Mark those we have used, as we go down the list. The random
	// fill afterwards will consist of those that remain.
	// TODO: Handle truncation?
	std::vector<bool> used(numcands, false);
	used[to_add.get_candidate_num()] = true;

	for (ordering::const_iterator xpos = input.begin(); xpos != input.end()
		&& xpos->get_score() > rand_displace; ++xpos) {
		output.insert(*xpos);
		used[xpos->get_candidate_num()] = true;
	}

	// Add unused candidates below our candidate, in random order.
	for (size_t counter = 0; counter < used.size(); ++counter)
		if (!used[counter])
			output.insert(candscore(counter, rand_displace -
					(randomizer.next_double() + 0.01)));

	output.insert(to_add);

	return (true);
}
