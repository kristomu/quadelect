#pragma once

#include <set>
#include <string>
#include <vector>

// After permuting according to cand_permutation, we end up in
// to_scenario.

struct isomorphism {
	std::vector<std::vector<int> > cand_permutations;
	copeland_scenario to_scenario;
	bool canonical;
};

// I need better names for the variables.

class fixed_cand_equivalences {
	private:
		// This map gives the equivalence relation between scenarios under
		// the constraint that we're not allowed to change who candidate A
		// is. As the gen_custom_functs all calculate scores from the
		// perspective of A, the equivalences here go between scenarios
		// that can in some sense be reduced to one scenario when testing
		// gen_custom_functs.
		std::map<copeland_scenario, isomorphism>
			noncanonical_scenario_reductions;

		// candidate_remappings[x][scenario] gives what scenarios we can
		// turn the input scenario into (and how) if we want the xth candidate
		// to become A. These are used for combining gen_custom_functs
		// into an election method. Any election with the correct number of
		// candidates will have a scenario that belongs to one of the
		// derived_scenario_reductions equivalence classes. If we call one
		// scenario per equivalence class "canonical" for that class,
		// and we want to calculate the score of the xth candidate with a
		// gen_custom_funct that only calculates the score of A, we can
		// permute the xth candidate into A by looking up
		// candidate_remappings[x][scenario]. Note that only canonical
		// scenarios are represented.
		std::vector<std::map<copeland_scenario, isomorphism> >
			candidate_remappings;

		// This map gives the equivalence relation between scenarios under
		// no constraint, i.e. we don't care about how the candidates are
		// relabeled. The vector is a list of which scenarios can be reached
		// by permuting the candidates in an election with the input scenario.
		std::map<copeland_scenario, std::vector<copeland_scenario> >
			scenario_equivalences;

		size_t num_candidates;

		// Helper function.
		std::vector<std::vector<int> > all_permutations_centered_on(int cand,
			size_t numcands) const;

		std::map<copeland_scenario, isomorphism>
			get_noncanonical_scenario_reductions(size_t numcands,
			bool verbose) const;

		std::map<copeland_scenario, isomorphism> get_one_candidate_remapping(
			size_t numcands, int current_candidate,
			const std::map<copeland_scenario, isomorphism> &
			canonical_reductions) const;

		std::vector<std::map<copeland_scenario, isomorphism> >
			get_all_candidate_remappings(size_t numcands,
			const std::map<copeland_scenario, isomorphism> &
			derived_reductions) const;

		void initialize(size_t numcands) {
			noncanonical_scenario_reductions =
				get_noncanonical_scenario_reductions(num_candidates,
					false);

			candidate_remappings = get_all_candidate_remappings(
				num_candidates, noncanonical_scenario_reductions);

			// scenario_equivalences
		}

	public:
		void set_num_candidates(size_t numcands) {
			num_candidates = numcands;
			initialize(num_candidates);
		}

		size_t get_num_candidates() const { return num_candidates; }

		fixed_cand_equivalences(size_t numcands) {
			set_num_candidates(numcands);
		}

		isomorphism get_canonical_isomorphism(
			const copeland_scenario & scenario) const {
			return noncanonical_scenario_reductions.find(scenario)->second;
		}

		isomorphism get_candidate_remapping(
			const copeland_scenario & desired_scenario,
			size_t candidate_to_become_A) const {

			// TODO: Determine if trying to access data from an end() iterator
			// automatically gives an exception, or undefined behavior.
			// It does not give exception. TODO: Throw one if ctbA is out
			// of bounds or desired scenario couldn't be found.

			return candidate_remappings[candidate_to_become_A].
				find(desired_scenario)->second;
		}

		// To remove later: need to figure out how iterators work first.

		const std::map<copeland_scenario, isomorphism> & get_noncanonical_scenarios() const {
			return noncanonical_scenario_reductions;
		}

		// Ditto, these are scaffolds.
		const std::vector<std::map<copeland_scenario, isomorphism> > &
			get_cand_remaps() const {
				return candidate_remappings;
			}
};