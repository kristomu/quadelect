// Different simply implemented positional voting methods.

#ifndef _VOTE_POS_SIMPLE
#define _VOTE_POS_SIMPLE

#include "../../ballots.h"
#include "../method.h"
#include "as241.h"
#include <list>
#include <vector>

#include "positional.h"

using namespace std;

// This one is used for Bucklin and QLTD. Not. We can still use it for
// "fractional approval". TODO: Make a true Bucklin wrapper around it.
class sweep : public positional {
	private:
		double sweep_point;

	protected:
		size_t zero_run_beginning() const {
			return (ceil(sweep_point));
		}
		double pos_weight(size_t position, size_t last_position) const {
			if (position < ceil(sweep_point)) {
				if (position > floor(sweep_point)) {
					return (sweep_point-floor(sweep_point));
				} else	{
					return (1);
				}
			} else {
				return (0);
			}
		}

		string pos_name() const {
			return ((string)"Sweep(" + dtos(sweep_point) + ")");
		}

	public:
		sweep(positional_type kind_in, double sweep_setting) :
			positional(kind_in) {
			sweep_point = sweep_setting;
		}
};

// Example of how to make a positional method.

class plurality : public positional {
	protected:
		size_t zero_run_beginning() const {
			return (1);
		}
		double pos_weight(size_t position, size_t last_position) const {
			if (position == 0) {
				return (1);
			} else {
				return (0);
			}
		}
		string pos_name() const {
			return ("Plurality");
		}
	public:
		plurality(positional_type kind_in) : positional(kind_in) {}
};

class borda : public positional {
	protected:
		double pos_weight(size_t position, size_t last_position) const {
			return (last_position - position);
		}
		string pos_name() const {
			return ("Borda");
		}

	public:
		borda(positional_type kind_in) : positional(kind_in) {}
};

class antiplurality : public positional {
	protected:
		double pos_weight(size_t position, size_t last_position) const {
			if (position == last_position) {
				return (0);
			} else {
				return (1);
			}
		}
		string pos_name() const {
			return ("Antiplurality");
		}

	public:
		antiplurality(positional_type kind_in) : positional(kind_in) {}
};

class for_and_against : public positional {
	protected:
		double pos_weight(size_t position, size_t last_position) const {
			if (position == 0) {
				return (1);
			}
			if (position == last_position) {
				return (-1);
			}
			return (0);
		}
		string pos_name() const {
			return ("VoteForAgainst");
		}

	public:
		for_and_against(positional_type k_i) : positional(k_i) {}
};

// Some others that are also in IEVS

class nauru : public positional {
	protected:
		double pos_weight(size_t position, size_t last_position) const {
			return (1/(double)(position +1));
		}

		string pos_name() const {
			return ("Nauru Borda");
		}

	public:
		nauru(positional_type kind_in) : positional(kind_in) {}
};

class heismantrophy : public positional {
	protected:
		size_t zero_run_beginning() const {
			return (3);
		}
		double pos_weight(size_t position, size_t last_position) const {
			if (position < 3) {
				return (3 - position);
			}
			return (0);
		}

		string pos_name() const {
			return ("Heisman Trophy");
		}

	public:
		heismantrophy(positional_type kind_in) : positional(kind_in) {}
};

class baseballmvp : public positional {
	protected:
		size_t zero_run_beginning() const {
			return (10);
		}
		double pos_weight(size_t position, size_t last_position) const {
			if (position == 0) {
				return (14);
			}
			if (position < 10) {
				return (10 - position);
			}
			return (0);
		}

		string pos_name() const {
			return ("Baseball MVP");
		}

	public:
		baseballmvp(positional_type kind_in) : positional(kind_in) {}
};

class eurovision : public positional {
	protected:
		size_t zero_run_beginning() const {
			return (10);
		}
		double pos_weight(size_t position, size_t last_position) const {
			// 12 10 8 7 ...
			if (position == 0) {
				return (12);
			}
			if (position == 1) {
				return (10);
			}
			if (position < 10) {
				return (10-position);
			}
			return (0);
		}

		string pos_name() const {
			return ("Eurovision");
		}

	public:
		eurovision(positional_type kind_in) : positional(kind_in) {}
};

class dabagh : public positional {
	protected:
		size_t zero_run_beginning() const {
			return (2);
		}
		double pos_weight(size_t position, size_t last_position) const {
			// 2 1 0 0 0 ...
			if (position >= 2) {
				return (0);
			}
			return (2 - position);
		}

		string pos_name() const {
			return ("Dabagh");
		}

	public:
		dabagh(positional_type kind_in) : positional(kind_in) {}
};

// Warren's "Optimal NREM" method with a curve-fit approximation for G1^N
class nrem : public positional {
	protected:
		// Note, these are one-indexed, not zero
		double gauss(size_t curcand, size_t num_cands) const;
		// This one is zero-indexed
		double pos_weight(size_t position, size_t last_position) const;

		string pos_name() const {
			return ("NREM-Opt");
		}

	public:
		nrem(positional_type kind_in) : positional(kind_in) {}
};

// A few deliberately perverse ones, to check the dynamics of voter reweighting
// and PR/minority support.

class worstpos : public positional {
	protected:
		double pos_weight(size_t position, size_t last_position) const {
			if (position == last_position) {
				return (1);
			} else {
				return (0);
			}
		}

		string pos_name() const {
			return ("Worst Plurality");
		}

	public:
		worstpos(positional_type kind_in) : positional(kind_in) {}
};

class worstborda : public positional {
	protected:
		double pos_weight(size_t position, size_t last_position) const {
			return (position);
		}
		string pos_name() const {
			return ("Worst Borda");
		}

	public:
		worstborda(positional_type kind_in) : positional(kind_in) {}
};

#endif
