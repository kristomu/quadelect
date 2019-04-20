#pragma once

#include "../direct_relative_criterion.h"

// Relative criterion (not really a fitting name) for expressing the
// elimination of candidates. The "criterion" is used as part of
// independence criteria like ISDA.

// This "relative criterion", interpreted as one, says "if a given candidate
// is eliminated, that should not harm/help A". The relevance as a component
// of ISDA is that ISDA is essentially: "if someone outside the Smith set is
// eliminated, then that should not harm/help A". Similarly for, say, IPDA,
// we have "if someone who's Pareto-dominated by someone else is eliminated,
// then that shouldn't help/harm A". Because the independence criteria depend
// on just what they're independent *of*, it makes little sense to use this
// elimination criterion on its own.

// Its closest direct relative criterion is IIA, which we know is
// unobtainable by any ranked method.

class elimination_util_const : public direct_relative_criterion_const {
	private:
		// elimination_spec[x] is -1 if the xth candidate is eliminated,
		// otherwise the number that candidate corresponds to after, e.g.
		// eliminating B in a  4cddt gives {0, -1, 1, 2};
		std::vector<int> elimination_spec;

		size_t get_num_noneliminated(const std::vector<int> &
			elimination_spec_in) const;

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
