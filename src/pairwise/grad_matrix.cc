// Gradual Condorcet-Borda matrix. Each entry starts by being the pair's median
// rank position difference. If there is a Condorcet winner, every such pair
// involving the CW on the winning side will have a positive value, and every
// pair involving the CW on the losing side will have a negative value.

// The matrix then has an "update" method which expands the truncated mean from
// the median out towards the mean. When it becomes the mean of all values, the
// Borda winner will beat all the others. Thus, the matrix lets one combine
// Borda and Condorcet into a method that will give as little as possible in
// the direction of Borda as required to break cycles or ties.

#include "grad_matrix.h"

size_t cond_borda_matrix::linear(size_t cand, size_t against, size_t numcands) const {

	return(against * numcands + cand);
}

double cond_borda_matrix::get_internal(size_t candidate, size_t against, 
		bool raw) const {

	if (candidate == against) return(0);

	double raw_score = engine.get_score(linear(candidate, against, 
				num_candidates));

	// If it's not allocated or the candidate number is too large, boom.
	// TODO: Do something with -INF/+INF GF_BOTH scores. What should their
	// values be? Approvalish tiebreak? Hm.
	// Also consider different score schedules now that we've got Condorcet
	// matrices set up. The schedules should go in engine, though.

	if (raw)
		return(raw_score);
	else	return(type.transform(raw_score, engine.get_score(linear(
						against, candidate, 
						num_candidates)),
				num_voters));
}

bool cond_borda_matrix::add_internal(size_t candidate, size_t against,
	double weight, double value) {

	if (std::max(candidate, against) >= num_candidates) {
		throw std::out_of_range("cond_borda::add_internal: "
			"Candidate number out of range");
	}

	return(engine.add_rating(linear(candidate, against, num_candidates), 
				weight, value));
}

bool cond_borda_matrix::set_internal(size_t candidate, size_t against,
	double weight, double value) {

if (std::max(candidate, against) >= num_candidates) {
		throw std::out_of_range("cond_borda::set_internal: "
			"Candidate number out of range");
	}
	return(engine.set_rating(linear(candidate, against, num_candidates), 
				weight, value));
}

bool cond_borda_matrix::set_internal(size_t candidate, size_t against,
	double value) {

	return(set_internal(candidate, against, 1, value));
}

cond_borda_matrix::cond_borda_matrix(const list<ballot_group> & scores,
		size_t num_candidates_in, pairwise_type kind, bool cardinal,
		completion_type completion_in) : abstract_condmat(kind) {

	if (num_candidates_in == 0) {
		throw std::runtime_error("cond_borda_matrix: Can't initialize matrix "
			"with no candidates!");
	}

	// Make sure we can get space for all candidates.
	num_candidates = num_candidates_in;
	int pairwise = linear(num_candidates, num_candidates, num_candidates);

	if (engine.add_candidates(pairwise) != pairwise) {
		throw std::logic_error("cond_borda_matrix: Consistency error "
			"adding candidates!");
	}

	num_voters = 0;
	count_ballots(scores, num_candidates_in, cardinal, completion_in);
}

cond_borda_matrix::cond_borda_matrix(const cond_borda_matrix & in,
		pairwise_type kind) : abstract_condmat(kind) {

	engine = in.engine;
	num_voters = in.num_voters;
	num_candidates = in.num_candidates;
}

