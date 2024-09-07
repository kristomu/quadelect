// Ya ya QnD

// BLUESKY: Support other positional systems.

#include "cm_cfprm.h"

using namespace std;

// Public stuff

void CFPRM::init(int candidates_in, const vector<ballot_bunch> & data_in) {
	set_data(candidates_in, data_in);
	post_initialize(false);
}

CFPRM::CFPRM(int candidates_in, const vector<ballot_bunch> & data_in) {
	init(candidates_in, data_in);
}

void CFPRM::compare(const vector<bool> & first_set, const vector<bool> &
	second_set) {

	get_values(first_set, second_set, voting_record, first_score,
		second_score);
}

double CFPRM::get_first_score() {
	return (first_score);
}

double CFPRM::get_second_score() {
	return (second_score);
}

string CFPRM::get_identity(bool long_form) {
	if (long_form) {
		return ("Condorcet-Flavored Proportional Representation Method [election-methods]");
	} else	{
		return ("CFPRM");
	}
}

// ------------------ //

// Private:

void CFPRM::set_data(int candidates_in,
	const vector<ballot_bunch> & data_in) {
	candidates = candidates_in;
	voting_record = data_in;
	first_score = -1;
	second_score = -1;
}

void CFPRM::post_initialize(bool debug_in) {
	debug = debug_in;
}


// members_of_x are all buckets, members_of_x[y] returns true if y is a member
// of a.

// gets rank sum - a sort of reverse Borda.
// Now ordinary Borda.
vector<double> CFPRM::get_borda_sum(const vector<ballot_bunch> & box,
	double & num_voters) {

	vector<double> toRet(candidates, 0);

	num_voters = 0;

	int last_rank;

	vector<ballot_bunch>::const_iterator outer_pos = box.begin();
	vector<ballot_component>::const_iterator inner_pos;

	while (outer_pos != box.end()) {
		num_voters += outer_pos -> multiplier;
		inner_pos = outer_pos->ballot.begin();

		last_rank = ((outer_pos->ballot.end())-1)->rank;
		while (inner_pos != outer_pos->ballot.end()) {
			toRet[inner_pos->candidate_reference] = outer_pos->
				multiplier * (last_rank - inner_pos->rank+ 1);
			inner_pos++;
		}
		outer_pos++;
	}

	return (toRet);
}

// 1: favors A. -1: favors B. 0: tie

int CFPRM::favors_A(const vector<bool> & members_of_A, const vector<bool> &
	members_of_B, const ballot_bunch & this_ballot,
	double & r_favored) {

	// A ballot prefers set A to set B if the number of pairwise wins
	// of members of X against Y is greater than the number of pairwise
	// losses.

	// We do this by the Borda union method. Discard all candidates that
	// are not either in A or B. Then use a normal Borda count. If A's
	// Borda count is greater than B's, then A is preferred.

	// since ballots rank with least number = best, we have to start at
	// the bottom and do something like (if candidate[ballot[x]] is in
	// A or in B then if rank[ballot[x]] < rank[ballot[last in x|y]] then
	// counter++; in either case, borda[whatever set it was in] += counter.

	// Addenda: It seems using rank works just as well, so we'll do that.
	// Off with the Borda. ?

	int As_rank = 0, Bs_rank = 0, AnotB_rank = 0, BnotA_rank = 0;
	int AnotB_members = 0, BnotA_members = 0;

	int last_rank = this_ballot.ballot[0].rank;
	int rank_within_AB = 0;

	vector<ballot_component>::const_iterator pos = this_ballot.ballot.
		begin();

	if (debug) {
		cout << "[proportional:CFPRM] ";
	}

	// this is ugly. We should ideally have something like ReduceBallots
	// which removes members not in either A or B and updates the rank
	// numbers to be linear again. But for now..
	//
	// { We have that now; the rejudging of CPO-STV. TODO: Consider it.}

	if (debug) {
		cout << "Getting ready for rank: ";
	}
	while (pos != this_ballot.ballot.end()) {
		if (debug) {
			cout << pos->candidate_reference;
			if (members_of_A[pos->candidate_reference]) {
				cout << "A";
			} else {
				cout << "-";
			}
			if (members_of_B[pos->candidate_reference]) {
				cout << "B";
			} else {
				cout << "-";
			}

			cout << "\t" << flush;
		}
		if (members_of_A[pos->candidate_reference] || members_of_B
			[pos->candidate_reference]) {

			if (last_rank != pos->rank) {
				last_rank = pos->rank;
				rank_within_AB++;
			}

			if (members_of_A[pos->candidate_reference]) {
				As_rank += rank_within_AB;
			} else {
				//		cout << "BnotA member: Global rank " << pos->rank << "\tlocal rank: " << rank_within_AB << endl;
				BnotA_members++;
				BnotA_rank += rank_within_AB;
			}

			if (members_of_B[pos->candidate_reference]) {
				Bs_rank += rank_within_AB;
			} else {
				//		cout << "AnotB member: Global rank " << pos->rank << "\tlocal rank: " << rank_within_AB << endl;
				AnotB_members++;
				AnotB_rank += rank_within_AB;
			}
		}
		pos++;
	}

	if (debug) {
		cout << endl;
	}

	/*	cout << "A's rank (closer to 0 is better): " << As_rank << endl;
		cout << "B's rank (closer to 0 is better): " << Bs_rank << endl;

		cout << "AnotB rank: " << AnotB_rank << endl;
		cout << "BnotA rank: " << BnotA_rank << endl;*/

	if (debug) {
		cout << "[proportional:CFPRM] ";
	}

	if (As_rank < Bs_rank) {
		r_favored = AnotB_rank/(double)AnotB_members;
		if (debug) {
			cout << "R-Favored (A not B): " << r_favored << " (" << AnotB_rank << " / "
				<< AnotB_members << ") BnotA: " << BnotA_rank/(double)BnotA_members <<
				endl;
		}
		return (1);
	}

	if (Bs_rank < As_rank) {
		r_favored = BnotA_rank/(double)BnotA_members;
		if (debug) {
			cout << "R-Favored (B not A): " << r_favored << " (" << BnotA_rank << " / "
				<< BnotA_members << ") AnotB: " << AnotB_rank/(double)AnotB_members <<
				endl;
		}
		return (-1);
	}

	if (debug) {
		cout << "Tie: " << Bs_rank << endl;
	}
	return (0);
}

