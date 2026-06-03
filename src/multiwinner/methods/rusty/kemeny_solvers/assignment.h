#include <vector>
#include <glpk.h>

#include "tools/tools.h"
#include "common/ballots.h"

#include "multiwinner/methods/rusty/auxiliary/qballot.h"

using namespace std;

// Solve the forced clustering problem for a single instance.

// BLUESKY: Points system MIP solver.
// Also should check overdetermined case, where clusters > voters

class assignment {

	private:
		glp_smcp params;
		glp_prob * lp;

		double num_voters;
		int num_ballots, num_clusters;

		void initialize(double n, int u, int k, const
			vector<q_ballot> & voters);

	public:
		assignment(double n, int u, int k, const vector<q_ballot> &
			voters);

		~assignment();

		void set_constraint(int ballot_num, int cluster_num,
			double value) const;

		double calc_minimum() const;
		int get_status() const;
		bool success() const;

		double get_unknown(int index) const;
};