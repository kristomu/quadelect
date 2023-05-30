// Abstract classes for ballot generators.

// Maybe add in Gaussian here some time for great refactoring justice re Yee.
// See the barycentric class for an idea of how to do this while also making it
// possible to set parameters beforehand.

// Make ballot_generator even more pure, with generate_ballots as pure virtual,
// then derive indiv_ballot_generator from it. Thus we can have barycentric,
// gaussian, etc, as a non-indiv_ballot generator.

// TODO: Have an option to return both full and truncated ballots. When
// calculating social utilities, every voter has a social preference for a
// candidate, he just doesn't know some of them if he truncates.

#ifndef _BALLOT_GENERATOR
#define _BALLOT_GENERATOR

#include "../singlewinner/method.h"
#include "../tools/ballot_tools.h"
#include "../random/random.h"

#include <numeric>
#include <list>


class pure_ballot_generator {
	protected:
		bool truncate, compress;

		virtual std::list<ballot_group> generate_ballots_int(int num_voters,
			int numcands, bool do_truncate,
			rng & random_source) const = 0;

	public:
		// Inheriting classes will have to set defaults on these
		// constructors, so that they don't leave the variables in
		// an undefined state.
		pure_ballot_generator() {
			compress = true;
			truncate = false;
		}
		pure_ballot_generator(bool compress_in) {
			compress = compress_in; truncate = false;
		}
		pure_ballot_generator(bool compress_in, bool do_truncate) {
			compress = compress_in; truncate = do_truncate;
		}

		// Provides an empty ballot on error.
		// We ask for truncation because some criteria, like
		// mono-append, make no sense without.
		std::list<ballot_group> generate_ballots(int num_voters,
			int numcands, rng & random_source) const {
			ballot_tools bt;
			if (compress)
				return (bt.compress(generate_ballots_int(
								num_voters,
								numcands,
								truncate,
								random_source
							)));
			else
				return (generate_ballots_int(num_voters,
							numcands, truncate,
							random_source));
		}

		virtual std::string name() const = 0;
		virtual ~pure_ballot_generator() {}
};


// A class for ballot generators where each ballot has no dependency upon any
// other.
class indiv_ballot_generator : public pure_ballot_generator {
	protected:
		virtual ordering generate_ordering(int numcands,
			bool do_truncate, rng & random_source)
		const = 0;

		std::list<ballot_group> generate_ballots_int(int num_voters,
			int numcands, bool do_truncate,
			rng & random_source) const;

	public:
		indiv_ballot_generator() : pure_ballot_generator() {}
		indiv_ballot_generator(bool compress_in) :
			pure_ballot_generator(compress_in) {}
		indiv_ballot_generator(bool compress_in, bool do_truncate) :
			pure_ballot_generator(compress_in, do_truncate) {}

};

#endif
