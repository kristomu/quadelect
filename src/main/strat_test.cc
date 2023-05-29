#include "strat_test.h"

/*
		int total_generation_attempts;
		election_method * method;
		*/

basic_strategy StrategyTest::strategize_for_election(
	const list<ballot_group> & ballots,
	ordering honest_outcome, size_t numcands, bool verbose) const {

	// NOTE! Does not work if there's a tie!
	// If there's a tie, we shouldn't check challengers who tie the
	// winner, after all; more generally, it's not clear how to handle
	// ties.
	size_t winner = honest_outcome.begin()->get_candidate_num();

	bool strategy_worked = false;
	ordering::const_iterator pos;

	if (verbose) {
		cout << "Trying to strategize for this election: " << endl;
		ballot_tools().print_ranked_ballots(ballots);
		cout << "Honest outcome is " << ordering_tools().ordering_to_text(
				honest_outcome, true) << endl;
	}

	for (size_t challenger = 0; challenger < numcands && !strategy_worked;
		++challenger) {
		if (challenger == winner) {
			continue;
		}

		if (verbose) {
			cout << "Trying to rig in favor of " << challenger << endl;
		}

		list<ballot_group> prefers_winner, prefers_challenger;
		double num_prefers_challenger = 0;

		// Find those who prefer the challenger. Add those that
		// don't into prefers_winner and add up the score of those that
		// do.

		for (list<ballot_group>::const_iterator bgpos = ballots.begin();
			bgpos != ballots.end(); ++bgpos) {
			int saw_winner = -1, saw_challenger = -1;
			int rank = 0;

			for (pos = bgpos->contents.begin(); pos !=
				bgpos->contents.end() &&
				(saw_winner == -1 ||
					saw_challenger == -1); ++pos) {
				if (pos->get_candidate_num() == winner) {
					saw_winner = rank;
				}
				if (pos->get_candidate_num() == challenger) {
					saw_challenger = rank;
				}

				++rank;
			}

			// We consider equal-ranks to be in favor of the winner,
			// thus we only explicitly check if the challenger was
			// rated above the winner.
			if (saw_challenger < saw_winner) {
				num_prefers_challenger += bgpos->weight;
				prefers_challenger.push_back(*bgpos);
			} else {
				prefers_winner.push_back(*bgpos);
			}
		}

		/*cout << num_prefers_challenger << " prefers " << challenger
		    << endl;*/

		if (num_prefers_challenger == 0) {
			continue;
		}
		assert(num_prefers_challenger > 0);

		// TODO: Check some stock strategies here:
		//      Compromising: A>B voters rank A first (B original winner)
		//      Burial: A>B voters rank B last (B original winner)
		//      Two-sided: Both compromising and burial at once.

		// Store the ballots that prefer the winner. These will be augmented by
		// the ballots that prefer the challenger to produce a new election.

		for (int tries = 0; tries < strategy_attempts_per_try && !strategy_worked;
			++tries) {

			// Add the strategic ballot by drawing from IC.

			// The number of distinct ballots that the strategy uses.
			// NOTE: The combined weight of the ballots still equals the
			// number of voters engaged in strategy; this is just about how many
			// "bailiwicks" the vote management center divides the voters into.
			int max_strat_ballots = 1 + tries % 3, q;
			int num_strat_ballots = 0; // Actual number of bailiwicks employed.
			bool already_strategized = false;
			double cumul = 0;

			// Try some standard strategies first.
			switch (tries) {
				case ST_BURIAL: // Burial
					for (ballot_group strategic_ballot: prefers_challenger) {
						if (verbose) {
							cout << "Burial for " << challenger << " against " << winner <<
								": ballot used to be " <<
								ordering_tools().ordering_to_text(strategic_ballot.contents, false);
						}
						strategic_ballot.replace_score(winner,
							strategic_ballot.get_min_score()-1);
						if (verbose) {
							cout << " --> " <<
								ordering_tools().ordering_to_text(strategic_ballot.contents, false)
								<< endl;
						}
						prefers_winner.push_back(strategic_ballot);
					}
					already_strategized = true;
					max_strat_ballots = prefers_challenger.size();
					num_strat_ballots = max_strat_ballots;
					break;
				case ST_COMPROMISING: // Compromise
					for (ballot_group strategic_ballot: prefers_challenger) {
						if (verbose) {
							cout << "Compromise for " << challenger << " against " << winner <<
								": ballot used to be " <<
								ordering_tools().ordering_to_text(strategic_ballot.contents, false);
						}
						strategic_ballot.replace_score(challenger,
							strategic_ballot.get_max_score()+1);
						if (verbose) {
							cout << " --> " <<
								ordering_tools().ordering_to_text(strategic_ballot.contents, false)
								<< endl;
						}
						prefers_winner.push_back(strategic_ballot);
					}
					already_strategized = true;
					max_strat_ballots = prefers_challenger.size();
					num_strat_ballots = max_strat_ballots;
					break;
				case ST_TWOSIDED: // Two-sided
					for (ballot_group strategic_ballot: prefers_challenger) {
						strategic_ballot.replace_score(challenger,
							strategic_ballot.get_max_score()+1);
						strategic_ballot.replace_score(winner,
							strategic_ballot.get_min_score()-1);
						prefers_winner.push_back(strategic_ballot);
					}
					already_strategized = true;
					max_strat_ballots = prefers_challenger.size();
					num_strat_ballots = max_strat_ballots;
					break;
				case ST_REVERSE: {
					// Reverse social order with two-sided strategy
					ballot_group strategic_ballot(num_prefers_challenger,
						ordering_tools().reverse(honest_outcome), true, false);
					strategic_ballot.replace_score(challenger,
						strategic_ballot.get_max_score()+1);
					strategic_ballot.replace_score(winner,
						strategic_ballot.get_min_score()-1);
					prefers_winner.push_back(strategic_ballot);
					already_strategized = true;
					max_strat_ballots = 1;
					num_strat_ballots = max_strat_ballots;
					break;
				}
				default:
					break;
			}

			if (already_strategized && verbose) {
				string strategies[] = {"burial", "compromise", "two-sided", "reverse"};
				cout << "DEBUG: Trying strategy " << strategies[tries] << endl;
				cout << "After transformation, the ballot set looks like this:" << endl;
				ballot_tools().print_ranked_ballots(prefers_winner);
			}

			for (q = 0; q < max_strat_ballots && !already_strategized; ++q) {
				list<ballot_group> strategy;
				while (strategy.empty())
					strategy = strat_generator->generate_ballots(1, numcands,
							*randomizer);
				if (q == max_strat_ballots-1) {
					assert(num_prefers_challenger - cumul > 0);
					strategy.begin()->weight = num_prefers_challenger - cumul;
				} else {
					strategy.begin()->weight = round(randomizer->drand() *
							num_prefers_challenger/(double)max_strat_ballots);
					cumul += strategy.begin()->weight;
				}

				// Add the strategic ballot if it has nonzero weight.
				if (strategy.begin()->weight > 0) {
					prefers_winner.push_back(*strategy.begin());
					++num_strat_ballots;
				}
			}

			// Determine the winner again! A tie counts if our man
			// is at top rank, because he wasn't, before.
			ordering strat_result = method->elect(prefers_winner,
					numcands, true);

			/*cout << ordering_tools().ordering_to_text(
			  strat_result,
			        rcl, true) << endl;*/

			// Then remove the strategic coalition ballot so another
			// one can be inserted later.
			for (q = 0; q < num_strat_ballots; ++q) {
				prefers_winner.pop_back();
			}

			// Check if our candidate is now at top rank.
			for (pos = strat_result.begin(); pos !=
				strat_result.end() &&
				pos->get_score() == strat_result.
				begin()->get_score(); ++pos) {
				if (pos->get_candidate_num() == challenger) {
					strategy_worked = true;
				}
			}

			if (verbose && strategy_worked) {
				cout << "Strategy to elect " << challenger
					<< " worked!" << endl;
				cout << "Outcome after strategy: " <<
					ordering_tools().ordering_to_text(strat_result, false) << endl;
				cout << "After strategy: " << endl;
				ballot_tools().print_ranked_ballots(prefers_winner);
			}

			if (strategy_worked) {
				if (tries < ST_OTHER) {
					return (basic_strategy)tries;
				}
				return ST_OTHER;
			}

		}
	}

	if (verbose && !strategy_worked) {
		cout << "Strategy didn't work!" << endl;
	}

	return ST_NONE;
}

