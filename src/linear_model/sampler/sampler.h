#pragma once

// An implementation of the billiard walk algorithm given in

// GRYAZINA, Elena; POLYAK, Boris. Random sampling: Billiard walk algorithm.
// European Journal of Operational Research, 2014, 238.2: 497-504.

#include <iostream>
#include <eigen3/Eigen/Dense>
#include <glpk.h>

#include <random>
#include <numeric>
#include <stdexcept>

#include "../../random/random.h"

#include "../polytope/polytope.h"
#include "../polytope/properties/distance.h"
#include "../polytope/properties/center.h"

struct ray {
	Eigen::VectorXd orig;
	Eigen::VectorXd dir;
};

struct halfplane_result {
	bool colliding;
	double distance;
	int halfplane_idx;
};

template<typename T> class billiard_sampler {

	private:
		T polytope_to_sample;
		bool output_extended_points;
		double diameter;
		ray current_sampler_ray;
		rng randomizer;

		Eigen::VectorXd random_unit_vector(int dimension);
		halfplane_result get_closest_halfplane_dist(
			const polytope & poly_in, const ray & ray_in) const;

		bool billiard_walk_internal(const polytope & poly_in, ray & ray_in,
			double max_distance, int max_reflections) const;

		// quick_diameter_bounds determines whether to use a linear
		// relaxation when calculating the furthest-distance diameter.
		// For high dimensions, quick_diameter_bounds=true is highly
		// recommended, though it may make mixing times slower, because
		// the MIP takes too long time to solve.
		void update_properties(bool quick_diameter_bounds) {
			if (!polytope_to_sample.is_full_rank()) {
				throw std::domain_error("billiard_sampler: rank-deficient matrices are unsupported!");
			}

			// Make this function return a lower bound parameterized
			// by time we want to spend solving the MIP? tm_lim in
			// glp_smcp... Eh, seems to work well enough so far.
			diameter = polytope_distance(quick_diameter_bounds).
				get_l2_diameter_lb(polytope_to_sample);

			if (diameter == 0) {
				throw std::logic_error("billiard_sampler: LP diameter "
					"approximation is 0!");
			}

			// Set the starting position to the Chebyshev center
			current_sampler_ray.orig = polytope_center().get_center(
				polytope_to_sample);

		}

		// Preserving means that it doesn't update the initial point of
		// the sampling: repeated calls will always return the same value.
		ray billiard_walk_preserving(const Eigen::VectorXd & initial_point,
			double tau_distance, int max_reflections, int max_retries);

	public:
		// Defaults as in the paper, and with max_retries = 100.
		Eigen::VectorXd billiard_walk();

		void set_rng_seed(uint64_t seed) { randomizer.s_rand(seed); }

		// Output_full_points: if true, this returns the full points of a
		// polytope (e.g. x instead of z for equality_polytope) even though
		// those are not the kind of points the sampler uses internally.
		void set_polytope(T polytope_in, bool output_ext_points) {
			output_extended_points = output_ext_points;
			polytope_to_sample = polytope_in;
			update_properties();
		}

		billiard_sampler(T polytope_in, bool output_ext_points,
			bool quick_diameter_bounds, uint64_t seed) :
			polytope_to_sample(polytope_in), randomizer(seed) {

			output_extended_points = output_ext_points;
			update_properties(quick_diameter_bounds);
		}

		billiard_sampler(uint64_t seed) : randomizer(seed) {}

		Eigen::VectorXd get_current_point() const {
			if (output_extended_points) {
				return polytope_to_sample.get_full_coordinates(
					current_sampler_ray.orig);
			} else {
				return current_sampler_ray.orig;
			}
		}
};

// Since we're using templating, all the functions have to be in the header
// as well.

template<typename T> Eigen::VectorXd billiard_sampler<T>::random_unit_vector(
	int dimension) {
	// Create a ray pointing in a random direction with unit magnitude.
	// Having unit magnitude makes k the distance to the closest edge in
	// get_closest_half_plane_dist without any need to normalize there.

	Eigen::VectorXd out(dimension);

	// RNG hack: Box-Muller all the way. This wastes one normal variable if
	// the dimension is odd. Perhaps I should create a normal_distribution
	// class to deal with that, eventually, but for now...

	// Stolen from generator/spatial/gaussian.cc

	for (int i = 0; i < dimension; i += 2) {
		double x = 0, y = 0, rad = 0;

		while (rad > 1.0 || rad == 0) {
			// Choose x,y on the square (-1, -1) to (+1, +1)
			x = -1 + 2 * randomizer.drand();
			y = -1 + 2 * randomizer.drand();

			// Calculate the squared radius from origin to see if we're
			// within the unit circle.
			rad = x * x + y * y;
		}

		// Okay, now we have (x,y) within the unit circle. Transform to get a
		// random Gaussian distributed variable.
		double conv_factor = sqrt(-2.0 * log(rad) / rad);

		out(i) = x * conv_factor;
		if (i+1 < dimension) {
			out(i+1) = y * conv_factor;
		}
	}

	return out / out.norm();
}

