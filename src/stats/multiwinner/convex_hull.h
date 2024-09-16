// Calculate the convex hull of VSEs. The interface is similar to
// vse_region_mapper's: call add_point or add_points and then update once
// every new point has been added.

// This currently only supports 2D convex hulls; I'll add a more general
// algorithm later if/when I need it.

// I'm also going to hold off on quantization for now. It probably needs
// to happen, but I don't know how to do so without suffering compounding
// losses due to inaccuracy.

// Uses Andrew's monotone chain algorithm with cross product, as seen on
// https://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain

#include "vse.h"

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

class VSE_convex_hull {
	private:
		std::vector<VSE_point_wrapper> convex_hull(
			std::vector<VSE_point_wrapper> & P,
			bool verbose);

	public:
		std::vector<VSE_point_wrapper> hull;
		std::vector<VSE_point_wrapper> new_points;

		void add_point(const VSE_point & cur_round_VSE);
		void add_points(const std::vector<VSE_point> & points);

		double get_area() const;

		VSE_convex_hull() {
			// Add empty VSE so that add_point will work properly.
			hull.resize(1);
		}

		void update() {
			hull = convex_hull(new_points, true);
			new_points.clear();
		}

		void dump_coordinates(std::ostream & where) const;
};
