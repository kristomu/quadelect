// Code for reducing CPO-STV ballots. Uses James-Green Armytage's rules:

// Something's wrong here, but at least it doesn't crash. I suspect split_vote.
// {FIXED}

// If we need to surplus transfer R, and R's surplus (above quota q) is r, then
// 		if the ballot is of the form R > B or A = R > B
// 			becomes B or A > B with strength
// 				original strength * r/(q+r).
// 		if the ballot is of the form A > R > B or A > R = B,
// 			becomes A > B with strength *= r/(q+r), and
// 			A with strength orig * 1-(r/(q+r)).
//		if the ballot is of the form A > B > R, becomes A > B
//		if the ballot is of the form R, it is removed.
//
//	(Afterwards, add [quota]: R)
//		(Amplification: first count the number we remove, then
//		 r_new = r - [how many], then the new strengths as above
//		 are * r_new/(q+r_new) and 1-r_new/(q+r_new) respectively.
//		 	This is not in JGA's rules.)
//
//	Or more strictly,
//		if R is in the ballot's list,
//			if it has rank equal to the last rank, just remove it.
//				(Handles R, A = B = R)
//			if it has rank equal to the first rank,
//				remove it and change strength
//					and if it was the only one of first
//					rank, shift all ranks below up one
//					step.
//			otherwise
//				insert a new entry of all with ranks above
//				the current, and set that one's strength to
//				the old multiplied by 1-r/(q+r), remove those
//				higher ranks and R itself from the current and
//				multiply its strength by r/(q+r), and finally
//				rejudge it.
//
//	Rejudging: given a list that starts at rank p, set that rank to 0 and
//	change all ranks below to compensate.

#include "cm_cpo_stv.h"

// TODO: Move first_score, second_score to ComparisonMethod.

// return codes from a ballot split

// -- PUBLIC --


void cm_CPO_STV::init(int candidates_in,
	const vector<ballot_bunch> & data_in) {
	set_data(candidates_in, data_in);
	post_initialize(false);

	// set quota and quota_seats to -1 so that compare will cache the quota
	// next.
	quota = -1;
	quota_seats = -1;
}

cm_CPO_STV::cm_CPO_STV(int candidates_in,
	const vector<ballot_bunch> & data_in) {
	init(candidates_in, data_in);
}

void cm_CPO_STV::compare(const vector<bool> & first_set,
	const vector<bool> &
	second_set) {

	single_cpo_outcome(first_set, second_set, voting_record, first_score,
		second_score);
}

double cm_CPO_STV::get_first_score() {
	return (first_score);
}

double cm_CPO_STV::get_second_score() {
	return (second_score);
}

string cm_CPO_STV::get_identity(bool long_form) {
	if (long_form) {
		return ("STV by Comparison of Pairwise Outcomes [Tideman]");
	} else	{
		return ("CPO-STV");
	}
}


// ------- PRIVATE ------

void cm_CPO_STV::set_data(int candidates_in, const vector<ballot_bunch> &
	data_in) {

	candidates = candidates_in;
	voting_record = data_in;

	first_score = -1;
	second_score = -1;
}

void cm_CPO_STV::post_initialize(bool debug_in) {
	// get quota
	// THIS IS WRONG. Need seats input! TODO
	double quota = get_quota(voting_record, candidates);

	debug = debug_in;
}


// These should go into another class

double cm_CPO_STV::get_multiplier_sum(const vector<ballot_bunch> & box) {
	// first count the number of voters, or the sum of their collective
	// strength.

	double toRet = 0;

	for (int counter = 0; counter < box.size(); counter++) {
		toRet += box[counter].multiplier;
	}

	return (toRet);
}

double cm_CPO_STV::get_hare_quota(const vector<ballot_bunch> & box,
	int seats) {

	return (get_multiplier_sum(box) / (double)seats);
}

double cm_CPO_STV::get_hagenbach_quota(const vector<ballot_bunch> & box,
	int seats) {
	return (get_multiplier_sum(box) / (double)(seats+1));
}

// Recommended by Tideman
double cm_CPO_STV::get_quota(const vector<ballot_bunch> & box, int seats) {
	return (get_hagenbach_quota(box, seats));
}

// And now for our regularly scheduled class.








