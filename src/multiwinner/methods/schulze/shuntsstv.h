#pragma once

// Schulze STV.

// This is a shunt function for thirdelect/quadelect usage, though
// Schulze's main code could definitely be improved, particularly as
// regards the C-style callocs and frees. -KM

#include "common.h"
#include "multiwinner/methods/methods.h"
#include <list>

class SchulzeSTVCalc : public SchulzeCommon {
	public:
		void Elimination();
		void Calculation_of_the_Strengths_of_the_Vote_Managements();
		void Kombinationen();
		void Kombinationen2();
		council_t Dijkstra();

		council_t analyze_and_get_outcome();
};

class SchulzeSTV : public multiwinner_method {

	public:
		council_t get_council(size_t council_size,
			size_t num_candidates, const election_t & ballots) const;

		std::string name() const {
			return ("Schulze STV");
		}

		// Render an election in the Schulze STV input file format.
		void print_schulze_stv_prefs(size_t council_size,
			size_t num_candidates, const election_t & ballots) const;

		// HACK - not really true.
		// Ideally should be two functions: first, "is permissible for
		// all", second, "is permissible for this particular setup"
		// (that is, numcands, council size, etc.)
		// The former would be used for one-char info so that those that
		// only sample a subset are marked as such.

		// Or perhaps "Expected time" and "expected space", with -1
		// on both for "always acceptable".
		bool polytime() const {
			return (false);
		}

};