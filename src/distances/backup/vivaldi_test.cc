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
		/*std::vector<coord> generate_random_coords(int how_many,
				int dimensions) const;*/
		std::vector<std::vector<double> > construct_distances(
			const std::vector<coord>
			& coords, double Lp) const;

	public:
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

std::vector<coord> synth_coordinates::generate_random_coords(int how_many,
	int dimensions) const {

	std::vector<coord> coordinates;

	for (int counter = 0; counter < how_many; ++counter) {
		coord to_add;
		for (int sec = 0; sec < dimensions; ++sec) {
			to_add.push_back(drand48());
		}

		coordinates.push_back(to_add);
	}

	return (coordinates);
}

long double synth_coordinates::dist(const coord & a, const coord & b,
	double Lp) const {

	long double error = 0;

	int dimension = std::min(a.size(), b.size());

	for (int counter = 0; counter < dimension; ++counter) {
		error += (a[counter]-b[counter])*(a[counter]-b[counter]);
	}

	// Most common. Perhaps make a quicker nth root function later.
	if (Lp == 2) {
		return (sqrt(error));
	}

	return (fabs(pow(error, 1.0/Lp)));
}

std::vector<std::vector<double> > synth_coordinates::construct_distances(
	const std::vector<coord> & coords, double Lp) const {

	std::vector<std::vector<double> > toRet(coords.size(), std::vector<double>(
			coords.size(), 0));

	for (int counter = 0; counter < coords.size(); ++counter)
		for (int sec = 0; sec < coords.size(); ++sec)
			toRet[counter][sec] = dist(coords[counter],
					coords[sec], Lp);

	return (toRet);
}

void synth_coordinates::update(coord & me, coord & to_target,
	const coord & other, double Lp,
	int my_index, int other_index, double delta,
	double noise_factor,
	const std::vector<std::vector<double> > & recorded_distances) const {

	// Get unit vector from ourselves to the target
	// (Construct a polar form of (target - me) with magnitude 1)
	//coord to_target(min(other.size(), me.size()));
	int counter;
	for (counter = 0; counter < to_target.size(); ++counter) {
		to_target[counter] = other[counter] - me[counter];
	}

	// Get current magnitude
	long double magnitude = dist(me, other, Lp);

	// If we're on top of the other, get outta here, nothing we can do.
	if (magnitude == 0) {
		return;
	}

	// If we should be on top of the other but aren't, do something about
	// it.
	if (recorded_distances[my_index][other_index] == 0) {
		me = other;
		return;
	}

	// Set magnitude to distance from rest position, times delta.
	long double ddist = magnitude - recorded_distances[my_index]
		[other_index];

	ddist *= (1 - noise_factor * drand48()) * delta;

	// Add the vector towards our target with magnitude ddist to our
	// current position.

	for (counter = 0; counter < to_target.size(); ++counter) {
		me[counter] += to_target[counter] * (ddist/magnitude);
	}
}

double synth_coordinates::get_rmse_accuracy(const std::vector<coord> &
	coordinates,
	const std::vector<std::vector<double> > & recorded_distances,
	double Lp) const {

	double error = 0;
	int sumcount = 0;

	for (int counter = 0; counter < coordinates.size(); ++counter)
		for (int sec = 0; sec < coordinates.size(); ++sec) {
			++sumcount;
			double term = dist(coordinates[counter],
					coordinates[sec], Lp) -
				recorded_distances[counter][sec];
			error += term*term;
		}

	return (sqrt(error / (double)sumcount));
}

double synth_coordinates::recover_coords(std::vector<coord> & coordinates,
	double Lp, double mindelta, double delta, double delta_stepsize,
	int numiters,
	const std::vector<std::vector<double> > & recorded_distances,
	bool verbose) const {

	std::vector<double> our_deltas(coordinates.size(), delta);
	std::vector<coord> record_coordinates;
	double record_error = INFINITY, cur_error;

	// Set up the update coordinate so that it'll have the right number
	// of dimensions.
	coord scratch(coordinates[0].size());

	std::vector<int> one(coordinates.size());
	iota(one.begin(), one.end(), 0);
	std::vector<int> vs = one;

	for (int iter = 0; iter < numiters && record_error > 0; ++iter) {

		// Randomize the order in which we compare distances. Doing so
		// breaks some path-dependent local optima and improves the
		// result.
		random_shuffle(one.begin(), one.end());
		random_shuffle(vs.begin(), vs.end());

		for (int counter = 0; counter < coordinates.size(); ++counter) {
			int point = one[counter];

			for (int sec = 0; sec < coordinates.size(); ++sec) {
				int vs_point = vs[sec];

				if (point == vs_point) {
					continue;
				}

				// Update with a slight element of simulated
				// annealing.
				update(coordinates[vs_point], scratch,
					coordinates[point],
					Lp, vs_point, point,
					our_deltas[vs_point],
					1 - (iter/(double)numiters),
					recorded_distances);

				our_deltas[vs_point] = std::max(
						our_deltas[vs_point] -
						delta_stepsize,	mindelta);
			}
		}

		cur_error = get_rmse_accuracy(coordinates, recorded_distances,
				Lp);

		if (verbose)
			std::cout << "After iteration " << iter << ": error is " <<
				cur_error << std::endl;

		if (cur_error < record_error) {
			if (verbose)
				std::cout << "New record! " << cur_error
					<< " is better than " << record_error
					<< std::endl;

			record_coordinates = coordinates;
			record_error = cur_error;
		}
	}

	// Finally, restore to the best we got.
	coordinates = record_coordinates;
	return (record_error);
}

double synth_coordinates::test_recovery(int num_coords, int dimensions,
	double Lp_generation, double Lp_recovery, bool verbose) const {

	std::vector<coord> attempt = generate_random_coords(num_coords,
			dimensions);
	std::vector<std::vector<double> > exogenous = construct_distances(attempt,
			Lp_generation);
	attempt = generate_random_coords(num_coords, dimensions); // Scramble.
	return (recover_coords(attempt, Lp_recovery, 0.025, 1.0, 0.015,
				2000, exogenous, verbose));
}

/*main() {
	synth_coordinates test;

	test.test_recovery(100, 3, 2, 2, true);
}*/

#endif
