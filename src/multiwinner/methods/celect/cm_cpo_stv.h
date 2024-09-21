#ifndef __CM_CPO_STV
#define __CM_CPO_STV
// Code for reducing CPO-STV ballots. Uses James-Green Armytage's rules:

// Something's wrong here, but at least it doesn't crash. I suspect split_vote.

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

#include "ballot_struct.h"
#include "plurality.h"

#include <iostream>
using namespace std;

#include "comparison.h"

// TODO: Move first_score, second_score to ComparisonMethod.

// return codes from a ballot split
typedef enum { LAST_RANK, FIRST_RANK, NOT_MENTIONED, SPLIT } split_status;

class cm_CPO_STV : public ComparisonMethod<vector<ballot_bunch> > {

	private:
		bool debug; // TODO: Add another constructor to set this
		int candidates;

		double first_score;
		double second_score;

		double quota;
		int quota_seats;

		vector<ballot_bunch> voting_record;

		void set_data(int candidates_in, const vector<ballot_bunch> &
			data_in);
		void post_initialize(bool debug_in);

		// internal functions

		// Handles ballot rank discontinuities.
		ballot_bunch rejudge(const ballot_bunch & input, int
			candidate_to_remove);
		void mass_rejudge(vector<ballot_bunch> & box,
			const vector<bool> & to_remove);

		// splits ballots according to position of a specific candidate.
		// This is used for fractional STV.
		split_status split_ballot(int separator_candidate,
			vector<ballot_component> & higher,
			vector<ballot_component> & lower_equ,
			const vector<ballot_component> & to_divide);

		// used for quota calculations
		double get_multiplier_sum(const vector<ballot_bunch> & box);
		double get_hare_quota(const vector<ballot_bunch> & box,
			int seats);
		double get_hagenbach_quota(const vector<ballot_bunch> & box,
			int seats);
		double get_quota(const vector<ballot_bunch> & box, int seats);

		// Transferring.
		void transfer(int candidate, double votes, vector<ballot_bunch>
			& local_box);

		void single_cpo_outcome(const vector<bool> & first_set, const
			vector<bool> & second_set, const
			vector<ballot_bunch> & box, double &
			first_outcome, double & second_outcome);

	public:
		void init(int candidates_in, const vector<ballot_bunch> &
			data_in);
		cm_CPO_STV(int candidates_in, const vector<ballot_bunch> &
			data_in);

		void compare(const vector<bool> & first_set, const vector<bool>
			& second_set);

		double get_first_score();
		double get_second_score();
		string get_identity(bool long_form);
};

#endif