// Returns the closest half-plane intersecting the ray, and the distance to
// it from the ray's origin.
template<typename T> halfplane_result billiard_sampler<T>::get_closest_halfplane_dist(
	const polytope & poly_in, const ray & ray_in) const {

	halfplane_result out;

	double dist_record = std::numeric_limits<double>::infinity();
	size_t halfplane_idx = 0;

	// Note: can be larger than the number of dimensions!
	size_t num_halfplanes = poly_in.get_num_halfplanes();

	// Hack to avoid numerical precision issues. In essence, this factor
	// gives each edge a thickness, where we'll never go from inside the
	// thickness of the edge to some other point inside that band.
	double dist_epsilon = 1e-9;

	for (size_t i = 0; i < num_halfplanes; ++i) {
		double travel_magnitude = ray_in.dir.dot(poly_in.get_A().row(i));

		// If the ray direction is parallel to the edge, skip.
		if (travel_magnitude == 0) { continue; }

		double dist = (poly_in.get_b()[i] - ray_in.orig.dot(poly_in.get_A().
			row(i))) / travel_magnitude;

		// If we're heading away from the half-plane, no need to check
		// further, so skip.
		if (dist < 0) { continue; }

		if (dist < dist_record && dist > dist_epsilon) {
			dist_record = dist;
			halfplane_idx = i;
		}

		// If we're within epsilon distance, we're at an edge. The ray may
		// be pointing out of the polytope or into it. If it's pointing out
		// of the polytope, then we can't travel any distance without moving
		// out of bounds, and so billiard sampling needs to reflect instead.

		// We're traveling out of the polytope if the derivative of
		// (x_p + dist * x_v) * a[i] - b[i] wrt dist is positive. The
		// derivative is precisely travel_magnitude, and so we get...

		if (dist <= dist_epsilon && travel_magnitude > 0) {
			out.colliding = true;
			out.halfplane_idx = i;
			return out;
		}
	}

	// If dist_record is infinity, then the space is unbounded and the
	// ray is pointing into the unbounded region. Throw an exception.
	if (std::isinf(dist_record)) {
		throw new std::domain_error("closest_half_plane: unbounded polytope!");
	}

	out.colliding = false;
	out.distance = dist_record;
	out.halfplane_idx = halfplane_idx;

	return out;
}

template<typename T> bool billiard_sampler<T>::billiard_walk_internal(const polytope & poly_in, ray & ray_in,
	double max_distance, int max_reflections) const {

	double distance_remaining = max_distance;
	halfplane_result closest;

	ray current_ray = ray_in;

	for (int i = 0; i < max_reflections; ++i) {
		closest = get_closest_halfplane_dist(poly_in, current_ray);

		// If we're not colliding, then setting the ray origin to its
		// old origin + distance * direction keeps us inside the polytope.
		// That means we can travel towards the closest edge along the
		// ray's direction of travel. We will then either exhaust our max
		// distance, or end up at an edge with the ray direction pointing
		// out of the polytope.

		// On the other hand, if it's not colliding, then we're already at
		// an edge with the ray direction pointing out of the polytope, so
		// we can't travel at all.
		if (!closest.colliding) {
			double distance_to_travel = std::min(distance_remaining,
				closest.distance);
			current_ray.orig += current_ray.dir * distance_to_travel;
			distance_remaining -= distance_to_travel;

			if (distance_remaining == 0) {
				ray_in = current_ray;
				return true;
			}
		}

		// We're at an edge and we need to reflect.
		Eigen::VectorXd int_normal = -poly_in.get_A().row(closest.
			halfplane_idx);
		int_normal /= int_normal.norm();

		current_ray.dir -= 2 * current_ray.dir.dot(int_normal) * int_normal;
	}

	// Passed max_reflections without covering the required distance.
	return false;
}

template<typename T> ray billiard_sampler<T>::billiard_walk_preserving(
	const Eigen::VectorXd & initial_point, double tau_distance,
	int max_reflections, int max_retries) {

	int dimension = polytope_to_sample.get_dimension();

	ray candidate;
	candidate.orig = initial_point;

	for (int i = 0; i < max_retries; ++i) {
		candidate.dir = random_unit_vector(dimension);

		double max_distance = -tau_distance * log(randomizer.drand());

		if (billiard_walk_internal(polytope_to_sample, candidate,
			max_distance, max_reflections)) {

			return candidate; // it has now been updated.
		}
	}

	throw std::runtime_error("billiard_walk: timed out trying to find \
		a new point");
}

template<typename T> Eigen::VectorXd billiard_sampler<T>::billiard_walk() {

	current_sampler_ray = billiard_walk_preserving(current_sampler_ray.orig,
		diameter, 10 * polytope_to_sample.get_dimension(), 100);

	if (output_extended_points) {
		return polytope_to_sample.get_full_coordinates(
			current_sampler_ray.orig);
	} else {
		return current_sampler_ray.orig;
	}
}