#include <vector>
#include <glpk.h>

#include "tools/tools.h"

using namespace std;

// Solve the forced clustering problem for a single instance.

// BLUESKY: Points system MIP solver.
// Also should check overdetermined case, where clusters > voters

// TODO: Add constraint < previous record (to quickly determine if it's
// infeasible)

class assignment {

	private:
		glp_smcp params;
		glp_prob * lp;

		double num_voters;
		int num_ballots, num_clusters;

		bool use_record;
		int record_constraint;

		void initialize(double n, int u, int k, const vector<double> &
			voters);

	public:
		assignment(double n, int u, int k, const vector<double> &
			voters, bool using_record);

		assignment(double n, int u, int k, const vector<double> &
			voters) : assignment(n, u, k, voters, false) {}

		~assignment();

		void set_constraint(int ballot_num, int cluster_num,
			double value) const;

		double calc_minimum() const;
		int get_status() const;
		bool success() const;

		double get_unknown(int index) const;
};