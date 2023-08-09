#include "get_methods.h"

std::list<pairwise_method *> get_pairwise_methods(
	const std::list<pairwise_type> & types,
	bool include_experimental) {

	// For each type, and for each pairwise method, dump that combination
	// to the output. Possible feature request: have the method determine
	// if it supports the type in question (e.g. types that can give < 0
	// wrt stuff like Keener).

	std::list<pairwise_method *> out;

	for (std::list<pairwise_type>::const_iterator pos = types.begin(); pos !=
		types.end(); ++pos) {
		out.push_back(new kemeny(*pos));
		out.push_back(new maxmin(*pos));
		out.push_back(new schulze(*pos));

		// Reasonable convergence defaults.
		if (pos->get() == CM_WV || pos->get() == CM_MARGINS
			|| pos->get() == CM_PAIRWISE_OPP) {
			out.push_back(new dquick(*pos));
			out.push_back(new sinkhorn(*pos, 0.01, true));
			out.push_back(new sinkhorn(*pos, 0.01, false));
			out.push_back(new hits(*pos, 0.001));
			out.push_back(new odm_atan(*pos, 0.001));
			out.push_back(new odm_tanh(*pos, 0.001));
			out.push_back(new odm(*pos, 0.001));

			// Keener
			out.push_back(new keener(*pos, 0.001, false, false));
			out.push_back(new keener(*pos, 0.001, false, true));
			out.push_back(new keener(*pos, 0.001, true, false));
			out.push_back(new keener(*pos, 0.001, true, true));
		}

		out.push_back(new ext_minmax(*pos, false));
		out.push_back(new ext_minmax(*pos, true));
		out.push_back(new ord_minmax(*pos));

		if (include_experimental) {
			out.push_back(new tup(*pos, TUP_TUP));
			out.push_back(new tup(*pos, TUP_SV));
			out.push_back(new tup(*pos, TUP_MIN));
			out.push_back(new tup(*pos, TUP_ALT_1));
			out.push_back(new tup(*pos, TUP_ALT_2));
			out.push_back(new tup(*pos, TUP_ALT_3));
			out.push_back(new tup(*pos, TUP_ALT_4));
			out.push_back(new tup(*pos, TUP_ALT_5));
			out.push_back(new tup(*pos, TUP_ALT_6));
		}
	}

	// Doesn't matter what these are set to. There should be a test for that
	// so that we can just do an if.

	// Defined elsewhere...
	//out.push_back(new copeland(CM_WV));

	out.push_back(new copeland(CM_WV, 2, 2, 1));
	out.push_back(new copeland(CM_WV, 2, 1, 0));
	out.push_back(new randpair(CM_WV));

	return (out);
}

// Positional methods.

std::list<positional *> get_positional_methods(bool truncate) {
	// Put that elsewhere?
	std::list<positional_type> types;
	if (truncate)
		for (int p = PT_FIRST; p <= PT_LAST; ++p) {
			types.push_back(positional_type(p));
		}
	// No point in using both if you aren't going to truncate.
	else	{
		types.push_back(positional_type(PT_FRACTIONAL));
	}

	std::list<positional *> out;

	for (std::list<positional_type>::const_iterator pos = types.begin(); pos !=
		types.end(); ++pos) {
		out.push_back(new plurality(*pos));
		out.push_back(new ext_plurality(*pos));
		out.push_back(new borda(*pos));
		out.push_back(new antiplurality(*pos));
		out.push_back(new ext_antiplurality(*pos));
		out.push_back(new for_and_against(*pos));
		out.push_back(new nauru(*pos));
		out.push_back(new heismantrophy(*pos));
		out.push_back(new baseballmvp(*pos));
		out.push_back(new eurovision(*pos));
		out.push_back(new dabagh(*pos));
		out.push_back(new nrem(*pos));
		out.push_back(new worstpos(*pos));
		out.push_back(new worstborda(*pos));
	}

	return (out);
}

std::list<pairwise_method *> get_sets() {

	std::list<pairwise_method *> out;

	out.push_back(new condorcet_set());
	out.push_back(new mdd_set(true));
	out.push_back(new mdd_set(false));
	out.push_back(new partition_set(false));

	out.push_back(new cdtt_set());
	out.push_back(new cgtt_set());
	out.push_back(new landau_set());
	out.push_back(new schwartz_set());
	out.push_back(new sdom_set());
	out.push_back(new pdom_set());
	out.push_back(new smith_set());

	out.push_back(new copeland(CM_WV)); // Nudge nudge.

	return (out);
}