// Err, should probably not use camelcase... Fix later. TODO
strategy_result
StrategyTest::attempt_execute_strategy() { // Mega method, fix later TODO
	int ranks = 10;                 // <- ????
	int ties_in_a_row = 0;
	ordering::const_iterator pos;
	ordering honest_outcome;

	// Ties don't count because any method that's decisive will let a voter
	// break the tie.
	// Err, a method may not always be decisive! TODO: Fix that. A tie
	// should count as a method with many equal-winners and be considered
	// susceptible to strategy if someone else can be made a winner, or one
	// of the equal-winners turned into a sole winner.

	size_t numcands = numcands_min;
	if (numcands_max > numcands_min) {
		numcands = randomizer->irand(numcands_min, numcands_max+1);
	}

	// Seems to be something about - while there's a tie, try again.
	while (ranks > 1) {
		ranks = 0;
		++ties_in_a_row;
		if (ties_in_a_row > 100) {
			cout << "Too many ties in a row. Aborting!" << endl;
			too_many_ties = true;
			// Should be a third option here, not true or false.
			// TODO that. Kinda done now.
			return (STRAT_FAILED);
		}

		ballots = ballot_gens[ballot_gen_idx++]->generate_ballots(numvoters,
				numcands, *randomizer);
		if (ballot_gen_idx == ballot_gens.size()) {
			ballot_gen_idx = 0;
		}

		//cache.clear();

		// First, get the honest winner. If it's a tie, disregard. Otherwise,
		// for every other candidate, find the ballots corresponding to voters
		// who rank that candidate ahead of the winner. Then, a bunch of times,
		// run the IIC generator with numvoters = 1 and use that ballot for the
		// strategist coalition. (An exhaustive approach may come later.) If
		// this makes the candidate in question win, we're done, otherwise
		// try again until we give up and go to the next candidate, or give up
		// completely and consider the method invulnerable in this particular
		// case.

		honest_outcome = method->elect(ballots, numcands, true);

		// Check that there isn't a tie.
		//int ranks = 0;
		//ordering::const_iterator pos;
		for (pos = honest_outcome.begin(); pos != honest_outcome.end() &&
			pos->get_score() == honest_outcome.begin()->get_score(); ++pos) {
			++ranks;
		}

		// This is actually how many are at first rank. Rename the vars to
		// make that more clear.
		total_generation_attempts += ranks;

		if (ranks > 1) {
			ballots.clear();
			return (STRAT_TIE);
		}
	}

	ties_in_a_row = 0;

	// TODO: Set true with only one method (Heaviside 2 2 -1 -1), and then
	// compare to Python.
	if (strategize_for_election(ballots, honest_outcome,
			numcands, false) != ST_NONE) {
		return STRAT_SUCCESS;
	}

	return (STRAT_FAILED);
}
