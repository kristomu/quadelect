// This class keeps track of the random and optimal values of
// what we want to get the VSE of. Being able to specify the
// direction of optimality makes it possible to use this both for
// disproportionality (higher is worse) and utility (higher is better).

// TODO? Refactor some of the singlewinner VSE stuff to use this?

enum optimality_direction { MINIMIZE, MAXIMIZE };

class VSE_limits {
	private:
		double worst, random, best;
		bool seen_worst, seen_best;
		size_t times_updated;
		optimality_direction opt_dir;

	public:
		void clear() {
			worst = 0;
			random = 0;
			best = 0;
			times_updated = 0;
			seen_worst = false;
			seen_best = false;
		}

		bool is_better(double new_val, double old_val) const {
			return ((opt_dir == MAXIMIZE) && new_val > old_val) ||
				((opt_dir == MINIMIZE) && new_val < old_val);
		}

		bool is_worse(double new_val, double old_val) const {
			return ((opt_dir == MAXIMIZE) && new_val < old_val) ||
				((opt_dir == MINIMIZE) && new_val > old_val);
		}

		void update(double current_value) {
			if (!seen_worst || is_worse(current_value, worst)) {
				worst = current_value;
				seen_worst = true;
			}

			if (!seen_best || is_better(current_value, best)) {
				best = current_value;
				seen_best = true;
			}

			random += current_value;
			++times_updated;
		}

		VSE_limits(optimality_direction opt_dir_in) {
			clear();
			opt_dir = opt_dir_in;
		}

		double get_worst() const {
			return worst;
		}
		double get_random() const {
			return random/(double)times_updated;
		}
		double get_best() const {
			return best;
		}
};