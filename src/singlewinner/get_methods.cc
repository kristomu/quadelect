#include "get_methods.h"

std::vector<std::shared_ptr<pairwise_method> > get_pairwise_methods(
	const std::vector<pairwise_type> & types,
	bool include_experimental) {

	// For each type, and for each pairwise method, dump that combination
	// to the output. Possible feature request: have the method determine
	// if it supports the type in question (e.g. types that can give < 0
	// wrt stuff like Keener).

	std::vector<std::shared_ptr<pairwise_method> > out;

	for (std::vector<pairwise_type>::const_iterator pos = types.begin(); pos !=
		types.end(); ++pos) {
		out.push_back(std::make_shared<kemeny>(*pos));
		out.push_back(std::make_shared<maxmin>(*pos));
		out.push_back(std::make_shared<schulze>(*pos));

		// Reasonable convergence defaults.
		if (pos->get() == CM_WV || pos->get() == CM_MARGINS
			|| pos->get() == CM_PAIRWISE_OPP) {
			out.push_back(std::make_shared<dquick>(*pos));
			out.push_back(std::make_shared<sinkhorn>(*pos, 0.01, true));
			out.push_back(std::make_shared<sinkhorn>(*pos, 0.01, false));
			out.push_back(std::make_shared<hits>(*pos, 0.001));
			out.push_back(std::make_shared<odm_atan>(*pos, 0.001));
			out.push_back(std::make_shared<odm_tanh>(*pos, 0.001));
			out.push_back(std::make_shared<odm>(*pos, 0.001));

			// Keener
			out.push_back(std::make_shared<keener>(*pos, 0.001, false, false));
			out.push_back(std::make_shared<keener>(*pos, 0.001, false, true));
			out.push_back(std::make_shared<keener>(*pos, 0.001, true, false));
			out.push_back(std::make_shared<keener>(*pos, 0.001, true, true));
		}

		out.push_back(std::make_shared<ext_minmax>(*pos, false));
		out.push_back(std::make_shared<ext_minmax>(*pos, true));
		out.push_back(std::make_shared<ord_minmax>(*pos));

		if (include_experimental) {
			out.push_back(std::make_shared<tup>(*pos, TUP_TUP));
			out.push_back(std::make_shared<tup>(*pos, TUP_SV));
			out.push_back(std::make_shared<tup>(*pos, TUP_MIN));
			out.push_back(std::make_shared<tup>(*pos, TUP_ALT_1));
			out.push_back(std::make_shared<tup>(*pos, TUP_ALT_2));
			out.push_back(std::make_shared<tup>(*pos, TUP_ALT_3));
			out.push_back(std::make_shared<tup>(*pos, TUP_ALT_4));
			out.push_back(std::make_shared<tup>(*pos, TUP_ALT_5));
			out.push_back(std::make_shared<tup>(*pos, TUP_ALT_6));
		}
	}

	out.push_back(std::make_shared<copeland>(CM_WV, 2, 2, 1));
	out.push_back(std::make_shared<copeland>(CM_WV, 2, 1, 0));
	out.push_back(std::make_shared<randpair>(CM_WV));

	return out;
}

// Positional methods.

std::vector<std::shared_ptr<positional> > get_positional_methods(
	bool truncate) {
	// Put that elsewhere?
	std::vector<positional_type> types;
	if (truncate)
		for (int p = PT_FIRST; p <= PT_LAST; ++p) {
			types.push_back(positional_type(p));
		}
	// No point in using both if you aren't going to truncate.
	else {
		types.push_back(positional_type(PT_FRACTIONAL));
	}

	std::vector<std::shared_ptr<positional> > pmethods;

	pmethods.push_back(std::make_shared<bucklin>());

	for (std::vector<positional_type>::const_iterator pos = types.begin();
		pos !=
		types.end(); ++pos) {
		pmethods.push_back(std::make_shared<plurality>(*pos));
		pmethods.push_back(std::make_shared<ext_plurality>(*pos));
		pmethods.push_back(std::make_shared<borda>(*pos));
		pmethods.push_back(std::make_shared<antiplurality>(*pos));
		pmethods.push_back(std::make_shared<ext_antiplurality>(*pos));
		pmethods.push_back(std::make_shared<for_and_against>(*pos));
		pmethods.push_back(std::make_shared<nauru>(*pos));
		pmethods.push_back(std::make_shared<heismantrophy>(*pos));
		pmethods.push_back(std::make_shared<baseballmvp>(*pos));
		pmethods.push_back(std::make_shared<eurovision>(*pos));
		pmethods.push_back(std::make_shared<dabagh>(*pos));
		pmethods.push_back(std::make_shared<nrem>(*pos));
		pmethods.push_back(std::make_shared<worstpos>(*pos));
		pmethods.push_back(std::make_shared<worstborda>(*pos));
	}

	return (pmethods);
}