// gets the separating marker (R1 and R2 in the original CFPRM proposal) for
// generating correct weighing later. This uses rank sums directly and not
// Borda counts.

double CFPRM::get_rab_borda(const vector<double> & rank_sum,
	const vector<bool> &
	members_of_A, const vector<bool> & members_of_B,
	const double & num_voters) {

	int counter;
	int numcand = 0; // number of candidates we added to the Borda.
	// Used for averaging.

	double ab_sum = 0;

	// A - B is the set of those that are members of A but not B.

	for (int counter = 0; counter < rank_sum.size(); counter++) {
		if (members_of_A[counter] && !members_of_B[counter]) {
			numcand++;
			ab_sum += rank_sum[counter];
		}
	}

	ab_sum /= (double)(numcand*num_voters);

	if (debug) {
		cout << "RAB_SUM: " << ab_sum << endl;
	}

	return (ab_sum);
}

// this assumes the ballot favors A. If it favors B, just swap rab and rba, and
// m_o_A and m_o_B.
// { Ties?? }

int CFPRM::get_weight(const ballot_bunch & this_ballot, const double & rab,
	const vector<bool> & members_of_A, const
	vector<bool> & members_of_B) {

	// Jump onto this_ballot.ballot[rab]. Because the ballot is sorted,
	// the first candidate having a rank lower (greater number) than rab
	// must be later or at ballot[rab].

	// Then count the number of candidates ranked lower (just break when we
	// find the first, and then count all downwards; thus we don't have to
	// do an if comparison on every one), adding to "members_of_B that are
	// below" if it is a member of B.

	int barrier = (int)floor(rab);

	int only_ab_rank, last_rank;

	// We can't use the heuristic as we don't have a reduced ballot.

	vector<ballot_component>::const_iterator pos = this_ballot.ballot.
		begin();// + barrier;

	only_ab_rank = 0;
	last_rank = pos->rank;

	while (only_ab_rank <= barrier && pos != this_ballot.ballot.end()) {
		pos++;
		if (last_rank != pos->rank && (members_of_A[pos->
					candidate_reference] || members_of_B[
					pos->candidate_reference])) {
			only_ab_rank++;
			last_rank = pos->rank;
		}
	}

	if (pos == this_ballot.ballot.end()) {
		cout << "Broke by default. (barrier = " << barrier << ")"<< endl;
		return (1);
	}

	int how_many = 0;

	/*cout << "First candidate to be below is " << pos->candidate_reference
		<< " (" << (reverse.find(pos->candidate_reference))->second << ")\t " << flush;

	cout << "Inserting ";*/
	while (pos != this_ballot.ballot.end()) {
		if (members_of_B[pos->candidate_reference]) {
			//	cout << pos->candidate_reference << " " << flush;
			//	cout << "Inserted " << pos->candidate_reference << endl;
			how_many++;
		}
		pos++;
	}
	//cout << endl;

	return (how_many);
}

/*void get_values(const vector<ballot_bunch> & box, const vector<bool> &
		members_of_A, const vector<bool> & members_of_B, double &
		a_results, double & b_results /*, const vector<double> &
		rank_sum, const double & num_voters, const map<int, string> &
		reverse*v/) {*/

void CFPRM::get_values(const vector<bool> & members_of_A,
	const vector<bool> &
	members_of_B, const vector<ballot_bunch> & box,
	double & a_results, double & b_results) {

	a_results = 0;
	b_results = 0; 	//

	int favor;
	int weight;

	double r_dynamic; // average borda of subsets of (Favored set)-
	//(not favored set)

	if (debug) {
		cout << "[proportional:CFPRM] ";
		for (int i = 0; i < members_of_A.size(); i++) {
			cout << i;
			if (members_of_A[i]) {
				cout << "A";
			} else {
				cout << "-";
			}
			if (members_of_B[i]) {
				cout << "B";
			} else {
				cout << "-";
			}
			cout << "\t";
		}
		cout << endl;
	}

	for (int counter = 0; counter < box.size(); counter++) {
		favor = favors_A(members_of_A, members_of_B, box[counter],
				r_dynamic);
		if (debug) {
			cout << "[proportional:CFPRM] " << counter << "\t";
		}
		if (favor == 0) {
			if (debug) {
				cout << "Tie." << endl;
			}
			continue; // tie.
		}
		// A favored
		if (favor == 1) {
			weight = get_weight(box[counter], r_dynamic,
					members_of_A, members_of_B);
			if (debug) cout << "Favors A: Weight: " << weight <<
					"\t" << box[counter].multiplier << endl;

			a_results += weight*box[counter].multiplier;
		} else {
			weight = get_weight(box[counter], r_dynamic,
					members_of_B, members_of_A);
			if (debug) cout << "Favors B: Weight: " << weight
					<< "\t" << box[counter].multiplier << endl;
			b_results += weight*box[counter].multiplier;
		}
	}
}

