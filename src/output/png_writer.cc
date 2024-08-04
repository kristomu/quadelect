#include "png_writer.h"

#include <iostream>

#include <cmath>
#include <fstream>
#include <stdexcept>

#include <zlib.h>

// Callback functions.
// Credit goes to ImageMagick because this way of handling libpng errors
// is woefully undocumented, but the ImageMagick source shows how it's
// done.

// As I understand it, png_info is a pointer to (any kind of) struct
// defined at init time, which can be used to pass information about
// what picture we're dealing with. We'll just use the object itself
// so that we can clean up properly.

static void png_error_handler(png_struct * caller_ptr,
	png_const_charp message) {

	png_writer * caller = (png_writer *)caller_ptr;

	caller->cleanup();

	throw std::runtime_error(
		"png_writer: Error writing file " + caller->get_filename() + ": "
		+ std::string(message));
}

static void png_warning_handler(png_struct * caller_ptr,
	png_const_charp message) {

	std::cerr << "png_writer: Warning: " << message << std::endl;
}

png_byte png_writer::clamp_color(double intensity_in) const {
	int color = round(256 * intensity_in);

	// Clamp everything below zero or >= 256 to the interval
	// [0...255].
	return std::min(255, std::max(0, color));
}

void png_writer::init_png_file(std::string filename_in,
	size_t width_in, size_t height_in) {

	png_filename = filename_in;

	if (inited) {
		finalize(); // Finish anything that's been left dangling.
	}

	width = width_in;
	height = height_in;

	// Create the PNG structures required.
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
			this, png_error_handler, png_warning_handler);

	if (!png_ptr) {
		throw std::runtime_error("png_writer: "
			"Could not create PNG struct!");
	}

	// Set parameters that work well for Yee.
	// (High compression and memory level)
	png_set_compression_level(png_ptr, 9);
	png_set_compression_mem_level(png_ptr, 9);
	png_set_compression_strategy(png_ptr, Z_FILTERED);

	png_infoptr = png_create_info_struct(png_ptr);
	if (!png_infoptr) {
		throw std::runtime_error("png_writer: "
			"Could not create PNG info struct!");
	}

	fp = fopen(png_filename.c_str(), "wb");
	if (!fp) {
		png_destroy_write_struct(&png_ptr, &png_infoptr);
		throw std::runtime_error("png_writer: "
			"Could not open file " + png_filename + " for writing!");
	}

	// Link the png_ptr structure to the file we opened.
	png_init_io(png_ptr, fp);

	// Set metadata.
	int bit_depth = 8;
	int color_type = PNG_COLOR_TYPE_RGB;
	png_set_IHDR(png_ptr, png_infoptr, width, height,
		bit_depth, color_type, PNG_INTERLACE_ADAM7,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	// Allocate memory for the image data (ew)
	row_pointers = (png_byte**)png_malloc(png_ptr,
			height * sizeof(png_byte*));
	for (size_t y = 0; y < height; y++) {
		row_pointers[y] = (png_byte*)png_malloc(png_ptr,
				width * 3 * sizeof(png_byte));
	}

	inited = true;
}

void png_writer::put_pixel(size_t x, size_t y,
	double red, double green, double blue) {

	if (!inited) {
		throw std::runtime_error("png_writer: Tried to put pixel "
			"without initialization!");
	}

	row_pointers[y][x * 3] = clamp_color(red);
	row_pointers[y][x * 3 + 1] = clamp_color(green);
	row_pointers[y][x * 3 + 2] = clamp_color(blue);
}

void png_writer::put_pixel(size_t x, size_t y,
	const std::vector<double> & pixel_colors) {

	put_pixel(x, y, pixel_colors[0], pixel_colors[1], pixel_colors[2]);
}

void png_writer::add_text(std::string keyword, std::string text) {

	text_keys.push_back(keyword);
	text_values.push_back(text);
}

void png_writer::cleanup() {
	if (row_pointers != NULL) {
		for (size_t y = 0; y < height; ++y) {
			png_free(png_ptr, row_pointers[y]);
		}
		png_free(png_ptr, row_pointers);

		row_pointers = NULL;
	}

	if (fp != NULL) {
		fclose(fp);
		fp = NULL;
	}

	if (png_ptr != NULL || png_infoptr != NULL) {
		png_destroy_write_struct(&png_ptr, &png_infoptr);
		png_ptr = NULL;
		png_infoptr = NULL;
	}

	text_keys.clear();
	text_values.clear();

	png_filename = "";
	inited = false;
}

void png_writer::finalize() {

	// Set text values.
	size_t num_texts = std::min(text_keys.size(),
			text_values.size());

	png_text * text_ptr = new png_text[num_texts];

	for (size_t i = 0; i < num_texts; ++i) {
		text_ptr[i].compression = PNG_TEXT_COMPRESSION_NONE;
		text_ptr[i].key = (char *)text_keys[i].c_str();
		text_ptr[i].text = (char *)text_values[i].c_str();
	}

	png_set_text(png_ptr, png_infoptr, text_ptr, num_texts);

	// Write the image to disk.
	png_set_rows(png_ptr, png_infoptr, row_pointers);
	png_write_png(png_ptr, png_infoptr, PNG_TRANSFORM_IDENTITY, NULL);

	// Clean up after ourselves.
	cleanup();
	delete[] text_ptr;
}