void cond_borda_matrix::count_ballots(const list<ballot_group> & scores, 
		size_t num_candidates_in, bool cardinal, completion_type
		completion) {

	// Check that we have as many candidates as needed. If not, boom.
	// TODO: Just wipe and resize if too small.
	if (engine.get_num_candidates() < num_candidates_in * 
			num_candidates_in || num_candidates <
			num_candidates_in) {
		throw std::runtime_error("count_ballots: Too few candidates!");
	}

	// For each ballot,
	//	pri_count = 0, sec_count = 0
	//	for (pri = first to last ballot)
	//		if pri's score is not equal to last score, ++pri_count;
	//		old_sec = pri's score, sec_count = pri_count.
	//
	//		for (sec = pri+1 to last ballot)
	//			if sec's score is not equal to last score (old_
	//			sec), ++sec_count
	//			add(pri, sec, pri_count - sec_count)
	//			add(sec, pri, sec_count - pri_count)

	vector<bool> seen(num_candidates);
	size_t counter;

	for (list<ballot_group>::const_iterator pos = scores.begin(); pos !=
			scores.end(); ++pos) {

		num_voters += pos->weight;

		// Truncation is handled just like with an ordinary Condorcet
		// matrix: truncated candidates are considered to rank below
		// every other candidate. For cardinal, ??? (perhaps equal to
		// distance between top and bottom?)

		fill(seen.begin(), seen.end(), false);

		size_t pri_count = num_candidates, sec_count = num_candidates;
		ordering::const_iterator pri, sec;
		double primary_old = INFINITY, secondary_old;

		// First find out which are ranked at all.
		for (pri = pos->contents.begin(); pri != pos->contents.end();
				++pri) {
			if (pri->get_candidate_num() >= num_candidates) {
				throw std::out_of_range("cond_borda::count_ballots: "
					"Voter ranked a candidate greater then number of candidates.");
			}
			seen[pri->get_candidate_num()] = true;
		}

		bool ok; // Did we manage to add the points? If this is false,
		// go boom, as it should never happen.

		// Rank all those who are not as equal among themselves.
		// TODO: Fix oddity where Copeland based on this doesn't match
		// ordinary Copeland. Find out why. It works with equal rank
		// above last, so that can't be the problem.
		// No, we commented out the equal-rank for the wrong method,
		// doh. Check later... with the *right* generator.
		for (counter = 0; counter < seen.size(); ++counter) {
			if (seen[counter]) continue;

			for (sec_count = 0; sec_count < seen.size(); 
					++sec_count)
				if (counter != sec_count && !seen[sec_count]) {
					ok = add_internal(counter, sec_count,
							pos->weight, 0);

					ok &= add_internal(sec_count, counter,
							pos->weight, 0);

					if (!ok) {
						throw std::logic_error("cond_borda::count_ballots: "
							"Could not add ranking!");
					}
				}
		}

		for (pri = pos->contents.begin(); pri != pos->contents.end();
				++pri) {
			if (primary_old != pri->get_score())
				--pri_count;

			secondary_old = pri->get_score();
			primary_old = pri->get_score();
			sec_count = pri_count;

			size_t pri_cand = pri->get_candidate_num();

			for (sec = pri; sec != pos->contents.end(); ++sec) {
				if (sec == pri) continue;

				if (sec->get_score() != secondary_old)
					--sec_count;

				if (!seen[sec->get_candidate_num()]) {
					throw std::logic_error("cond_borda::count_ballots: "
						"Got a ranking for a candidate we didn't see a "
						"ranking for before???");
				}

				double dist;

				if (cardinal)
					dist = pri->get_score() -
						sec->get_score();
				else	dist = pri_count - sec_count;

				bool x;

				x = add_internal(pri_cand, 
						sec->get_candidate_num(),
						pos->weight, dist);

				x &= add_internal(sec->get_candidate_num(),
						pri_cand, pos->weight, -dist);

				if (!x) {
					throw std::logic_error("cond_borda::count_ballots: "
						"Could not add ranking!");
				}

				secondary_old = sec->get_score();
			}

			// Rank all the unranked candidates one step lower.
			for (counter = 0; counter < seen.size(); ++counter)
				if (!seen[counter]) {
					double dist;

					// what goes here? pri's score for now,
					// i.e. assume the unranked cands are
					// ranked at zero.
					if (cardinal)
						dist = pri->get_score();
					else	dist = pri_count - sec_count+1;

					bool x;

					x = add_internal(pri_cand, counter,
							pos->weight, dist);

					x &= add_internal(counter, pri_cand,
							pos->weight, -dist);
				}
		}
	}

	// Do this because it makes stepsize smaller, thus reduces the number
	// of calls to the base method in any "gradual relaxation method" that
	// loops between update() and running the matrix through some pairwise
	// system.
	engine.compress();

	// TODO: determine if this should be done or not. Probably should.
	engine.init(0.5, completion);
}

void cond_borda_matrix::zeroize() {
	if (num_candidates == 0) {
		return;
	}

	engine = grad_fracile();
	engine.add_candidates(linear(num_candidates, num_candidates, 
				num_candidates));
}

bool cond_borda_matrix::reinit(completion_type completion) {
	return(engine.init(0.5, completion));
}

bool cond_borda_matrix::update() {
	return(engine.update(false));
}

