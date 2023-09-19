#pragma once

#include "method.h"
#include "../ballots.h"
#include "../tools/tools.h"
#include "../tools/ballot_tools.h"

#include "positional/simple_methods.h"

#include <list>
#include <memory>

// Loser-elimination meta-method. This method takes a base method and
// repeatedly disqualifies the loser (if loser-elimination) or those with a
// below-mean score (if average loser elimination).

// The tiebreaks are by first difference, but they can be set to be by last
// difference with a boolean. If the tie remains, it is broken by random
// candidate, and thus the method is not technically cloneproof. (Might do
// something with that later)

class loser_elimination : public election_method {
	private:
		std::shared_ptr<const election_method> base;

		// If average_loser_elim is true, then all candidates at
		// or below mean score is eliminated. If not, only the loser
		// is eliminated.
		bool average_loser_elim;

		// If first_differences is set to false, then the method breaks
		// ties in last in first out order. The more usual tiebreak is
		// the method of first differences, which is what we use if
		// the variable is true, and that breaks ties by the first
		// ballot that expresses a difference between the tied
		// candidates.
		bool first_differences;

		std::string cached_name;

		ordering break_tie(const ordering & original_ordering,
			const std::list<ordering> & past_ordering,
			int num_candidates) const;

	protected:
		std::pair<ordering, bool> elect_inner(const
			election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string determine_name() const;

	public:

		loser_elimination(
			std::shared_ptr<const election_method> base_method,
			bool average_loser, bool use_first_diff);

		std::string name() const {
			return (cached_name);
		}
};

// Convenient aliases for some common elimination methods.

template<typename T> class elim_shortcut : public election_method {
	private:

		// We need to use shared pointers for the positional
		// base method and the eliminator because otherwise the
		// base method will go out of scope before the eliminator,
		// resulting in an invalid delete.

		// I think there's a way to manually specify the order
		// that parameters go out of scope, but I couldn't find
		// out how, so this will have to do.

		std::shared_ptr<T> positional_base;
		std::shared_ptr<loser_elimination> eliminator;
		bool first_diff;

	protected:
		// The elimination method name.
		virtual std::string common_name() const = 0;

		std::pair<ordering, bool> elect_inner(const
			election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const {

			return eliminator->elect_detailed(papers, hopefuls,
					num_candidates, cache, winner_only);
		}

	public:
		elim_shortcut(positional_type equal_rank_handling,
			bool average_elimination, bool use_first_diff) {

			first_diff = use_first_diff;

			positional_base = std::make_shared<T>(equal_rank_handling);
			eliminator = std::make_shared<loser_elimination>(
					positional_base, average_elimination, first_diff);
		}

		std::string name() const {
			std::string name_out = positional_base->show_type() +
				common_name();
			if (first_diff) {
				name_out += "/fd";
			} else {
				name_out += "/ld";
			}

			return name_out;
		}
};

class instant_runoff_voting : public elim_shortcut<plurality> {
	protected:
		std::string common_name() const {
			return "IRV";
		}

	public:
		instant_runoff_voting(positional_type equal_rank_handling,
			bool use_first_diff) : elim_shortcut(equal_rank_handling,
					false, use_first_diff) {}
};

class baldwin : public elim_shortcut<borda> {
	protected:
		std::string common_name() const {
			return "Baldwin";
		}

	public:
		baldwin(positional_type equal_rank_handling,
			bool use_first_diff) : elim_shortcut(equal_rank_handling,
					false, use_first_diff) {}
};

class coombs : public elim_shortcut<antiplurality> {
	protected:
		std::string common_name() const {
			return "Coombs";
		}

	public:
		coombs(positional_type equal_rank_handling,
			bool use_first_diff) : elim_shortcut(equal_rank_handling,
					false, use_first_diff) {}
};

class ifpp : public elim_shortcut<plurality> {
	protected:
		std::string common_name() const {
			return "IFPP";
		}

	public:
		ifpp(positional_type equal_rank_handling,
			bool use_first_diff) : elim_shortcut(equal_rank_handling,
					true, use_first_diff) {}
};

class nanson : public elim_shortcut<borda> {
	protected:
		std::string common_name() const {
			return "Nanson";
		}

	public:
		nanson(positional_type equal_rank_handling,
			bool use_first_diff) : elim_shortcut(equal_rank_handling,
					true, use_first_diff) {}
};