std::vector<std::shared_ptr<pairwise_method> > get_pairwise_sets() {

	std::vector<std::shared_ptr<pairwise_method> > pw_sets;

	pw_sets.push_back(std::make_shared<condorcet_nonloser_set>());
	pw_sets.push_back(std::make_shared<condorcet_set>());
	pw_sets.push_back(std::make_shared<mdd_set>(true));
	pw_sets.push_back(std::make_shared<mdd_set>(false));
	pw_sets.push_back(std::make_shared<partition_set>(false));

	pw_sets.push_back(std::make_shared<cdtt_set>());
	pw_sets.push_back(std::make_shared<cgtt_set>());
	pw_sets.push_back(std::make_shared<landau_set>());
	pw_sets.push_back(std::make_shared<schwartz_set>());
	pw_sets.push_back(std::make_shared<smith_set>());

	// Copeland fails resolvability, so it can kinda be seen
	// as a set.
	pw_sets.push_back(std::make_shared<copeland>(CM_WV));

	// I'm not sure that these work; they need tests before
	// I'm sure. Disabled for now.
	/*
		pw_sets.push_back(std::make_shared<sdom_set>());
		pw_sets.push_back(std::make_shared<pdom_set>());
	*/

	return pw_sets;
}

std::vector<std::shared_ptr<election_method> > get_sets() {
	std::vector<std::shared_ptr<election_method> > sets;

	for (std::shared_ptr<election_method> em_set: get_pairwise_sets()) {
		sets.push_back(em_set);
	}

	sets.push_back(std::make_shared<mutual_majority_set>());
	sets.push_back(std::make_shared<dmt_set>());
	sets.push_back(std::make_shared<inner_burial_set>());

	return sets;
}

