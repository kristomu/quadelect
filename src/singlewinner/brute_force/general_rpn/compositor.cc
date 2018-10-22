// This class handles the composition of different gen_custom_functions into
// an election method for k candidates. We do the composition by relying on a
// concept called a Copeland scenario.

// Let a Copeland matrix c[A,B] be defined as
//		c[A, B] = 1		if A beats B pairwise
//		c[A, B] = 0		if B beats A pairwise
// (We don't handle ties as they occur comparatively rarely.)

// Two elections have the same Copeland scenario if they're associated with
// the same Copeland matrix, or if it's possible to permute the candidate
// names of one election to get the same Copeland matrix as the other 
// election.

// (Formally speaking, "has the same Copeland scenario" is an equivalence
// relation over elections with no pairwise ties, and we want to specify
// a gen_custom_function for each equivalence class in order to create an
// election method.)

// For speed, we use a k!-long std::vector<double> to represent a concrete
// election. The vector gives the number of voters who voted each of the
// k! possible rankings in a k-candidate election.

// The class handles the following functions (might be separated later):
//		- Determining the equivalence classes for a k-candidate election
//		- Taking an election (in vector format) and relabeling it either
//			to be a particular scenario or have a particular candidate
//			as candidate 0, and returning what scenario and candidate
//			permutation this corresponds to.

#include "../../../tools.h"

#include <set>
#include <string>
#include <vector>
#include <numeric>
#include <algorithm>

#include <iostream>
#include <iterator>

// This class handles Copeland scenarios and goes from pairwise matrices
// to scenario booleans. Note that < does not implement the equivalence
// relation above, but directly distinguishes scenarios.

// There are two forms of representing the scenario: long form, which is
// the Copeland matrix, and short form, which collapses the upper diagonal
// onto a linear boolean vector, e.g. the 4-candidate version lists the
// Copeland elements A>B A>C A>D B>C B>D C>D in order. The scenario class
// uses long form internally, as that is easier to permute.

// TODO: Use short form instead? We need some form of conversion in any
// case: either long->short when enumerating scenarios, or short->long
// when permuting.

class copeland_scenario {
	private:
		std::vector<std::vector<bool> > scenario;
		int number_of_candidates;

		std::vector<bool> copeland_matrix_to_short_form(const
			std::vector<std::vector<bool> > & copeland_matrix) const;

		std::vector<std::vector<bool> > short_form_to_copeland_matrix(const
			std::vector<bool> & short_form, int numcands) const;

		std::vector<bool> int_to_vbool(int numcands) const;

		// Produces a Copeland matrix where the candidates have been
		// reordered according to order. See the implementation for more
		// information.
		std::vector<std::vector<bool> > permute_candidates(const 
			std::vector<std::vector<bool> > & copeland_matrix, const
			std::vector<int> & order) const;

	public:

		bool operator<(const copeland_scenario & other) const {
			return scenario < other.scenario;
		}

		std::vector<bool> get_short_form() const {
			return copeland_matrix_to_short_form(scenario);
		}

		std::string to_string() const;

		copeland_scenario(const std::vector<std::vector<bool> > & 
			copeland_matrix) {

			number_of_candidates = copeland_matrix.size();
			scenario = copeland_matrix;
		}

		copeland_scenario(const std::vector<bool> & short_form,
			int numcands) {

			number_of_candidates = numcands;
			scenario = short_form_to_copeland_matrix(short_form,
				number_of_candidates);
		}

		copeland_scenario(int numcands) : 
			copeland_scenario(std::vector<bool>((numcands-1)*numcands/2, 
				false), numcands) {}

		// Default to number of candidates = 1 for maps and such.
		// Such a default is useless for any real work, so remember to
		// use a proper constructor.
		copeland_scenario() : copeland_scenario(1) {}

		void permute_candidates(std::vector<int> & permutation) {
			scenario = permute_candidates(scenario, permutation);
		}

		// For enumeration purposes
		copeland_scenario & operator++();

		copeland_scenario operator++(int throwaway) {
			copeland_scenario unincremented(*this);
			operator++();
			return unincremented;
		}

		bool operator==(const copeland_scenario & other) {
			return scenario == other.scenario;
		}

		bool operator!=(const copeland_scenario & other) {
			return scenario != other.scenario;
		}
};

