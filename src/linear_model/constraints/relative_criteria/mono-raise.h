// Relative criterion constraint generator for mono-raise
// This always raises the first candidate (A).

#include "../relative_criterion.h"

class mono_raise_const : public relative_criterion_const {
	private:
		std::pair<int, std::vector<int> > get_monotonicity_indices(
			const std::vector<int> & permutation, int cand_to_raise) const;

	protected:
		bool permissible_transition(
			const std::vector<int> & before_permutation,
			const std::vector<int> & after_permutation) const;

	public:
		mono_raise_const(int numcands_in) : 
			relative_criterion_const(numcands_in) {}

		std::string name() const { return "Mono-raise"; }
};