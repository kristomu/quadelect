// Class for the reversal symmetry two-test: A method fails reversal symmetry
// if there's a ballot set for which "most liked" (everybody votes in the order
// specified) gives the same result/winner as "most hated" (everybody votes in
// reverse order).

// This is not a good thing for multiwinner elections. See WDS.

#ifndef _TWOTEST_RSYM
#define _TWOTEST_RSYM

#include "../two_tests.h"


class test_reversal_symmetry : public twotest {

	private:
		bool winner_only, permit_ties;
		ordering_tools otools;

	protected:
		// No data needed, hence we don't need to redef
		// generate_aux_data.
		std::pair<bool, election_t> rearrange_ballots(
			const election_t & input, size_t numcands,
			const std::vector<size_t> & data) const;

		bool applicable(const ordering & check,
			const std::vector<size_t> & data, bool orig) const;

		bool pass_internal(const ordering & original, const ordering &
			modified, const std::vector<size_t> & data,
			size_t numcands) const;

		std::string explain_change_int(const std::vector<size_t> & data,
			const std::map<size_t, std::string> & cand_names) const;

	public:

		std::string name() const;

		test_reversal_symmetry(bool winner_only_in,
			bool permit_ties_in) {
			winner_only = winner_only_in;
			permit_ties = permit_ties_in;
		}

		test_reversal_symmetry() {
			winner_only = true;
			permit_ties = false;
		}

};

#endif
