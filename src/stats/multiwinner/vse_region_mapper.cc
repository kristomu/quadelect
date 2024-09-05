#include "vse_region_mapper.h"
#include "tools/tools.h"

#include <cmath>
#include <vector>

#include <iostream>

std::vector<double> vse_region_mapper::get_quantized_position(
	const VSE_point & pt) const {

	std::vector<double> quantized_VSEs;

	for (const VSE & v: pt) {
		quantized_VSEs.push_back(round(v.get()*
				elements_per_dimension)/(double)elements_per_dimension);
	}

	return quantized_VSEs;
}

double vse_region_mapper::get_quantization_error(
	const VSE_point & pt) const {

	double squared_error = 0;

	for (const VSE & coordinate: pt) {
		double quantized = round(coordinate.get()*
				elements_per_dimension)/(double)elements_per_dimension;

		squared_error += square(quantized-coordinate.get());
	}

	return sqrt(squared_error);
}

void vse_region_mapper::add_augmented_point(
	const VSE_point & augmented_point) {

	std::vector<double> quant_pos =
		get_quantized_position(augmented_point);

	if (next_VSE_cloud.find(quant_pos) == next_VSE_cloud.end()) {
		next_VSE_cloud[quant_pos] = augmented_point;
	} else {
		VSE_point incumbent = next_VSE_cloud.find(quant_pos)->second;

		if (get_quantization_error(augmented_point) <
			get_quantization_error(incumbent)) {

			next_VSE_cloud[quant_pos] = augmented_point;
		}
	}
}

void vse_region_mapper::add_point(const VSE_point & cur_round_VSE) {

	// If the cloud is empty, just add the point directly.
	if (VSE_cloud.empty()) {
		add_augmented_point(cur_round_VSE);
		return;
	}

	// Otherwise, augment every existing cloud point from the previous
	// round with this point, and add them all to the new cloud.
	for (auto pos = VSE_cloud.begin(); pos != VSE_cloud.end(); ++pos) {

		// Copy the cloud point, add the new data, and insert.
		VSE_point new_cloud_point = pos->second;
		for (size_t i = 0; i < new_cloud_point.size(); ++i) {
			new_cloud_point[i].add_last(cur_round_VSE[i]);
		}

		add_augmented_point(new_cloud_point);
	}
}

void vse_region_mapper::add_points(const std::vector<VSE_point> & points) {
	for (const VSE_point & p: points) {
		add_point(p);
	}
}

void vse_region_mapper::update() {
	VSE_cloud = filter_augmented_points(next_VSE_cloud);
	next_VSE_cloud.clear();
	std::cout << "Update: now we have " << VSE_cloud.size() << " points.\n";
}

std::vector<VSE_point> vse_region_mapper::get() const {
	std::vector<VSE_point> out;

	for (auto pos = VSE_cloud.begin(); pos != VSE_cloud.end(); ++pos) {
		out.push_back(pos->second);
	}

	return out;
}

void vse_region_mapper::dump_coordinates(std::ostream & where) const {

	for (auto pos = VSE_cloud.begin(); pos != VSE_cloud.end(); ++pos) {
		bool first = true;
		for (const VSE & v: pos->second) {
			if (!first) {
				where << " ";
			}
			where << v.get();
			first = false;
		}
		where << "\n";
	}
}