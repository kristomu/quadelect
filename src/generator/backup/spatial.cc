// N-dimensional Euclidean space. Candidates and voters are random points on
// [0...1] and rating is equal to 1 / (distance from voter to candidate).

// TODO: somehow export the distance data. Correlation and means, etc, see
// Tideman for fits to real data.

// TODO: Set dimensions. for now, fixed to n = 2. Also consider partial
// (correlated) dimensions, eyeing the SVD political compass, and other norms
// than L2.

// Bluesky: on a sphere? Tideman did this, I think. Or was that Hylland or the
// guy with BPW?

#ifndef _BALLOT_GEN_SPATIAL
#define _BALLOT_GEN_SPATIAL

#include "ballotgen.cc"
#include <iostream>

using namespace std;

class spatial_generator : public pure_ballot_generator {
	private:
		double distance(const vector<double> & a,
			const vector<double> & b) const;
		vector<double> rnd_vector(int size) const;

	protected:
		list<ballot_group> generate_ballots_int(int num_voters,
			int numcands, bool do_truncate) const;

	public:
		spatial_generator() : pure_ballot_generator() {}
		spatial_generator(bool compress_in) :
			pure_ballot_generator(compress_in) {}
		spatial_generator(bool compress_in, bool do_truncate) :
			pure_ballot_generator(compress_in, do_truncate) {}

};

double spatial_generator::distance(const vector<double> & a,
	const vector<double> & b) const {

	double sum = 0;

	assert(a.size() == b.size());

	for (int counter = 0; counter < a.size(); ++counter) {
		sum += (a[counter]-b[counter])*(a[counter]-b[counter]);
	}

	return (sqrt(sum));
}

vector<double> spatial_generator::rnd_vector(int size) const {

	vector<double> output(size, 0);

	for (int counter = 0; counter < size; ++counter) {
		output[counter] = drand48();
	}

	return (output);
}

list<ballot_group> spatial_generator::generate_ballots_int(int num_voters,
	int numcands, bool do_truncate) const {

	list<ballot_group> toRet;

	int dimension = 2;

	// First generate the candidate positions.
	vector<vector<double> > cand_positions;

	int counter, sec;

	for (counter = 0; counter < numcands; ++counter) {
		cand_positions.push_back(rnd_vector(dimension));
	}

	// Then, for each candidate, determine his position. If we need to
	// truncate, find a radius beyond which we don't care, otherwise
	// rank candidates in order of distance.

	// TODO: Move to another function.

	vector<double> voter_pos, max_dist(dimension, 0),
		   distances_to_cand(numcands, 0);

	for (counter = 0; counter < num_voters; ++counter) {
		ballot_group our_entry;
		our_entry.weight = 1;

		// Get our location.
		voter_pos = rnd_vector(dimension);

		// Dump distances to all the candidates and get the minimum
		// distance (which may be needed for truncation).
		double mindist = INFINITY;
		for (sec = 0; sec < numcands; ++sec) {
			distances_to_cand[sec] = distance(voter_pos,
					cand_positions[sec]);

			mindist = min(mindist, distances_to_cand[sec]);
		}

		// Report all distances within this area. If we're not going
		// to truncate, include everyone...
		double care_within = dimension;

		if (do_truncate) {
			// Otherwise, find the distance to the farthest corner,
			// and pick a random distance between us and that
			// point.

			// Also TODO: Make more general.

			double sum = 0;

			// This can be done because dimensions are linear.
			// If we're closer to 0, then 1 is farther away than
			// 0, so pick it; the same logic applies to 1.
			for (sec = 0; sec < dimension; ++sec)
				if (voter_pos[sec] > 0.5) {
					max_dist[sec] = 0;
				} else	{
					max_dist[sec] = 1;
				}

			// Pick a random distance between none at all and the
			// farthest point.
			care_within = drand48() * distance(voter_pos,
					max_dist);

			// If the distance is closer than the closest candidate,
			// expand so at least one candidate gets rated.
			if (care_within < mindist) {
				care_within = mindist;
			}
		}

		// Compatibility with Warren.
		double spacing = sqrt(0.6 * dimension);

		// For all candidates...
		for (sec = 0; sec < numcands; ++sec) {
			// Get distance to candidate.
			double our_dist = distances_to_cand[sec];

			if (our_dist > care_within) {
				continue;
			}

			// And add to ordering.
			our_entry.contents.insert(candscore(sec, 1 /
					(spacing + our_dist)));
		}

		our_entry.complete = (our_entry.contents.size() == numcands);
		our_entry.rated = true;

		toRet.push_back(our_entry);
	}

	return (toRet);
}

#endif