ballot_bunch cm_CPO_STV::rejudge(const ballot_bunch & input,
	int candidate_to_remove) {
	// quite easy. Also removes R.

	ballot_bunch target;
	target.multiplier = input.multiplier;
	target.ballot.resize(0);

	if (input.ballot.size() == 0) {
		return (target);
	}

	// First dump all but the candidate we want to remove.
	int counter;

	for (counter = 0; counter < input.ballot.size(); counter++)
		if (input.ballot[counter].candidate_reference !=
			candidate_to_remove) {
			target.ballot.push_back(input.ballot[counter]);
		}

	// empty ballot? OK
	if (target.ballot.size() == 0) {
		//	cout << "[rejudge] Got empty ballot. " << endl;
		return (target);
	}

	// Then clear up any jumps greater than 1. (e.g A > B > C with B
	// 	removed.)

	int offset = target.ballot[0].rank; // becomes 0
	int old_rank = 0;

	//cout << "[rejudge] Offset is " << offset << endl;
	for (counter = 0; counter < target.ballot.size(); counter++) {
		target.ballot[counter].rank -= offset;
		// if there's a > 1 jump, make it 1.
		// TODO: Beware, test this.
		if (target.ballot[counter].rank - old_rank > 1) {
			offset += (target.ballot[counter].rank - old_rank) - 1;
			target.ballot[counter].rank = old_rank + 1;
		}
		old_rank = target.ballot[counter].rank;
	}

	return (target);
}

// done to remove candidates that appear in neither comparison. This allows
// transfers to work correctly, but it's costly. TODO: Fix.

void cm_CPO_STV::mass_rejudge(vector<ballot_bunch> & box,
	const vector<bool> & to_remove) {

	for (int counter = 0; counter < to_remove.size(); counter++) {
		if (!to_remove[counter]) {
			continue;
		}

		for (int sec = 0; sec < box.size(); sec++) {
			box[sec] = rejudge(box[sec], counter);
		}
	}
}


/*#define LAST_RANK 1
#define FIRST_RANK -1
#define NOT_MENTIONED 2*/

split_status cm_CPO_STV::split_ballot(int separator_candidate,
	vector<ballot_component> & higher,
	vector<ballot_component> & lower_equ,
	const vector<ballot_component> & to_divide) {

	// divides to_divide into [rank closer to 0 than separator] and
	// [rank equal or further away from 0 than separator].

	// first find the rank of the separator, and also highest and lowest
	// rank. Returns -1 if the separator is of rank 0, and 1 if the
	// separator is of lowest rank. Returns 2 if not mentioned.

	int counter;
	int lowest_rank = (to_divide.end()-1)->rank;
	int separator_pos = -1;

	for (counter = 0; counter < to_divide.size() && separator_pos == -1;
		counter++)
		if (to_divide[counter].candidate_reference ==
			separator_candidate) {
			separator_pos = counter;
		}

	//z/ain/-Abrin/

	if (separator_pos == -1) {
		return (NOT_MENTIONED);
	}
	if (to_divide[separator_pos].rank == 0) {
		return (FIRST_RANK);
	}
	if (to_divide[separator_pos].rank == lowest_rank) {
		return (LAST_RANK);
	}

	// okay, easy now. Should use lists.

	higher.resize(0);
	lower_equ.resize(0);

	int separator_rank = to_divide[separator_pos].rank;
	// higher
	for (counter = 0; counter < separator_pos && to_divide[counter].rank
		!= separator_rank; counter++) {
		higher.push_back(to_divide[counter]);
	}
	// die reste
	for (; counter < to_divide.size(); counter++) {
		lower_equ.push_back(to_divide[counter]);
	}

	// Optimization comes later.
	return (SPLIT);
}

// TODO: Actually write this.
// if rank is either first or last, just rejudge.

// also need a function to remove empty ballots. Do that by swapping last with
// it, then just trunc.
void cm_CPO_STV::transfer(int candidate, double votes,
	vector<ballot_bunch> &
	local_box) {
	// votes is number of votes for the candidate.

	ballot_bunch to_insert; // for when the transferring vote is
	// lodged between two others.

	double proportion = (votes-quota)/votes;

	vector<ballot_component> higher, lower;

	int original_ballots = local_box.size(); // so that push_back doesn't
	// distort stuff.

	int counter;

	for (counter = 0; counter < original_ballots; counter++) {
		split_status retval = split_ballot(candidate, higher, lower,
				local_box[counter].ballot);


		switch (retval) {
			case NOT_MENTIONED: break; // no change needed
			// case 0: higher has strength 1-proportion,
			// lower strength proportion. We use clever
			// case fallthroughs.
			case SPLIT: to_insert.ballot = higher;
				to_insert.multiplier = local_box[counter].
					multiplier * (1.0-proportion);
				local_box.push_back(to_insert);
				local_box[counter].ballot = lower;

			case FIRST_RANK: local_box[counter].multiplier *=
					proportion;

			case LAST_RANK: local_box[counter] = rejudge(local_box[counter],
						candidate);
				break;
			default: break;
		}
	}

	// TODO: Add "remove blank ballots" by swapping here.

	int last = local_box.size() - 1;

	for (counter = 0; counter <= last; counter++) {
		while (local_box[counter].ballot.size() == 0 && counter < last) {
			swap(local_box[counter], local_box[last--]);
		}
	}

	// last is now either the last non-empty ballot or the first empty,
	// depending on how the system exited the loop.

	if (local_box[last].ballot.size() > 0) {
		last++;
	}
	if (debug) {
		cout << last << "\t\t" << local_box.size() << endl;
	}
	if (last != local_box.size()) {
		local_box.resize(last+1);
	}

	if (debug) {
		cout << "We who have lasted aeons," << endl;
	}

	// finally, add the quota [only works for plurality. Complete LATER!]
	to_insert.ballot.resize(1);
	if (debug) {
		cout << " We look to the future." << endl;
	}
	to_insert.ballot[0].rank = 0;
	to_insert.ballot[0].candidate_reference = candidate;
	to_insert.multiplier = quota;

	local_box.push_back(to_insert);

	if (debug) {
		cout << " We live today" << endl;
		cout << local_box[local_box.size()-1].ballot[0].
			candidate_reference << endl;

		cout << " Or perhaps tomorrow." << endl;

		for (counter = 0; counter < local_box.size(); counter++) {
			cout << local_box[counter].multiplier << "\t";
			for (int sec = 0; sec < local_box[counter].
				ballot.size(); sec++)
				cout << local_box[counter].ballot[sec].
					candidate_reference << " ";
			cout << endl;
		}
	}
}

