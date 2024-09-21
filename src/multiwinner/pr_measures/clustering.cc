#include "clustering.h"

#include "tools/tools.h"
#include "multiwinner/helper/errors.h"

// Given a point and a number of cluster centers, determine what
// center is closest to the point. Ties are broken arbitrarily
// but consistently (in practice, first cluster wins).

size_t cluster_proportionality::get_optimum_cluster(
	const std::vector<double> & point,
	const std::vector<std::vector<double> > & centers) const {

	size_t record_cluster = 0;
	double record_distance = -1;
	bool set_record = false;

	for (size_t cluster_idx = 0; cluster_idx < centers.size();
		++cluster_idx) {

		double current_distance = lp_distance(2, point,
				centers[cluster_idx]);

		if (!set_record || current_distance < record_distance) {
			set_record = true;
			record_cluster = cluster_idx;
			record_distance = current_distance;
		}
	}

	return record_cluster;
}

// Choose cluster centers, then determine which cluster each candidate and voter
// belongs to. Use the voter-cluster assignments to determine the fraction of
// voters in each cluster, and the candidate-cluster assignments as a cache
// so that calculating the error is relatively fast.

void cluster_proportionality::prepare(const positions_election & p_e) {

	size_t num_voters = p_e.voters_pos.size(),
		   num_candidates = p_e.candidates_pos.size();

	// Pick the k first voter coordinates as the cluster centers.
	if (num_voters < num_clusters) {
		throw std::invalid_argument("Clustering proportionality:"
			" too few voters - need to be more than the number of clusters.");
	}

	clusters = std::vector<std::vector<double> >(
			p_e.voters_pos.begin(), p_e.voters_pos.begin()+num_clusters);

	std::fill(cluster_voter_proportions.begin(),
		cluster_voter_proportions.end(), 0);

	for (size_t voter = 0; voter < num_voters; ++voter) {
		size_t opt = get_optimum_cluster(
				p_e.voters_pos[voter], clusters);
		cluster_voter_proportions[opt] += 1.0/num_voters;
	}

	cluster_for_candidate = std::vector<size_t>(num_candidates, 0);

	for (size_t cand = 0; cand < num_candidates; ++cand) {
		cluster_for_candidate[cand] = get_optimum_cluster(
				p_e.candidates_pos[cand], clusters);
	}
}

// Calculate the proportion of winners belonging to each cluster and then
// match this against the proportion of voters using the Sainte-Laguë index.
// TODO: Think of how to use different error measures: inheritance,
// composition or templating?

double cluster_proportionality::get_error(
	const std::list<size_t> & outcome) {

	size_t num_seats = outcome.size();

	std::fill(cluster_winner_proportions.begin(),
		cluster_winner_proportions.end(), 0);

	for (size_t winner: outcome) {
		cluster_winner_proportions[cluster_for_candidate[winner]]++;
	}

	for (double & cluster_prop: cluster_winner_proportions) {
		cluster_prop /= (double)num_seats;
	}

	return sli(cluster_winner_proportions,
			cluster_voter_proportions);
}
