// This class takes a number of solutions as an input, as well as their
// performance on multiple dimensions, and updates a region that consists of
// every VSE that could be optained by choosing one of these solutions every
// round.

// For instance, the solutions may be every council set that is consistent
// with the Droop proportionality criterion; the output then gives the whole
// region that could (theoretically) be achieved by constraining oneself to
// pass the DPC.

// Because storing tons of points is very expensive, and the updating function
// has to combine every existing point with every new point, the points are
// quantized. Updating points creates a new collection of points. They are
// binned in n-space to produce "voxels"; multiple points within the same
// hypercube are discarded before the new point cloud is admitted.

#include "vse.h"

#include <map>

class vse_region_mapper {
	private:
		std::map<std::vector<double>, VSE_point>
		VSE_cloud, next_VSE_cloud;

		// If the region is n-dimensional, then the hypercube
		// [0..1]^n will contain at most elements_per_dimension^n
		// different points when run through the mapper.
		// VSE is likely to be within this range for most good
		// methods, though it can strictly speaking go outside
		// the hypercube for methods that are worse at the given
		// property than random selection is.
		// Elements_per_dimension controls granularity and space
		// complexity. The higher, the more fine-grained and the
		// more space required.
		int elements_per_dimension;

		// Return the Euclidean distance between a point's
		// real and quantized coordinates.
		std::vector<double> get_quantized_position(
			const VSE_point & pt) const;

		double get_quantization_error(
			const VSE_point & pt) const;

		// For enforcing additional constraints, e.g. Pareto frontier.
		virtual std::map<std::vector<double>, VSE_point>
		filter_augmented_points(const
			std::map<std::vector<double>, VSE_point> & proposed) const {
			return proposed;
		}

	public:
		// Call this with a vector of VSEs; the mapper will use
		// their last round results. Each n-vector defines a new
		// point in n-space, so this would be called once per
		// new point.
		void add_point(const VSE_point & cur_round_VSE);
		// Then call this to update the cloud.
		void update();

		// return the points gathered so far.
		std::vector<VSE_point> get() const;

		vse_region_mapper(int elements_per_dim_in) {
			elements_per_dimension = elements_per_dim_in;

			// Add a null point so that we don't have to special-
			// case a bunch of stuff in add_point.
			VSE_cloud[ {}] = VSE();
		}
};