// Eurgh! But I don't like 2^n!

void cm_CPO_STV::single_cpo_outcome(const vector<bool> & first_set, const
	vector<bool> & second_set, const vector<ballot_bunch> & box,
	double & first_outcome, double & second_outcome) {

	// We might be able to speed this up by using candidate-buckets,
	// where ballot_reference[x] lists all those with x in them. But
	// bwah. (This would also mean changing box to a list, and box.ballot
	// to a list too)

	// 1. Make a copy of the box because we're going to alter it.
	// 2. Construct the exclusion set (members in neither) and intersection
	// 	set (members in both)
	// 2. Count plurality votes of the intersection set.
	// 3. Transfer votes of all above quota.
	// 	3.2. If there were any, go to 2.
	// 4. From the plurality votes, sum up the counts for first and second
	// 	set candidates. The former is first_outcome, the latter is
	// 	second_outcome.
	// 5. Return.

	vector<ballot_bunch> local_box = box;

	vector<bool> exclusion_set(candidates, false);
	vector<bool> intersection(candidates, false);

	int counter;

	int seats = 0;

	// create sets
	for (counter = 0; counter < candidates; counter++) {
		if (!first_set[counter] && !second_set[counter]) {
			exclusion_set[counter]= true;
		}
		if (first_set[counter] && second_set[counter]) {
			intersection[counter] = true;
		}
		if (first_set[counter]) {
			seats++;
		}
	}

	if (quota == -1 || quota_seats != seats) {
		quota = get_quota(box, seats);
		quota_seats = seats;
	}

	bool transferred = true; // first time so that it'll run at least
	// once.

	vector<positional_count> plurality_score;

	// remove what's in neither
	mass_rejudge(local_box, exclusion_set);

	// TODO: print ballot here for debug purposes.

	while (transferred) {
		if (debug) {
			cout << "And again, Sam!" << endl;
		}
		plurality_score = positional_raw(candidates, local_box,
				plurality_function, exclusion_set, true);
		if (debug) {
			cout << "Done, dude." << endl;
		}
		transferred = false;
		for (counter = 0; counter < candidates; counter++) {
			if (plurality_score[counter].score > quota &&
				intersection[counter]) {
				if (debug) {
					cout << plurality_score[counter].candidate_reference <<
						" above quota with a score of " << plurality_score[counter].score << endl;
				}
				transfer(plurality_score[counter].
					candidate_reference,
					plurality_score[counter].score,
					local_box);
				transferred = true;
			}
		}
	}

	// we've completed the transfers, so count one final time to get the
	// scores of all candidates, not just the transferable ones.

	plurality_score = positional_raw(candidates, local_box,
			plurality_function, exclusion_set, false);

	first_outcome = 0;
	second_outcome = 0;

	for (counter = 0; counter < candidates; counter++) {
		int cand = plurality_score[counter].candidate_reference;
		if (debug) {
			cout << "Candidate " << cand << " has a score of " <<
				plurality_score[counter].score << endl;
		}
		if (first_set[cand]) {
			if (debug) {
				cout << "\t That candidate is a member of the first set." << endl;
			}
			first_outcome += plurality_score[counter].score;
		}
		if (second_set[cand]) {
			if (debug) {
				cout << "\t That candidate is a member of the second set." << endl;
			}
			second_outcome += plurality_score[counter].score;
		}
	}
}
