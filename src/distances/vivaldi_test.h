// Sketch Vivaldi algorithm for turning range data into coordinates. It may
// find a local optimum.
// This fits in with the w. benchmark in that we want to find a subset that
// "covers" the entire space well. To do that, we use squared correlation as
// range data.

#pragma once

#include <algorithm>
#include <iostream>
#include <ext/numeric>

#include <stdlib.h>
#include <assert.h>
#include <vector>
#include <math.h>


typedef std::vector<long double> coord;

class synth_coordinates {
	private:
		// Distance function.
		long double dist(const coord & a, const coord & b,
			double Lp) const;
		// Update estimate of synthetic coordinates.
		void update(coord & me, coord & scratch, const coord & other,
			double Lp, int my_index, int other_index,
			double delta, double noise_factor,
			const std::vector<std::vector<double> > &
			recorded_distances) const;
		// Determine accuracy (RMSE) between given distances and
		// synthetic coordinate distances. This is used to report to
		// the user how accurate the estimate is.
		double get_rmse_accuracy(const std::vector<coord> & coordinates,
			const std::vector<std::vector<double> > &
			recorded_distances, double Lp) const;

		// Testing
		std::vector<std::vector<double> > construct_distances(
			const std::vector<coord>
			& coords, double Lp) const;

	public:
		// Used for testing purposes.
		std::vector<coord> generate_random_coords(int how_many,
			int dimensions) const;

		double recover_coords(std::vector<coord> & coordinates_out,
			double Lp, double mindelta, double delta,
			double delta_stepsize,
			int numiters, const std::vector<std::vector<double> > &
			recorded_distances, bool verbose) const;

		// Also for testing. If Lp_generation == Lp_recovery, the
		// return value should be very close to 0.
		double test_recovery(int num_coords, int dimensions,
			double Lp_generation, double Lp_recovery,
			bool verbose) const;
};