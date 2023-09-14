#pragma once

// The abstract base class for everything that generates coordinates in
// n-dimensional space. We use this to unify random and quasirandom generators
// while respecting the latter's requirement that the dimension has to be
// specified in advance.

#include <cstdint>
#include <vector>
#include <math.h>

class coordinate_gen {
	private:
		// Get integers from [0 ... end).
		template<typename T> std::vector<T> get_integers_T(
			const std::vector<T> end) {

			std::vector<double> coordinate = get_coordinate(
					end.size());

			std::vector<T> lattice_point(end.size());

			for (size_t i = 0; i < end.size(); ++i) {
				lattice_point[i] = round(coordinate[i] * (end.size()-1));
			}

			return lattice_point;
		}

	public:
		// Generates an n-dimensional vector in the hypercube [0..1]^n.
		virtual std::vector<double> get_coordinate(size_t dimension) = 0;

		virtual std::vector<uint64_t> get_longs(
			const std::vector<uint64_t> end) {
			return get_integers_T(end);
		}

		virtual std::vector<uint32_t> get_integers(
			const std::vector<uint32_t> end) {
			return get_integers_T(end);
		}
	public:

		// Aliases for getting values one at a time.
		// Because low discrepancy sequences are not independent,
		// they need to enforce that the caller requests exactly the
		// right number of variates; anything less (or more) will
		// destroy its performance in general. Thus the procedure
		// goes like this:

		// The caller should first call start_query(). Then it should
		// request every required variate, and then it should call
		// end_query(). Rejection sampling would require that the whole
		// sequence is thrown out and everything from the last
		// start_query is redone; it is thus not supported.

		// is_independent is false if we're dealing with a QMC
		// style generator where every coordinate must either be used
		// fully or not at all. It's true for truly independent
		// sources like RNGs.
		virtual bool is_independent() const = 0;

		virtual void start_query() = 0;
		virtual void end_query() = 0;

		virtual double next_double() = 0;
		virtual double next_double(double min, double max);

		// Ranges. Note that these are all half-open, i.e. [begin, end)
		virtual uint64_t next_long() = 0;
		virtual uint64_t next_long(uint64_t modulus) = 0;
		virtual uint64_t next_long(uint64_t begin, uint64_t end);

		virtual uint32_t next_int() = 0;
		virtual uint32_t next_int(uint32_t modulus) = 0;
		virtual uint32_t next_int(uint32_t begin, uint32_t end);

		// Used for random_shuffle etc. Assumes pointers are no longer
		// than 64 bit. Perhaps using () is a bit of a hack, but the
		// alternatives are worse.
		uint64_t operator()(uint64_t end) {
			return next_long(end);
		}

		uint64_t operator()() {
			return next_long();
		}

		// Is this an off-by-one or an opaque mixing of half-open
		// and closed intervals? Should max() mean that we can actually
		// obtain this value or not? Currently it does.

		uint64_t max() const {
			return UINT64_MAX;
		}
		uint64_t min() const {
			return 0;
		}
};