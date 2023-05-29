// Base class for monotonicity problems. Monotonicity criteria involve doing
// something that seems to be beneficial to a candidate; the method then fails
// if doing so makes the candidate lose (if the test is winner-only) or lowers
// his rank (if the test is full-rank), or conversely, if doing something that
// seems harmful to the candidate makes him win.

// Possibly: Alter these to make a single change only, since (all? most?)
// monotonicity criteria violations can be reduced to this. E.g. mono-raise
// is equivalent to someone swapping two candidates (raising the lower one).
// Just have the mono-raise function ignore p and always raise a random
// candidate on the first ballot -- if the right boolean is set. If the ballot
// is compressed, it should decrement the count by 1 and add another of that
// sort.

#ifndef _TWOTEST_MONO
#define _TWOTEST_MONO

#include "../../../tools/ballot_tools.h"
#include "../../../random/random.h"
#include "../../two_tests.h"

#include <numeric>
#include <iterator>
#include <vector>

using namespace std;

// Abstract class for monotonicity type criteria. The only virtual and complex
// alterable function is the one that modifies a given ballot.

class monotonicity : public twotest {

	private:
		// If this is true, we only care if, for mono-raise, ranking A
		// lower makes A win (or ranking A higher makes A not win). If
		// false, we care even if A just moved "the other way" in the
		// social order, but didn't become winner in any of them.
		bool check_winner_only, permit_ties;

		string explain_change_int(const vector<size_t> & data,
			const map<size_t, string> & cand_names) const;

	protected:

		ordering_tools otools;

		ordering::const_iterator find_cand(const ordering & to_search,
			size_t candnum) const;

		// Whether or not it's possible to turn the "raise" test into
		// a "lower" test (only possible for straightforwards
		// mono-raise).
		virtual bool allows_lowering() const = 0;

		virtual string basename() const = 0;

		// If this returns false, it has copied input to output.
		// Otherwise, output is an appropriately modified version
		// of input.
		virtual bool alter_ballot(const ordering & input,
			ordering & output, size_t numcands,
			const vector<size_t> & data,
			rng & randomizer) const {
			output = input; return (false);
		}

		// For mono-add-top, etc. If it returns false, nothing was done,
		// otherwise some ballots have been added to list<ballot_group>.
		virtual bool add_ballots(const vector<size_t> & data,
			rng & randomizer,
			list<ballot_group> & input,
			double total_weight, size_t numcands) const {
			return (false);
		}

	public:
		// The data is first, an integer constituting the candidate
		// to alter, then either 0 (lower) or 1 (raise), then a list
		// of ballot numbers to modify. We might also need a seed for
		// how far we're going to move the candidate in each ballot.
		vector<size_t> generate_aux_data(const list<ballot_group> & input,
			size_t numcands) const;

		// Modify data so as to prioritize a certain candidate (usually
		// raise him). Returns modified data.
		vector<size_t> set_candidate_to_alter(vector<size_t> data,
			size_t alter_to) const {
			data[0] = alter_to;
			return data;
		}

		pair<bool, list<ballot_group> > rearrange_ballots(
			const list<ballot_group> & input,
			size_t numcands, size_t number_to_add,
			const vector<size_t> & data) const;


		pair<bool, list<ballot_group> > rearrange_ballots(
			const list<ballot_group> & input,
			size_t numcands,
			const vector<size_t> & data) const;

		// Any are applicable, though ternary may reply INAPP if
		// the data is so that no ballot is altered.
		bool applicable(const ordering & check,
			const vector<size_t> & data, bool orig) const;

		bool pass_internal(const ordering & original, const ordering &
			modified, const vector<size_t> & data,
			size_t numcands) const;

		// Ternary?

		monotonicity(bool winner_only_in, bool permit_ties_in) {
			check_winner_only = winner_only_in;
			permit_ties = permit_ties_in;
		}

		monotonicity() {
			check_winner_only = true;
			permit_ties = false;
		}

		string name() const;

		bool winner_only() const {
			return (check_winner_only);
		}
};

#endif
