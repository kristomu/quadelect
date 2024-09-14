#include <vector>

class party_list_result {
	public:
		// seats[i] is the number of seats given to party i.
		std::vector<size_t> seats;
		size_t total_num_seats = 0;
};