std::vector<std::shared_ptr<election_method> > get_singlewinner_methods(
	bool truncate, bool include_experimental) {

	std::vector<std::shared_ptr<election_method> > all_methods;

	std::vector<std::shared_ptr<pairwise_method> > pairwise =
		get_pairwise_methods(pairwise_producer().provide_all_strategies(),
			include_experimental);

	copy(pairwise.begin(), pairwise.end(), back_inserter(all_methods));

	std::vector<std::shared_ptr<positional> > positional_methods =
		get_positional_methods(truncate);
	std::vector<std::shared_ptr<election_method> > sets = get_sets();

	// We have to do it in this clumsy manner because of possible bugs in
	// handling methods with some candidates excluded.
	std::vector<std::shared_ptr<election_method> > posnl_expanded =
		expand_meta(positional_methods, sets, true);

	// Gradual Condorcet-Borda with different bases, not just Condorcet.
	// The completion method doesn't really matter. Also, MDD* doesn't
	// really work here, and sets that can't handle negatives shouldn't
	// be applied to it.
	for (std::shared_ptr<pairwise_method> pw_set : get_pairwise_sets()) {
		all_methods.push_back(std::make_shared<gradual_cond_borda>(
				pw_set, false, GF_BOTH));
		all_methods.push_back(std::make_shared<gradual_cond_borda>(
				pw_set, true, GF_BOTH));
	}

	all_methods.push_back(std::make_shared<young>(true, true));
	all_methods.push_back(std::make_shared<young>(false, true));
	all_methods.push_back(std::make_shared<first_pref_copeland>());
	all_methods.push_back(std::make_shared<random_ballot>());
	all_methods.push_back(std::make_shared<random_candidate>());
	all_methods.push_back(std::make_shared<cardinal_ratings>(0, 10, false));
	all_methods.push_back(std::make_shared<cardinal_ratings>(0, 10, true));
	all_methods.push_back(std::make_shared<mode_ratings>());
	all_methods.push_back(std::make_shared<vi_median_ratings>(10, false,
			false));
	all_methods.push_back(std::make_shared<vi_median_ratings>(10, false,
			true));
	all_methods.push_back(std::make_shared<vi_median_ratings>(10, true,
			false));
	all_methods.push_back(std::make_shared<vi_median_ratings>(10, true, true));
	all_methods.push_back(std::make_shared<dsc>());
	all_methods.push_back(std::make_shared<ifpp_like_fpa_fpc>());
	all_methods.push_back(std::make_shared<fpa_sum_fpc>());
	all_methods.push_back(std::make_shared<fpa_max_fpc>());
	all_methods.push_back(std::make_shared<quick_runoff>());
	all_methods.push_back(std::make_shared<contingent_vote>());
	all_methods.push_back(std::make_shared<adjusted_cond_plur>());
	// Kevin Venzke's TTR variation of ACP.
	all_methods.push_back(std::make_shared<adjusted_cond_plur>(
			std::make_shared<contingent_vote>()));
	all_methods.push_back(std::make_shared<ifpp_method_x>());
	all_methods.push_back(std::make_shared<no_elimination_irv>());
	// And a few common elimination methods.
	all_methods.push_back(std::make_shared<instant_runoff_voting>(PT_WHOLE,
			true));
	all_methods.push_back(std::make_shared<baldwin>(PT_WHOLE, true));
	all_methods.push_back(std::make_shared<coombs>(PT_WHOLE, true));
	all_methods.push_back(std::make_shared<ifpp>(PT_WHOLE, true));
	all_methods.push_back(std::make_shared<nanson>(PT_WHOLE, true));

	all_methods.push_back(std::make_shared<donated_contingent_vote>());

	if (include_experimental) {
		for (int i = 0; i < TEXP_TOTAL; ++i) {
			all_methods.push_back(std::make_shared<three_experimental>((texp_type)i));
		}

		all_methods.push_back(std::make_shared<sv_att_second>());
		all_methods.push_back(std::make_shared<fpa_experiment>());
		//all_methods.push_back(std::make_shared<donated_contingent_vote>());

		// Maybe expand later
		all_methods.push_back(std::make_shared<cte>(std::make_shared<plurality>
				(PT_WHOLE)));
		all_methods.push_back(std::make_shared<cte>(std::make_shared<borda>
				(PT_WHOLE)));

		all_methods.push_back(std::make_shared<rmr1>(RMR_DEFEATED));
		all_methods.push_back(std::make_shared<rmr1>(RMR_DEFEATING));
		all_methods.push_back(std::make_shared<rmr1>(RMR_TWO_WAY));
		all_methods.push_back(std::make_shared<rmr1>(RMR_SCHWARTZ_EXP));

		// This has interesting almost-monotone properties.
		all_methods.push_back(std::make_shared<comma>(
				std::make_shared<rmr1>(RMR_DEFEATING),
				std::make_shared<comma>(
					std::make_shared<schwartz_set>(),
					std::make_shared<ext_plurality>(PT_WHOLE))));
	}

	// Then expand:
	// I can't keep myself. Make loser-elimination variants of everything
	// we've got so far.
	// NOTE: This is *very* experimental. May uncover bugs. For anything
	// more stable, don't do this one.
	// TODO: Make tests to verify that methods behave similarly when
	// elimination is done the "hard way" as when done by setting hopefuls
	// variables to false.
	std::vector<std::shared_ptr<election_method> > expanded = expand_meta(
			all_methods, sets, true);

	// Finally add some sets, because these should not be used as bases
	// for elimination methods.
	copy(sets.begin(), sets.end(), back_inserter(all_methods));

	// and

	copy(positional_methods.begin(), positional_methods.end(),
		back_inserter(all_methods));
	copy(posnl_expanded.begin(), posnl_expanded.end(),
		back_inserter(all_methods));
	copy(expanded.begin(), expanded.end(), back_inserter(all_methods));

	// Done!
	return all_methods;
}
