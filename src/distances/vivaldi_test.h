// Sketch Vivaldi algorithm for turning range data into coordinates. It may
// find a local optimum.
// This fits in with the w. benchmark in that we want to find a subset that
// "covers" the entire space well. To do that, we use squared correlation as
// range data.

#ifndef _VIVALDI
#define _VIVALDI

#include <algorithm>
#include <iostream>
#include <ext/numeric>

#include <stdlib.h>
#include <assert.h>
#include <vector>
#include <math.h>

using namespace std;

typedef vector<long double> coord;

class synth_coordinates {
	private:
		// Distance function.
		long double dist(const coord & a, const coord & b, 
				double Lp) const;
		// Update estimate of synthetic coordinates.
		void update(coord & me, coord & scratch, const coord & other, 
				double Lp, int my_index, int other_index,
				double delta, double noise_factor,
				const vector<vector<double> > & 
				recorded_distances) const;
		// Determine accuracy (RMSE) between given distances and
		// synthetic coordinate distances. This is used to report to
		// the user how accurate the estimate is.
		double get_rmse_accuracy(const vector<coord> & coordinates,
				const vector<vector<double> > &
				recorded_distances, double Lp) const;

		// Testing
		vector<vector<double> > construct_distances(const vector<coord>
				& coords, double Lp) const;

	public:
		// Used for testing purposes.
		vector<coord> generate_random_coords(int how_many,
				int dimensions) const;

		double recover_coords(vector<coord> & coordinates_out, 
				double Lp, double mindelta, double delta,
				double delta_stepsize,
				int numiters, const vector<vector<double> > & 
				recorded_distances, bool verbose) const;

		// Also for testing. If Lp_generation == Lp_recovery, the
		// return value should be very close to 0.
		double test_recovery(int num_coords, int dimensions, 
				double Lp_generation, double Lp_recovery,
				bool verbose) const;
};

#endif
