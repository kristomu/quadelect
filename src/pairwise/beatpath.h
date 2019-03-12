#ifndef _VOTE_C_BEATPATH
#define _VOTE_C_BEATPATH

#include "matrix.h"

using namespace std;

///////////////////////////////////////////////////////////////////////

// Beatpath for Schulze's method. The beatpath matrix transforms a pairwise
// matrix into its beatpath equivalent, while Schulze acts as a WV version of
// Copeland on Beatpath.

// Remember: the class's behavior depends both on WV/margins/etc of the original
// matrix and of this one. It may also use huge amounts of space if the original
// matrix isn't "physical" (i.e. something like CPO-STV).

class beatpath : public abstract_condmat {

	private:
		vector<vector<double> > contents;
		void make_beatpaths(const abstract_condmat & input, 
				const vector<bool> & hopefuls);

	protected:
		double get_internal(int candidate, int against, bool raw) const;

		// Set doesn't work since this is an inferred ordering. To set,
		// one would have to pull all the data back in and rerun. Thus
		// this just returns an assertion failure if you try to set.
		bool set_internal(int /*candidate*/, int /*against*/, 
			double /*value*/) {
			assert (1 != 1);
		}

	public:
		beatpath(const abstract_condmat & input, pairwise_type type_in);
		beatpath(const abstract_condmat & input, pairwise_type type_in,
				const vector<bool> & hopefuls);
		beatpath(const list<ballot_group> & scores, int num_candidates,
				pairwise_type kind);

		double get_num_candidates() const { return(contents.size()); }
};

#endif
