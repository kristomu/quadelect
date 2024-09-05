#include "vse_region_mapper.h"

std::vector<double> vse_region_mapper::get_quantized_position(
	const VSE_point & pt) const {

	std::vector<double> quantized_VSEs;

	for (const VSE & v: pt) {
		quantized_VSEs.push_back(round(
				pt.get()*elements_per_dimension)/(double)elements_per_dimension);
	}

	return quantized_VSEs;
}

double vse_region_mapper::get_quantization_error(
	const VSE_point & pt) const {

	double squared_error = 0;

	for (const VSE & v: pt) {
		double quantized = round(pt.get()*elements_per_dimension)/
			(double)elements_per_dimension;

		squared_error += square(quantized-pt.get());
	}

	return sqrt(squared_error);
}

void vse_region_mapper::add_point(const VSE_point & cur_round_VSE) {
	for (auto pos = VSE_cloud.begin(); pos != VSE_cloud.end(); ++pos) {

		// Copy the cloud point, add the new data, and insert.
		VSE new_cloud_point = pos->second;
		new_cloud_point.add_last(cur_round_VSE);

		std::vector<double> quant_pos =
			get_quantized_position(new_cloud_point);

		if (next_VSE_cloud.find(quant_pos) == next_VSE_cloud.end()) {
			next_VSE_cloud[quant_pos] = new_cloud_point;
		} else {
			VSE incumbent = next_VSE_cloud.find(quant_pos)->second;

			if (get_quantization_error(new_cloud_point) <
				get_quantization_error(incumbent)) {

				next_VSE_cloud[quant_pos] = new_cloud_point;
			}
		}
	}
}

void vse_region_mapper:update() {
	VSE_cloud = filter_augmented_points(next_VSE_cloud);
	next_VSE_cloud.clear();
}

std::vector<VSE_point> vse_region_mapper::get() const {
	std::vector<VSE_point> out;

	for (auto pos = VSE_cloud.begin(); pos != VSE_cloud.end(); ++pos) {
		out.push_back(pos->second);
	}

	return out;
}