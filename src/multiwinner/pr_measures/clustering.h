#include <vector>

#include "measure.h"

// Clustering-based proportionality measure. This groups candidates and voters
// into clusters based on proximity, and then checks the proportion of winners
// in each cluster against the proportion of voters.

class cluster_proportionality : public proportionality_measure {
	private:
		size_t num_clusters;
		std::vector<std::vector<double> > clusters;
		std::vector<size_t> cluster_for_candidate;

		// Fraction of voters covered by each cluster.
		std::vector<double> cluster_voter_proportions;

		// ... and fractions of winning candidates.
		std::vector<double> cluster_winner_proportions;

		size_t get_optimum_cluster(const std::vector<double> & point,
			const std::vector<std::vector<double> > & centers) const;

	public:
		void set_num_clusters(size_t num_clusters_in) {
			num_clusters = num_clusters_in;
			clusters.resize(num_clusters);
			cluster_voter_proportions.resize(num_clusters);
			cluster_winner_proportions.resize(num_clusters);

		}
		cluster_proportionality(size_t num_clusters_in) {
			set_num_clusters(num_clusters_in);
		}

		void prepare(const positions_election & p_e);
		double get_error(const std::list<size_t> & outcome);
};