std::vector<std::vector<bool> > 
	copeland_scenario::short_form_to_copeland_matrix(const 
	std::vector<bool> & short_form, int numcands) const {
	// The scenario vector is a 6-bit one with contests in order
	// A>B A>C A>D B>C B>D C>D
	// True means the first one in the list wins, false means the
	// second.

	// Hard-coded because I can't be bothered.

	std::vector<std::vector<bool> > copeland_matrix(numcands, 
		std::vector<bool>(numcands, false));

	int scenario_ctr = 0;
	for (int i = 0; i < numcands; ++i) {
		for (int j = i+1; j < numcands; ++j) {
			copeland_matrix[i][j] = short_form[scenario_ctr];
			copeland_matrix[j][i] = !copeland_matrix[i][j];
			++scenario_ctr;
		}
	}

	return copeland_matrix;
}

std::vector<bool> copeland_scenario::copeland_matrix_to_short_form(const
	std::vector<std::vector<bool> > & copeland_matrix) const{
	
	int numcands = copeland_matrix.size();

	std::vector<bool> short_form;

	for (int i = 0; i < numcands; ++i) {
		for (int j = i+1; j < numcands; ++j) {
			short_form.push_back(copeland_matrix[i][j]);
		}
	}

	return short_form;
}

// Permute the Copeland matrix into what it would be if we relabeled the
// candidates in an election and then inferred the Copeland matrix from
// that election.

// Order is a vector containing the ballot index of the candidate who's
// to become the nth. For instance,
// order = {3, 2, 0, 1} means
// what will be the first candidate (A) in the output is D (#3) in the input
// second candidate (B) in the output is C (#2) in the input
// third candidate (C) in the output is A (#0) in the input,
// and fourth candidate (D) in the output is B (#1) in the input.

std::vector<std::vector<bool> > copeland_scenario::permute_candidates(const 
	std::vector<std::vector<bool> > & copeland_matrix, const
	std::vector<int> & order) const {

	std::vector<std::vector<bool> > out_cm(copeland_matrix.size(),
		std::vector<bool>(copeland_matrix.size(), false));

	for (int i = 0; i < copeland_matrix.size(); ++i) {
		for (int j = 0; j < copeland_matrix.size(); ++j) {
			out_cm[i][j] = copeland_matrix[order[i]][order[j]];
		}
	}

	return out_cm;
}

std::string copeland_scenario::to_string() const {
	std::string outstr = "";
	for (bool x: copeland_matrix_to_short_form(scenario)) {
		if (x) {
			outstr += "T";
		} else {
			outstr += "F";
		}
	}
	return outstr;
}

copeland_scenario & copeland_scenario::operator++() {
	// Incrementing a scenario consists of treating its short form as
	// an integer, then incrementing it, and turning that back into its
	// short form. There's wraparound, so incrementing TTTTT...T turns
	// it back into FFFF...F.

	std::vector<bool> short_form = copeland_matrix_to_short_form(scenario);

	// Increment the short form. https://stackoverflow.com/questions/28060919

    for (std::vector<bool>::reverse_iterator pos = short_form.rbegin();
    		pos != short_form.rend(); ++pos) {

		if (!*pos) {
			*pos = true;
			break;
		}
		*pos = false;
	}

	// And put it back
	scenario = short_form_to_copeland_matrix(short_form, 
		number_of_candidates);

	return *this;
}

// To find every permutation as well as how we can get from one to the 
// other, we do the following:
//	- For every possible scenario:
//		- Go through every way of permuting the candidates.
//			If any fit, then this scenario can be reduced to a nonderived 
//			one we've already seen before: record that it is equivalent to
//			the other scenario with the given permutation of candidates. 
//		- If there is no such reduction, mark the current scenario as
//			nonderived so future scenarios can be checked against it.

// (TODO: Rewrite the stuff below to take into account that we can have
// multiple permutations. We will need all of them if we're to check
// monotonicity etc later.)

// We also need to find out how we can calculate every candidate's score
// given that we're in a particular scenario. The way we do this is to
// permute the candidates so that the candidate we want to find the score of
// is A (candidate 0), since each gen_custom_function gives the score from
// the point of view of A. A scenario might have multiple permutations that
// map to the same non-derived scenario, but by transitivity, it can't map
// to more than one non-derived scenario. To find out which it is, once the 
// step above is done, we do the following:
//	- For every possible scenario:
//		- For every candidate k from 1..n
//			- Determine all permutations that make candidate k into
//			  candidate 0 and that gets us to a non-derived scenario.
//			- Record the first in a map for (scenario, k). If there's a
//			  second, signal error.

// Because there should be only one permutation per candidate, we can
// compactly store both the derived to non-derived mapping and the
// candidate rotation mapping in a single map. If the scenario is derived,
// then its mapping for candidate 0 (i.e. what to use if we want the score
// for the first candidate) can point at the non-derived scenario it is
// derived from.

// --- //

