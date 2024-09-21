#include "normal_fit.h"
#include "multiwinner/helper/errors.h"

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
