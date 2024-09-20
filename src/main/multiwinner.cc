// More multiwinner hacks/clustering tests.

#include "multiwinner/methods.h"
#include "multiwinner/qpq.h"

#include "generator/spatial/all.h"
#include "random/random.h"
#include "tools/tools.h"

#include "multiwinner/helper/errors.h"

// TODO: Make error type parametric, or just inherit the function/
// template it or something.

class proportionality_measure {
	public:
		virtual void prepare(const positions_election & p_e) = 0;
		virtual double get_error(const std::list<size_t> & outcome) = 0;
};

// Clustering-based proportionality measure

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

// Check the error between a normal distribution fitted to the winners
// and the normal reference distribution.

class normal_proportionality : public proportionality_measure {
	private:
		gaussian_generator reference;
		std::vector<std::vector<double> > candidate_positions;
		rng rnd;

	public:
		// TODO, use a different seed
		normal_proportionality(gaussian_generator ref_in) : rnd(1) {
			reference = ref_in;
		}

		void prepare(const positions_election & p_e);
		double get_error(const std::list<size_t> & outcome);
};

void normal_proportionality::prepare(const positions_election & p_e) {
	candidate_positions = p_e.candidates_pos;
}

double normal_proportionality::get_error(
	const std::list<size_t> & outcome) {

	size_t num_seats = outcome.size();

	// Standard deviation is undefined if we have only one winner.
	if (num_seats < 2) {
		throw std::invalid_argument("normal_proportionality: "
			"Need at least two winners");
	}

	size_t dim, dims = reference.get_num_dimensions();

	std::vector<double> mu_estimate(dims, 0), sigma_estimate(dims, 0);

	// Get means first.
	for (size_t winner: outcome) {
		for (dim = 0; dim < dims; ++dim) {
			mu_estimate[dim] += candidate_positions[winner][dim];
		}
		for (dim = 0; dim < dims; ++dim) {
			mu_estimate[dim] /= (double)num_seats;
		}
	}

	// Get standard deviations.
	for (size_t winner: outcome) {
		for (dim = 0; dim < dims; ++dim) {
			sigma_estimate[dim] += square(
					candidate_positions[winner][dim] - mu_estimate[dim]);
		}
		for (dim = 0; dim < dims; ++dim) {
			// Setting this to 1 makes it large-biased, because a bunch of
			// candidates clustered around the center gives a more accurate mean
			// than if they're dispersed.
			sigma_estimate[dim] = sqrt(sigma_estimate[dim] / (double)num_seats);
		}
	}

	/*std::cout << "mu: ";
	std::copy(mu_estimate.begin(), mu_estimate.end(), std::ostream_iterator<double>(std::cout, " "));
	std::cout << "\n";
	std::cout << "sigma: ";
	std::copy(sigma_estimate.begin(), sigma_estimate.end(),
		std::ostream_iterator<double>(std::cout, " "));
	std::cout << std::endl;*/

	gaussian_generator estimated;
	estimated.set_params(dims, false);
	estimated.set_center(mu_estimate);
	estimated.set_dispersion(sigma_estimate);

	// Pick random points from the reference distribution and
	// get the pdf from both. Use Sainte-Laguë index-like measure.

	// TODO: KL divergence? Closed form integral instead of this
	// slow Monte Carlo stuff?

	size_t iters = 500;
	double error_sum = 0;

	for (size_t i = 0; i < iters; ++i) {
		std::vector<double> q = reference.rnd_vector(dims, rnd);

		double observed = estimated.pdf(q),
			   expected = reference.pdf(q);

		error_sum += square(observed-expected)/expected;
	}

	return error_sum / (double)iters;
}

// normsum, etc.

int main() {
	rng rnd(1);
	gaussian_generator gauss;
	gauss.set_params(5, false); // 5D spatial, say.
	std::cout << gauss.get_num_dimensions() << std::endl;
	gauss.set_dispersion(1);

	size_t num_voters = 4096;
	size_t num_candidates = 50;

	size_t num_clusters = 2; // say

	std::cout << gauss.pdf(std::vector<double>(5, 0.1)) << "\n";

	cluster_proportionality test(num_clusters);
	normal_proportionality ntest(gauss);

	for (double delta = 0.1; delta <= 1; delta += 0.1) {

		double error = 0;
		for (int i = 0; i < 5000; ++i) {

			positions_election p_e = gauss.generate_election_result(
					num_voters, num_candidates, false, rnd);

			ntest.prepare(p_e);

			// Elect using, say, QPQ.
			size_t num_seats = 7;

			std::list<size_t> qpq_council = QPQ(delta, true).get_council(
					num_seats, num_candidates, p_e.ballots);

			error += ntest.get_error(qpq_council);
		}

		std::cout << delta << "\t" << error << std::endl;
	}
}