// Gives every permutation where candidate cand is relabeled to the first
// candidate.
std::vector<std::vector<int> > all_permutations_centered_on(int cand, 
	int numcands) {

	// Create the first such permutation
	std::vector<int> perm(numcands);
	std::iota(perm.begin(), perm.end(), 0);
	std::swap(perm[cand], perm[0]);
	sort(perm.begin()+1, perm.end());

	std::vector<std::vector<int> > perms;

	// and go through every subsequent permutation of the remaining
	// candidates.
	do {
		perms.push_back(perm);
	} while (std::next_permutation(perm.begin()+1, perm.end()));

	perms.push_back(perm);

	return perms;
}

// After permuting according to cand_permutation, we end up in
// to_scenario.

struct isomorphism {
	std::vector<std::vector<int> > cand_permutations;
	copeland_scenario to_scenario;
	bool derived;
};

std::map<copeland_scenario, isomorphism> get_derived_scenario_reductions(
	int numcands) {

	std::map<copeland_scenario, isomorphism> reductions;

	copeland_scenario base_scenario(numcands), cur(numcands);
	int i = 0;

//	- For every possible scenario:
//		- Go through every way of permuting the candidates.
//			If any fit, then this scenario can be reduced to one we've
//			already seen before: record that it is equivalent to the other 
//			scenario with the given permutation of candidates. Mark the 
//			scenario as derived (i.e. reducible to something we've seen
//			before).

	std::vector<int> permutation(numcands);
	isomorphism cur_reduction;

	// For every possible scenario
	do {
		std::iota(permutation.begin(), permutation.end(), 0);
		bool derived = false, has_populated_isomorphism = false;

		cur_reduction.cand_permutations.clear();
		std::cout << std::endl;

		// For every way of permuting the candidates
		do {

			// Must be from the perspective of A! Fix this later...
			if (permutation[0] != 0) continue;
			// Create permuted scenario
			copeland_scenario permuted = cur;
			permuted.permute_candidates(permutation);

			/*std::cout << "With cur = " << cur.to_string() << " trying " << permuted.to_string() << "\t";
			if (reductions.find(permuted) != reductions.end()) { std::cout << "exists "; }
			if (reductions.find(permuted) != reductions.end() && reductions.find(permuted)->second.derived) { std::cout << "derived";}
			std::cout << std::endl;*/

			// Check if we've seen a nonderived that matches it.
			if (reductions.find(permuted) != reductions.end() && 
				!reductions.find(permuted)->second.derived) {
				// We've already seen it, so..

				// Ensure my transitivity hunch is correct.
				assert (!has_populated_isomorphism || 
					reductions.find(permuted)->second.to_scenario ==
					cur_reduction.to_scenario);

				// Add the current permutation to the list of such.
				derived = true;
				has_populated_isomorphism = true;
				cur_reduction.to_scenario = permuted;
				cur_reduction.cand_permutations.push_back(permutation);
				cur_reduction.derived = true;

				std::cout << cur.to_string() << " is derived" << "\t";
				std::copy(permutation.begin(), permutation.end(),
					std::ostream_iterator<int>(std::cout, " "));
				std::cout << "\t" << permuted.to_string() << std::endl;
				
			}
		} while (std::next_permutation(permutation.begin(), 
			permutation.end()));

		/*std::cout << " Out of permutation loop " << std::endl;*/

		// If it's derived, add the reduction to the list.
		if (derived) {
			reductions[cur] = cur_reduction;
		} else {
			// If it's non-derived, mark it as such by making it isomorphic
			// only to itself.
			cur_reduction.to_scenario = cur;
			std::iota(permutation.begin(), permutation.end(), 0);
			cur_reduction.cand_permutations.push_back(permutation);
			cur_reduction.derived = false;
			reductions[cur] = cur_reduction;
			std::cout << cur.to_string() << " is nonderived" << std::endl;
		}

	} while (++cur != base_scenario);

	return reductions;
	
}

/*std::set<copeland_scenario> get_nonderived_scenarios(int numcands) {
}
*/

// What's next?
// ballot_group --> factorial format vector
// list<ballot_group> --> factorial format vector
// list<ballot_group> --> list<ballot_group> permuting candidate names

// We may need the inverse of kth_permutation for this. 

// Those give us a way of generating a random election and getting the
// vote vectors for all candidates (this will necessarily be a part of
// any future election_method wrapper anyway).

// After this, we can then construct monotonicity pairs and winnow in
// parallel as shown in autocloneproofing.txt


main() {
	std::map<copeland_scenario, isomorphism> foo = 
	get_derived_scenario_reductions(4);
}


/*
class compositor {
	private:
		int number_candidates;

};*/
