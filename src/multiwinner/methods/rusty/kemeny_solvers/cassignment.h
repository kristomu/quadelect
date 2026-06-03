// KLUDGE to get both CFC-Range and FC-Kemeny.

#pragma once

#include <vector>
#include <glpk.h>

#include "tools/tools.h"
#include "common/ballots.h"

using namespace std;

// Solve the forced clustering problem for a single instance.

// BLUESKY: Points system MIP solver.
// Also should check overdetermined case, where clusters > voters

// Remember: must remove the other cassignment class or there will be a clash.

class cassignment {

	private:
		bool inited;

		glp_smcp params;
		glp_prob * lp;

		double num_voters;
		int num_ballots, num_clusters;

	public:
		void initialize(double n, int u, int k,
			const list<ballot_group> & voters,
			bool maximize);

		cassignment(double n, int u, int k, const list<ballot_group> &
			voters, bool maximize);
		cassignment();

		~cassignment();

		void set_constraint(int ballot_num, int cluster_num,
			double value) const;

		double calc_optimum(bool debug) const;
		double calc_optimum() const;
		int get_status() const;
		bool success() const;

		double get_unknown(int index) const;
};