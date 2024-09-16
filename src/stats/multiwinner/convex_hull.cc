// Calculate the convex hull of VSEs. The interface is similar to
// vse_region_mapper's: call add_point or add_points and then update once
// every new point has been added.

// This currently only supports 2D convex hulls; I'll add a more general
// algorithm later if/when I need it.

// Uses Andrew's monotone chain algorithm with cross product, as seen on
// https://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain

#include "convex_hull.h"

#include <algorithm>
#include <iostream>
#include <vector>

double cross(const VSE_point_wrapper & O, const VSE_point_wrapper & A,
	const VSE_point_wrapper & B) {

	return (A.x() - O.x()) * (B.y() - O.y())
		- (A.y() - O.y()) * (B.x() - O.x());
}

void VSE_convex_hull::add_point(const VSE_point & cur_round_VSE) {
	size_t old_convex_hull_points = hull.size();

	for (auto pos = hull.begin(); pos != hull.end(); ++pos) {

		// Copy the cloud point, add the new data, and insert.
		VSE_point new_cloud_point = pos->p;
		for (size_t i = 0; i < new_cloud_point.size(); ++i) {
			new_cloud_point[i].add_last(cur_round_VSE[i]);
		}

		new_points.push_back(new_cloud_point);

		// If we start to get a lot of points, deal with them,
		// because the convex hull calculation has space complexity
		// linear in the number of new points, which can blow our
		// memory budget if therre are lots of new points.

		if (new_points.size() > 50 * old_convex_hull_points) {
			new_points = convex_hull(new_points, false);
		}
	}
}

void VSE_convex_hull::add_points(const std::vector<VSE_point> & points) {
	size_t points_generated = 0;

	for (const VSE_point & p: points) {
		add_point(p);
		points_generated += hull.size(); // Is this true?
	}

	std::cout << "Handled " << points_generated << " points\n";
}

// Returns a list of points on the convex hull in counter-clockwise order.
// Note: the last point in the returned list is the same as the first one.
std::vector<VSE_point_wrapper> VSE_convex_hull::convex_hull(
	std::vector<VSE_point_wrapper> & P, bool verbose) {

	size_t n = P.size(), k = 0;
	if (n <= 3) {
		return P;
	}

	// Early pruning, etc, TODO; we know roughly where each point is,
	// too.

	std::vector<VSE_point_wrapper> H(2*n);

	// Sort points lexicographically
	std::sort(P.begin(), P.end());

	// Build lower hull
	for (size_t i = 0; i < n; ++i) {
		while (k >= 2 && cross(H[k-2], H[k-1], P[i]) <= 0) {
			k--;
		}
		H[k++] = P[i];
	}

	// Build upper hull
	for (size_t i = n-1, t = k+1; i > 0; --i) {
		while (k >= t && cross(H[k-2], H[k-1], P[i-1]) <= 0) {
			k--;
		}
		H[k++] = P[i-1];
	}

	H.resize(k-1);

	if (verbose) {
		std::cout << "Convex hull: reduced to " << k-1 << " points. " << std::endl;
	}

	return H;
}

double VSE_convex_hull::get_area() const {

	// Make use of the counterclockwise order of the vertices of the
	// hull.

	double area_sum = 0;

	for (size_t i = 0; i < hull.size(); ++i) {
		// 1/2 sum i=1..n y_i * (x_{i-1} - x_{i+1})
		size_t prev = (i + hull.size() - 1) % hull.size(),
			   next = (i + 1) % hull.size();

		area_sum += hull[i].y() * (hull[prev].x() - hull[next].x());
	}

	return area_sum/2.0;
}

void VSE_convex_hull::dump_coordinates(std::ostream & where) const {

	for (auto pos = hull.begin(); pos != hull.end(); ++pos) {
		where << pos->x() << " " << pos->y() << "\n";
	}
}
