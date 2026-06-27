#include "get_methods.h"

#include "methods/all.h"
#include "methods/rusty/all.h"

#include "singlewinner/all.h"

// TODO: Refactor the single-winner method stuff below, move the selection
// into singlewinner/get_methods and have this produce bloc multiwinner
// methods from singlewinner ones.
std::vector<std::shared_ptr<multiwinner_method> > get_bloc_methods(
	std::vector<std::shared_ptr<election_method> > &
	singlewinner_methods) {

	// Turn the input methods into bloc multiwinner methods.

	// I have to do it like this, sorta, because the multiwinner
	// library doesn't include the singlewinner code. Maybe I should
	// just lump it all into one thing???

	return {}; // TODO output more here.
}

std::vector<std::shared_ptr<multiwinner_method> > get_multiwinner_methods(
	bool extensive_param_sweep,
	bool expensive_experimental_methods) {

	// Set up some majoritarian election methods and their stats.
	// Condorcet methods should probably have their own arrays.
	// Really, it's pairwise methods that should.
	std::vector<std::shared_ptr<multiwinner_method> > methods;
	std::vector<std::shared_ptr<positional> > positional_methods;
	std::vector<std::shared_ptr<pairwise_method> > condorcet;
	std::vector<std::shared_ptr<election_method> > other_methods;

	// Set the random ballot method's seed to 1 for reproducibility.
	methods.push_back(std::make_shared<random_ballots>
		(1));

	// All are PT_WHOLE for now.
	positional_methods.push_back(std::make_shared<plurality>(PT_WHOLE));
	positional_methods.push_back(std::make_shared<borda>(PT_WHOLE));
	positional_methods.push_back(std::make_shared<antiplurality>(PT_WHOLE));
	positional_methods.push_back(std::make_shared<for_and_against>(PT_WHOLE));
	positional_methods.push_back(std::make_shared<nauru>(PT_WHOLE));
	positional_methods.push_back(std::make_shared<heismantrophy>(PT_WHOLE));
	positional_methods.push_back(std::make_shared<baseballmvp>(PT_WHOLE));
	positional_methods.push_back(std::make_shared<worstplur>(PT_WHOLE));
	positional_methods.push_back(std::make_shared<worstantiplur>(PT_WHOLE));

	size_t counter;

	// Then up it by loser elimination..
	for (counter = 0; counter < positional_methods.size(); ++counter)
		other_methods.push_back(std::make_shared<loser_elimination>(
				positional_methods[counter], false,
				true));

	for (counter = 0; counter < positional_methods.size(); ++counter)
		other_methods.push_back(std::make_shared<loser_elimination>(
				positional_methods[counter],
				true, true));

	// Add some it doesn't make any sense to have as *-Elimination or that
	// haven't got this implemented yet.

	/*other_methods.push_back(new bucklin(PT_WHOLE));
	other_methods.push_back(new bucklin(PT_WHOLE, 0.25));
	other_methods.push_back(new qltd(PT_WHOLE));
	other_methods.push_back(new qltd(PT_WHOLE, 0.25));*/

	other_methods.push_back(std::make_shared<cardinal_ratings>(-10, 10,
			false));
	other_methods.push_back(std::make_shared<cardinal_ratings>(-10, 10, true));

	// These are not all Condorcet. Minmin is not.
	// TODO: Better name for the vector.
	condorcet.push_back(std::make_shared<ext_minmax>(CM_MARGINS, true));
	condorcet.push_back(std::make_shared<ext_minmax>(CM_MARGINS, false));
	condorcet.push_back(std::make_shared<ext_minmax>(CM_PAIRWISE_OPP, true));
	condorcet.push_back(std::make_shared<ext_minmax>(CM_PAIRWISE_OPP, false));
	//condorcet.push_back(std::make_shared<schulze>(CM_WV));
	condorcet.push_back(std::make_shared<ranked_pairs>(CM_WV, false));
	condorcet.push_back(std::make_shared<ranked_pairs>(CM_MARGINS, false));
	condorcet.push_back(std::make_shared<ranked_pairs>(CM_MARGINS, true));
	condorcet.push_back(std::make_shared<max_ab>(CM_PAIRWISE_OPP));

	for (counter = 0; counter < condorcet.size(); ++counter) {
		other_methods.push_back(condorcet[counter]);
	}

	// Now derive multiwinner methods off these
	for (counter = 0; counter < positional_methods.size(); ++counter) {
		methods.push_back(
			std::make_shared<majoritarian_council>(
				positional_methods[counter]));
	}

	for (counter = 0; counter < other_methods.size(); ++counter) {
		methods.push_back(
			std::make_shared<majoritarian_council>(
				other_methods[counter]));
	}

	for (counter = 0; counter < positional_methods.size(); ++counter) {
		methods.push_back(
			std::make_shared<addt_ballot_reweighting>(
				positional_methods[counter]));
		methods.push_back(
			std::make_shared<mult_ballot_reweighting>(
				positional_methods[counter]));
	}

	// Maybe: IRV-SNTV

	for (counter = 0; counter < condorcet.size(); ++counter) {
		methods.push_back(
			std::make_shared<reweighted_condorcet>(
				condorcet[counter]));
	}

	// TODO: Add Meek STV shortcuts.
	//methods.push_back(std::make_shared<qltd_pr>(false, false, false)));
	//methods.push_back(std::make_shared<qltd_pr>(false, false, true)));
	methods.push_back(std::make_shared<qltd_pr>(false,
			true, false));
	methods.push_back(std::make_shared<qltd_pr>(false,
			true, true));
	//methods.push_back(std::make_shared<qltd_pr>(true, false, false)));
	//methods.push_back(std::make_shared<qltd_pr>(true, false, true)));
	methods.push_back(std::make_shared<qltd_pr>(true, true,
			false));
	methods.push_back(std::make_shared<qltd_pr>(true, true,
			true));

	methods.push_back(std::make_shared<old_qltd_pr>());

	methods.push_back(std::make_shared<set_webster>());

	// Only enable this if we can handle the extremely large structure
	// it requires.
	methods.push_back(std::make_shared<PSC>());
	methods.push_back(std::make_shared<PSC>(0.5));
	methods.push_back(std::make_shared<PSC>(0));

	methods.push_back(std::make_shared<set_pr_bucklin>());

	// Not as good as STV. "Plurality PSC".
	methods.push_back(
		std::make_shared<coalition_elimination>(positional_methods[0]));
	methods.push_back(
		std::make_shared<coalition_elimination>(positional_methods[0], 0.5));
	methods.push_back(
		std::make_shared<coalition_elimination>(positional_methods[0], 0));

	methods.push_back(std::make_shared<STV>(BTR_NONE));
	methods.push_back(std::make_shared<STV>(BTR_PLUR));
	methods.push_back(std::make_shared<STV>(BTR_COND)); // Schulze
	methods.push_back(std::make_shared<MeekSTV>(true));
	methods.push_back(std::make_shared<MeekSTV>(false));
	/**/

	methods.push_back(std::make_shared<QPQ>(-1, false));
	methods.push_back(std::make_shared<QPQ>(-1, true));

	if (extensive_param_sweep) {
		for (int x = 1; x <= 10; ++x) {
			methods.push_back(
				std::make_shared<QPQ>(x*0.1, false));
		}

		for (int x = 1; x <= 10; ++x) {
			methods.push_back(
				std::make_shared<QPQ>(x*0.1, true));
		}
	} else {
		methods.push_back(std::make_shared<QPQ>(-1, false));
		methods.push_back(std::make_shared<QPQ>(-1, true));
		methods.push_back(std::make_shared<QPQ>(0.1, false));
		methods.push_back(std::make_shared<QPQ>(0.1, true));
		methods.push_back(std::make_shared<QPQ>(0.5, false));
		methods.push_back(std::make_shared<QPQ>(0.5, true));
		methods.push_back(std::make_shared<QPQ>(1, false));
		methods.push_back(std::make_shared<QPQ>(1, true));
	}
	methods.push_back(std::make_shared<QPQ>(0.01, false));
	methods.push_back(std::make_shared<QPQ>(0.01, true));

	methods.push_back(std::make_shared<r_auction>(true));
	methods.push_back(std::make_shared<r_auction>(false));
	methods.push_back(std::make_shared<LRangeSTV>());
	methods.push_back(std::make_shared<QRangeSTV>());

	// Put on hold until we can do something about the huge memory
	// demands.
	methods.push_back(std::make_shared<SchulzeSTV>());
	methods.push_back(std::make_shared<SchulzePropOrdering>());

	// Not very good. Takes time, too.
	// And these were supposed to be "very good indeed"! What failed?
	// (Whatever I do, I get lousy results.)

	methods.push_back(std::make_shared<birational>());
	methods.push_back(std::make_shared<log_penalty>());
	methods.push_back(std::make_shared<isoelastic>());

	if (extensive_param_sweep) {
		for (int x = 0; x <= 100; ++x) {
			methods.push_back(
				std::make_shared<psi_voting>(x*0.01));
		}

		for (int x = 0; x <= 100; ++x) {
			methods.push_back(
				std::make_shared<harmonic_voting>(x*0.01));
		}

		for (int x = 0; x <= 100; ++x) {
			methods.push_back(
				std::make_shared<sequential_harmonic_voting>(
					x * 0.01));
		}
	} else {
		methods.push_back(
			std::make_shared<psi_voting>(0));
		methods.push_back(
			std::make_shared<psi_voting>(0.5));
		methods.push_back(
			std::make_shared<psi_voting>(1));
		methods.push_back(
			std::make_shared<harmonic_voting>(0));
		methods.push_back(
			std::make_shared<harmonic_voting>(0.5));
		methods.push_back(
			std::make_shared<harmonic_voting>(1));
		methods.push_back(
			std::make_shared<sequential_harmonic_voting>(0));
		methods.push_back(
			std::make_shared<sequential_harmonic_voting>(0.5));
		methods.push_back(
			std::make_shared<sequential_harmonic_voting>(1));
	}

	methods.push_back(std::make_shared<harmonic_voting>
		(10));
	methods.push_back(std::make_shared<harmonic_voting>
		(100));
	methods.push_back(std::make_shared<harmonic_voting>
		(1000));
	methods.push_back(std::make_shared<harmonic_voting>
		(100000));
	methods.push_back(std::make_shared<harmonic_voting>
		(1e9));

	// Not the most elegant way to parameterize.
	if (extensive_param_sweep) {
		for (double p = -10; p < 10; p += 0.1) {
			methods.push_back(
				std::make_shared<isoelastic>(isoelastic_eval(p)));
		}
	} else {
		methods.push_back(std::make_shared<isoelastic>(
				isoelastic_eval(-0.5)));
		methods.push_back(std::make_shared<isoelastic>(
				isoelastic_eval(0)));
		methods.push_back(std::make_shared<isoelastic>(
				isoelastic_eval(2)));
	}

	methods.push_back(std::make_shared<log_penalty>(
			log_penalty_eval(1)));
	methods.push_back(std::make_shared<log_penalty>(
			log_penalty_eval(10)));
	methods.push_back(std::make_shared<log_penalty>(
			log_penalty_eval(1000)));

	if (expensive_experimental_methods) {
		// The Kemeny methods below are *very* slow.

		methods.push_back(
			std::make_shared<fc_kemeny>(true)); // CFC-Kemeny

		// These don't seem as good as CFC.
		//methods.push_back(
		//	std::make_shared<fc_kemeny>(false)); // FC-Kemeny

		//methods.push_back(
		//	std::make_shared<mw_kemeny2_34e>());

		// MW-Kemeny
		//methods.push_back(
		//	std::make_shared<mw_kemeny>());
	}

	// Slow but okay quality values. May be something to investigate further,
	// later, since it's much better than my recent attempt at
	// reimplementing Set Webster.

	/*

	methods.push_back(std::make_shared<mono_webster_640>(
			MM_INVERTED, false, false));
	methods.push_back(std::make_shared<mono_webster_640>(
			MM_PLUSHALF, false, false));
	methods.push_back(std::make_shared<mono_webster_c37>(
			false, MMC_PLUSONE, true, false));
	*/

	return methods;
}
