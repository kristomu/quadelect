#pragma once

#include "../relative_criterion.h"

// Relative criterion (not really a fitting name) for expressing the
// elimination of candidates. The "criterion" is used for expressing
// independence criteria like ISDA.

class elimination_util_const : public relative_criterion_const {
	private:
		// elimination_spec[x] is -1 if the xth candidate is eliminated,
		// otherwise the number that candidate corresponds to after, e.g.
		// eliminating B in a  4cddt gives {0, -1, 1, 2};
		std::vector<int> elimination_spec;

	protected:
		bool permissible_transition(
			const std::vector<int> & before_permutation,
			const std::vector<int> & after_permutation) const;

		virtual bool is_valid_numcands_combination() const {
			return numcands_before > numcands_after; }

	public:
		// Eliminates the latter candidates. (i.e. numcands_before_in = 5,
		// after = 3, keeps ABC and eliminates D and E).
		elimination_util_const(size_t numcands_before_in, 
			size_t numcands_after_in);

		elimination_util_const(std::vector<int> elimination_spec_in);

		// Show that this is a utility class by demanding neither
		// no-harm nor no-help.
		bool no_harm() const { return false; }
		bool no_help() const { return false; }

		std::string name() const { return "UTIL: Elimination"; }
};
