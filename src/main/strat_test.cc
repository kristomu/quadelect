#include "strat_test.h"

/*		
		int total_generation_attempts;
		election_method * method;
		*/

// Err, should probably not use camelcase... Fix later. TODO
strategy_result StrategyTest::attempt_execute_strategy() { // Mega method, fix later TODO
    int ranks = 10;
    int ties_in_a_row = 0;
    ordering::const_iterator pos;
    ordering honest;

    // Ties don't count because any method that's decisive will let a voter
    // break the tie.
    // Err, a method may not always be decisive! TODO: Fix that. A tie
    // should count as a method with many equal-winners and be considered
    // susceptible to strategy if someone else can be made a winner, or one
    // of the equal-winners turned into a sole winner.

    while (ranks > 1) {
        ranks = 0;
        ++ties_in_a_row;
        if (ties_in_a_row > 100) {
            cout << "Too many ties in a row. Aborting!" << endl;
            too_many_ties = true;
            // Should be a third option here, not true or false.
            // TODO that. Kinda done now.
            return(STRAT_FAILED);
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

        honest = method->elect(ballots, numcands, true);

        // Check that there isn't a tie.
        //int ranks = 0;
        //ordering::const_iterator pos;
        for (pos = honest.begin(); pos != honest.end() && pos->get_score() ==
                honest.begin()->get_score(); ++pos)
            ++ranks;

        // This is actually how many are at first rank. Rename the vars to
        // make that more clear.
        total_generation_attempts += ranks;

        if (ranks > 1) {
		ballots.clear();
        	return (STRAT_TIE);
        }

        /*cout << ordering_tools().ordering_to_text(honest, rcl,
        		true) << endl;

        cout << "Ranks: " << ranks << endl;*/
        // TODO: Give up after n tries.
    }

    ties_in_a_row = 0;

    size_t winner = honest.begin()->get_candidate_num();

    //cout << "The winner is " << winner << endl;

    bool strategy_worked = false;

    for (size_t counter = 0; counter < numcands && !strategy_worked;
            ++counter) {
        if (counter == winner) continue;

        //cout << "Trying to rig in favor of " << counter << endl;

        list<ballot_group> prefers_winner;
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
                if (pos->get_candidate_num() == winner)
                    saw_winner = rank;
                if (pos->get_candidate_num() == counter)
                    saw_challenger = rank;

                ++rank;
            }

            // We consider equal-ranks to be in favor of the winner,
            // thus we only explicitly check if the challenger was
            // rated above the winner.
            if (saw_challenger < saw_winner)
                num_prefers_challenger += bgpos->weight;
            else
                prefers_winner.push_back(*bgpos);
        }

        /*cout << num_prefers_challenger << " prefers " << counter
        	<< endl;*/

        if (num_prefers_challenger == 0) continue;
        assert (num_prefers_challenger > 0);

        // was 512
        for (int tries = 0; tries < 384 && !strategy_worked; ++tries) {
            // Add the strategic ballot by drawing from IC.
            int iterations = 1 + tries % 3, q;
            double cumul = 0;
            for (q = 0; q < iterations; ++q) {
                list<ballot_group> strategy;
                while (strategy.empty())
                    strategy = strat_generator->generate_ballots(1, numcands,
                                                    *randomizer);
                if (q == iterations-1) {
                    assert (num_prefers_challenger - cumul > 0);
                    strategy.begin()->weight = num_prefers_challenger - cumul;
                } else {
                    strategy.begin()->weight = randomizer->drand() * num_prefers_challenger/(double)iterations;
                    cumul += strategy.begin()->weight;
                }

                // Add the strategic ballot.
                prefers_winner.push_back(*strategy.begin());
            }

            // Determine the winner again! A tie counts if our man
            // is at top rank, because he wasn't, before.
            ordering strat_result = method->elect( prefers_winner, numcands,
                                        true);

            /*cout << ordering_tools().ordering_to_text(
              strat_result,
            		rcl, true) << endl;*/

            // Then remove the strategic coalition ballot so another
            // one can be inserted later.
            for (q = 0; q < iterations; ++q)
                prefers_winner.pop_back();

            // Check if our candidate is now at top rank.
            for (pos = strat_result.begin(); pos !=
                    strat_result.end() &&
                    pos->get_score() == strat_result.
                    begin()->get_score(); ++pos)
                if (pos->get_candidate_num() == counter)
                    strategy_worked = true;
        }
        /*if (strategy_worked)
        	cout << "Strategy to elect " << counter << " worked!" << endl;*/
    }

    ballots.clear();

    if (strategy_worked) {
        //	cout << "Strategy worked!" << endl;
        return(STRAT_SUCCESS);
    }
    //else	cout << "Didn't work." << endl;
    //cout << endl << endl;
    return(STRAT_FAILED);
}
