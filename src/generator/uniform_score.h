// EXPERIMENTAL. JGA's impartial culture generalization: random uniform rating.

#pragma once

#include "ballotgen.h"
#include "impartial_gen.h"
#include <list>


class uniform_score : public impartial_gen {

	protected:
		double get_sample(coordinate_gen & coord_source) const {
			return (coord_source.next_double());
		}

	public:
		std::string name() const {
			return ("Uniform score");
		}

		uniform_score(bool compress_in) :
			impartial_gen(compress_in) {}
		uniform_score(bool compress_in, bool do_truncate) :
			impartial_gen(compress_in, do_truncate) {}

};