std::list<election_method *> get_singlewinner_methods(bool truncate,
	bool include_experimental) {

	std::list<election_method *> toRet;

	std::list<pairwise_method *> pairwise = get_pairwise_methods(
			pairwise_producer().provide_all_strategies(),
			include_experimental);

	copy(pairwise.begin(), pairwise.end(), back_inserter(toRet));

	std::list<positional *> positional_methods = get_positional_methods(
			truncate);
	std::list<pairwise_method *> pairwise_sets = get_sets();

	// We have to do it in this clumsy manner because of possible bugs in
	// handling methods with some candidates excluded.
	std::list<election_method *> posnl_expanded = expand_meta(
			positional_methods,
			pairwise_sets, true);

	// Gradual Condorcet-Borda with different bases, not just Condorcet.
	// The completion method doesn't really matter. Also, MDD* doesn't
	// really work here, and sets that can't handle negatives shouldn't
	// be applied to it.
	for (std::list<pairwise_method *>::const_iterator basis = pairwise_sets.
			begin(); basis != pairwise_sets.end(); ++basis) {
		toRet.push_back(new gradual_cond_borda(*basis, false, GF_BOTH));
		toRet.push_back(new gradual_cond_borda(*basis, true, GF_BOTH));
	}

	toRet.push_back(new young(true, true));
	toRet.push_back(new young(false, true));
	toRet.push_back(new first_pref_copeland());
	toRet.push_back(new random_ballot());
	toRet.push_back(new random_candidate());
	toRet.push_back(new cardinal_ratings(0, 10, false));
	toRet.push_back(new cardinal_ratings(0, 10, true));
	toRet.push_back(new mode_ratings());
	toRet.push_back(new vi_median_ratings(10, false, false));
	toRet.push_back(new vi_median_ratings(10, false, true));
	toRet.push_back(new vi_median_ratings(10, true, false));
	toRet.push_back(new vi_median_ratings(10, true, true));
	toRet.push_back(new dsc());
	toRet.push_back(new ifpp_like_fpa_fpc());
	toRet.push_back(new fpa_sum_fpc());
	toRet.push_back(new fpa_max_fpc());
	toRet.push_back(new quick_runoff());
	toRet.push_back(new contingent_vote());
	toRet.push_back(new adjusted_cond_plur());
	// Kevin Venzke's TTR variation of ACP.
	toRet.push_back(new adjusted_cond_plur(
			std::make_shared<contingent_vote>()));
	toRet.push_back(new ifpp_method_x());
	toRet.push_back(new no_elimination_irv());
	// And a few common elimination methods.
	toRet.push_back(new instant_runoff_voting(PT_WHOLE, true));
	toRet.push_back(new baldwin(PT_WHOLE, true));
	toRet.push_back(new coombs(PT_WHOLE, true));
	toRet.push_back(new ifpp(PT_WHOLE, true));
	toRet.push_back(new nanson(PT_WHOLE, true));

	if (include_experimental) {
		for (int i = 0; i < TEXP_TOTAL; ++i) {
			toRet.push_back(new three_experimental((texp_type)i));
		}

		toRet.push_back(new sv_att_second());
		toRet.push_back(new fpa_experiment());
		toRet.push_back(new donated_contingent_vote());
	}

	// Then expand:
	// I can't keep myself. Make loser-elimination variants of everything
	// we've got so far.
	// NOTE: This is *very* experimental. May uncover bugs. For anything
	// more stable, don't do this one.
	// TODO: Make tests to verify that methods behave similarly when
	// elimination is done the "hard way" as when done by setting hopefuls
	// variables to false.
	std::list<election_method *> expanded = expand_meta(toRet,
			pairwise_sets, true);

	// Finally add some sets, because these should not be used as bases
	// for elimination methods.
	copy(pairwise_sets.begin(), pairwise_sets.end(), back_inserter(toRet));

	// and

	copy(positional_methods.begin(), positional_methods.end(),
		back_inserter(toRet));
	copy(posnl_expanded.begin(), posnl_expanded.end(),
		back_inserter(toRet));
	copy(expanded.begin(), expanded.end(), back_inserter(toRet));

	// Done!
	return (toRet);
}
