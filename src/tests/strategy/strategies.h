#pragma once

#include <assert.h>
#include <stdlib.h>
#include <sstream>
#include <string>

#include <memory>
#include <stdexcept>

#include "../disproof.h"
#include "../runner.h"
#include "../test.h"

class per_ballot_strat : public criterion_test {
	private:
		// Returns a partially formed disproof - I'm going to do it
		// like that and then profile to see if it's too slow.
		void add_strategic_election_inner(
			disproof & partial_disproof, int64_t instance_index,
			const test_cache & cache, size_t numcands,
			pure_ballot_generator * ballot_generator,
			std::shared_ptr<coordinate_gen> randomizer) const;

	public:
		virtual ballot_group modify_ballots(ballot_group ballot,
			size_t winner, size_t challenger) const = 0;

		int64_t get_num_tries(size_t numcands) const {
			// One attempt per challenger
			return numcands-1;
		}

		std::string category() const {
			return "Strategy";
		}
};

class burial : public per_ballot_strat {
	public:
		ballot_group modify_ballots(ballot_group ballot,
			size_t winner, size_t challenger) const;

		std::string name() const {
			return "Burial immunity";
		}
};

class compromising : public per_ballot_strat {
	public:
		ballot_group modify_ballots(ballot_group ballot,
			size_t winner, size_t challenger) const;

		std::string name() const {
			return "Compromising immunity";
		}
};

class two_sided_strat : public per_ballot_strat {
	public:
		ballot_group modify_ballots(ballot_group ballot,
			size_t winner, size_t challenger) const;

		std::string name() const {
			return "Two-sided immunity";
		}
};

// Reverse the honest outcome and place the challenger first and
// the winner last.
// E.g. if the social outcome was W=A>B>C>D=E, and the winner is W and
// challenger C, the strategic faction all vote C>D=E>B>A>W.
class two_sided_reverse : public criterion_test {
	private:
		void add_strategic_election_inner(
			disproof & partial_disproof, int64_t instance_index,
			const test_cache & cache, size_t numcands,
			pure_ballot_generator * ballot_generator,
			std::shared_ptr<coordinate_gen> randomizer) const;

	public:
		std::string name() const {
			return "Two-sided reverse immunity";
		}

		int64_t get_num_tries(size_t numcands) const {
			// One attempt per challenger
			return numcands-1;
		}

		std::string category() const {
			return "Strategy";
		}

};

class coalitional_strategy : public criterion_test {
	private:
		void add_strategic_election_inner(
			disproof & partial_disproof, int64_t instance_index,
			const test_cache & cache, size_t numcands,
			pure_ballot_generator * ballot_generator,
			std::shared_ptr<coordinate_gen> randomizer) const;

	public:
		std::string name() const {
			return "Coalitional strategy immunity";
		}

		int64_t get_num_tries(size_t /*numcands*/) const {
			// Too many to count.
			return -1;
		}

		std::string category() const {
			return "Strategy";
		}
};