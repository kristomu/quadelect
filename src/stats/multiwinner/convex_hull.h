// Calculate the convex hull of VSEs. The interface is similar to
// vse_region_mapper's: call add_point or add_points and then update once
// every new point has been added.

// This currently only supports 2D convex hulls; I'll add a more general
// algorithm later if/when I need it.

// Uses Andrew's monotone chain algorithm with cross product, as seen on
// https://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain

#include "vse.h"
#include <vector>
#include <iostream>

// TODO: Quantization

class VSE_point_wrapper {
	public:
		VSE_point p;

		bool operator < (const VSE_point_wrapper & other) const {
			if (p.size() != 2) {
				throw std::invalid_argument("Trying to do convex hull "
					"with something that's not 2D!");
			}

			return p[0].get() < other.p[0].get() ||
				(p[0].get() == other.p[0].get()
					&& p[1].get() < other.p[1].get());
		}

		double x() const {
			return p[0].get();
		}
		double y() const {
			return p[1].get();
		}

		VSE_point_wrapper(const VSE_point & in) {
			p = in;
		}
		VSE_point_wrapper() {
			p = VSE_point(2);
		}
};

double cross(const VSE_point_wrapper & O, const VSE_point_wrapper & A,
	const VSE_point_wrapper & B) {

	return (A.x() - O.x()) * (B.y() - O.y())
		- (A.y() - O.y()) * (B.x() - O.x());
}


class VSE_convex_hull {
	private:
		std::vector<VSE_point_wrapper> convex_hull(
			std::vector<VSE_point_wrapper> & P);

	public:
		std::vector<VSE_point_wrapper> hull;
		std::vector<VSE_point_wrapper> new_points;

		void add_point(const VSE_point & cur_round_VSE) {
			for (auto pos = hull.begin(); pos != hull.end(); ++pos) {

				// Copy the cloud point, add the new data, and insert.
				VSE_point new_cloud_point = pos->p;
				for (size_t i = 0; i < new_cloud_point.size(); ++i) {
					new_cloud_point[i].add_last(cur_round_VSE[i]);
				}

				new_points.push_back(new_cloud_point);
			}
		}

		void add_points(const std::vector<VSE_point> & points) {
			for (const VSE_point & p: points) {
				add_point(p);
			}
		}


		VSE_convex_hull() {
			// Add empty VSE so that add_point will work properly.
			hull.resize(1);
		}

		void update() {
			hull = convex_hull(new_points);
			new_points.clear();
		}

		void dump_coordinates(std::ostream & where) const;
};

// Returns a list of points on the convex hull in counter-clockwise order.
// Note: the last point in the returned list is the same as the first one.
std::vector<VSE_point_wrapper> VSE_convex_hull::convex_hull(
	std::vector<VSE_point_wrapper> & P) {

	size_t n = P.size(), k = 0;
	if (n <= 3) {
		return P;
	}

	// Early pruning, etc, TODO; we know roughly where each point is,
	// too.

	std::cout << "Dealing with " << n << " potential points.\n";

	std::vector<VSE_point_wrapper> H(2*n);

	// Sort points lexicographically
	sort(P.begin(), P.end());

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
	return H;
}

void VSE_convex_hull::dump_coordinates(std::ostream & where) const {

	for (auto pos = hull.begin(); pos != hull.end(); ++pos) {
		where << pos->x() << " " << pos->y() << "\n";
	}
}
