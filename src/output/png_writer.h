#include <png.h>
#include <string>
#include <vector>

class png_writer {
	private:
		size_t width, height;
		FILE * fp;

		png_structp png_ptr;
		png_infop png_infoptr;

		// png.h expects C-style arrays, hence this
		// ugly way of doing things. This is a pointer
		// to a list of pointers for each row of the
		// picture.
		png_byte ** row_pointers;

		// Text metadata to be incorporated into the PNG file.
		// We need to store them like this because libpng needs
		// pointers to the text.
		std::vector<std::string> text_keys,
			text_values;

		// TODO: Flag that file is open etc, so that the
		// destructor closes the file automatically.
		bool inited = false;

		png_byte clamp_color(double intensity_in) const;

	public:
		void init_png_file(std::string filename,
			size_t width_in, size_t height_in);

		// Place a pixel with the given RGB value to the
		// output picture canvas. The colors should be in the
		// interval [0 ... 1). We also special-case handle 1
		// because some code may use [0...1] instead.
		void put_pixel(size_t x, size_t y,
			double red, double green, double blue);

		void put_pixel(size_t x, size_t y,
			const std::vector<double> & pixel_colors);

		void add_text(std::string key,
			std::string value);

		void cleanup();

		void finalize();

		png_writer() {}

		png_writer(std::string filename,
			size_t width_in, size_t height_in) {

			init_png_file(filename, width_in, height_in);
		}

		~png_writer() {
			cleanup();
		}
};