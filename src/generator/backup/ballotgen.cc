// Ballot generator: impartial (All rankings have equal probability)
// No equal-rank yet. Do that later, and also produce an ABC.

// Maybe add in Gaussian here some time for great refactoring justice re Yee.

// Make ballot_generator even more pure, with generate_ballots as pure virtual,
// then derive indiv_ballot_generator from it. Thus we can have barycentric,
// gaussian, etc, as a non-indiv_ballot generator.

#ifndef _BALLOT_GENERATOR
#define _BALLOT_GENERATOR

#include "../method.cc"
#include "../ballot_tools.cc"
#include <ext/numeric>
#include <list>

using namespace std;

class pure_ballot_generator {
	protected:
		bool truncate, compress;

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
		list<ballot_group> generate_ballots(int num_voters,
			int numcands) const {
			ballot_tools bt;
			if (compress)
				return (bt.compress(generate_ballots(num_voters,
								numcands,
								truncate)));
			else
				return (generate_ballots(num_voters, numcands,
							truncate));
		}
		virtual list<ballot_group> generate_ballots(int num_voters,
			int numcands, bool do_truncate) const = 0;
};


// A class for ballot generators where each ballot has no dependency upon any
// other.
class indiv_ballot_generator : public pure_ballot_generator {
	protected:
		virtual ordering generate_ordering(int numcands,
			bool do_truncate) const = 0;

	public:
		indiv_ballot_generator() : pure_ballot_generator() {}
		indiv_ballot_generator(bool compress_in) :
			pure_ballot_generator(compress_in) {}
		indiv_ballot_generator(bool compress_in, bool do_truncate) :
			pure_ballot_generator(compress_in, do_truncate) {}

		list<ballot_group> generate_ballots(int num_voters,
			int numcands, bool do_truncate) const;
		// WHAT??? TODO: Find out WTH is going on here.
		list<ballot_group> generate_ballots(int num_voters,
			int numcands) const {
			ballot_tools bt;
			if (compress)
				return (bt.compress(generate_ballots(
								num_voters,
								numcands,
								truncate)));
			else
				return (generate_ballots(num_voters, numcands,
							truncate));
		}

};

list<ballot_group> indiv_ballot_generator::generate_ballots(int num_voters,
	int numcands, bool do_truncate) const {

	list<ballot_group> toRet;
	ballot_group to_add;
	to_add.weight = 1;

	for (int counter = 0; counter < num_voters; ++counter) {
		to_add.contents = generate_ordering(numcands, do_truncate);
		toRet.push_back(to_add);
	}

	return (toRet);
}

// TODO: Find out whether the truncated ballots are equiprobably distributed.
class impartial : public indiv_ballot_generator {
	private:
		ordering generate_ordering(int numcands,
			bool do_truncate) const;

	public:
		impartial(bool compress_in) :
			indiv_ballot_generator(compress_in) {}
		impartial(bool compress_in, bool do_truncate) :
			indiv_ballot_generator(compress_in, do_truncate) {}

		/*	list<ballot_group> generate_ballots(
					int num_voters, int numcands,
					bool do_truncate) const;*/
};

//ordering impartial::generate_ordering(int numcands, bool do_truncate) const {
ordering impartial::generate_ordering(int numcands,
	bool do_truncate) const {
	/*list<ballot_group> impartial::generate_ballots(int num_voters, int numcands,
			bool do_truncate) const {*/

	// Fill a vector with 0...numcands, then shuffle randomly, then turn
	// into ordering. Note that this does the allocation and iota
	// every single time, which is wasteful. FIX LATER, which will entail
	// making non-const. Having iota done ahead of time is a bad idea,
	// because it destroys reproducibility.

	vector<int> candidates(numcands, 0);
	iota(candidates.begin(), candidates.end(), 0);

	ordering out;

	/*for (int ounter = 0; counter < num_voters; ++counter) {
		to_add.contents = generate_ordering(numcands, do_truncate);
		toRet.push_back(to_add);
	}*/

	random_shuffle(candidates.begin(), candidates.end());

	size_t this_far;
	if (do_truncate && candidates.size() > 1) {
		this_far = 1 + random() % (candidates.size());
	} else	{
		this_far = candidates.size();
	}

	for (int counter = 0; counter < this_far; ++counter)
		out.insert(candscore(candidates[counter],
				counter));

	return (out);
}

#endif
