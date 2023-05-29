// Sketch Vivaldi algorithm for turning range data into coordinates. It may
// find a local optimum.
// This fits in with the w. benchmark in that we want to find a subset that
// "covers" the entire space well. To do that, we use squared correlation as
// range data.

#include "vivaldi_test.cc"
#include "../tools/tools.cc"
#include <iterator>

using namespace std;

long double dist(const coord & a, const coord & b,
	double Lp) {

	long double error = 0;

	int dimension = min(a.size(), b.size());

	for (int counter = 0; counter < dimension; ++counter) {
		error += (a[counter]-b[counter])*(a[counter]-b[counter]);
	}

	// Most common. Perhaps make a quicker nth root function later.
	if (Lp == 2) {
		return (sqrt(error));
	}

	return (pow(error, 1.0/Lp));
}

void rotate_by(long double & x, long double & y,
	long double cos_angle, long double sin_angle) {
	double xnew;
	xnew = x * cos_angle - y * sin_angle;
	y = x * sin_angle + y * cos_angle;
	x = xnew;
}

double sq_hack_error(const vector<coord> & coordinates,
	const vector<vector<double> > & supposed_distances) {

	double error = 0;
	int ctr = 0;

	for (int counter = 0; counter < supposed_distances.size(); ++counter)
		for (int sec = counter+1; sec < supposed_distances.size();
			++sec) {
			error += square(supposed_distances[counter][sec] -
					dist(coordinates[counter], coordinates[sec],
						2));
			++ctr;
		}

	/*cout << "SDD: " << supposed_distances.size() << endl;
	cout << "DEBUG: " << 0.5 * (square(supposed_distances.size()) - supposed_distances.size())
		<< " vs " << ctr << endl;*/

	return (sqrt(error / (double)ctr));
}

// --- //

// Determine all but a single point.

double sq_hack_preload(const vector<coord> & coordinates,
	const vector<vector<double> > & supposed_distances,
	int exclude_this) {

	double error = 0;

	for (int counter = 0; counter < supposed_distances.size(); ++counter) {
		if (counter == exclude_this) {
			continue;
		}

		for (int sec = counter+1; sec < supposed_distances.size();
			++sec) {
			if (sec == exclude_this) {
				continue;
			}

			error += square(supposed_distances[counter][sec] -
					dist(coordinates[counter],
						coordinates[sec], 2));
		}
	}

	return (error);
}

// Determine the missing term for the point we may be changing.

double sq_hack_complete(const vector<coord> & coordinates,
	const vector<vector<double> > & supposed_distances,
	int point_to_check) {

	double error = 0;

	// Simply add up d[*, point] except the zero-distance to itself.

	for (int counter = 0; counter < supposed_distances.size(); ++counter)
		if (counter != point_to_check)
			error += square(supposed_distances[counter]
					[point_to_check] - dist(
						coordinates[counter],
						coordinates[point_to_check], 2));

	return (error);
}

main() {
	vector<pair<string, string> > kv_methods;

	kv_methods.push_back(pair<string, string>("FPP",
			"AAAAAAAAAAAAAAAAAAAAAAAAAAA"));
	kv_methods.push_back(pair<string, string>("IRV",
			"ABAABAABAABAABAABAABAABAABA"));
	kv_methods.push_back(pair<string, string>("CIRV",
			"ABAABAABAABAABAABAABAABACCC"));
	kv_methods.push_back(pair<string, string>("DSC",
			"AAAAAAABAAAAAAAABAAAAAAAACA"));
	kv_methods.push_back(pair<string, string>("DAC",
			"AAAABAABCAAAABBABBAAAACCACC"));
	kv_methods.push_back(pair<string, string>("Buck",
			"AAAABAABCABABBBBBBAAAACCCCC"));
	kv_methods.push_back(pair<string, string>("WV",
			"ABAABACBCABAABAABBAAAACACCC"));
	kv_methods.push_back(pair<string, string>("CA",
			"ABAABAABCABAABABBBAAAACACCC"));
	kv_methods.push_back(pair<string, string>("QR",
			"ABAABAABAABAABAABAACAACAACA"));
	kv_methods.push_back(pair<string, string>("BV",
			"AAAABAABCAAAABAABBAAAACAACC"));
	kv_methods.push_back(pair<string, string>("CKH",
			"ABAABAABCABAABAABCABAABACCC"));
	kv_methods.push_back(pair<string, string>("KH",
			"ABAABAABCABAABAABCABAABAABC"));
	kv_methods.push_back(pair<string, string>("WrI",
			"AAAAAACCCAAAAAACCCAAAAAACCC"));

	vector<vector<double> > distances(kv_methods.size(), vector<double>(
			kv_methods.size(), 0));

	int counter, sec, tri;
	for (counter = 0; counter < distances.size(); ++counter) {
		for (sec = counter+1; sec < distances.size(); ++sec) {
			int hamming = 0;
			for (tri = 0; tri < kv_methods[counter].second.size();
				++tri)
				if (kv_methods[counter].second[tri] !=
					kv_methods[sec].second[tri]) {
					++hamming;
				}

			distances[counter][sec] = hamming;
			distances[sec][counter] = hamming;
		}
	}

	synth_coordinates revealer;
	vector<coord> coords, prosp_coords;
	double record = INFINITY, record_err_sq = INFINITY, worst = -INFINITY,
		   mean = 0;

	int rounds = 400;
	for (counter = 0; counter < rounds; ++counter) {
		prosp_coords = revealer.generate_random_coords(
				distances.size(), 2);

		double err = revealer.recover_coords(prosp_coords, 2,
				0.0105 + 0.02 - drand48() * 0.04,
				1.0, 0.015 + 0.02 - drand48() * 0.04,
				1000, distances, false);
		double err_sq = sq_hack_error(prosp_coords, distances);

		double test_a = sq_hack_preload(prosp_coords, distances, -1);
		double test_b = sq_hack_preload(prosp_coords, distances, 0);
		double test_c = sq_hack_complete(prosp_coords, distances, 0);

		cout << "TEST: " << test_a << " vs " << test_b << ", " << test_c << ", " <<
			test_b + test_c << endl;

		if (err > worst) {
			worst = err;
		}
		mean += err;

		if (err < record) {
			assert(err_sq < record_err_sq);
			cout << "New record: " << err << endl;
			coords = prosp_coords;
			record = err;
			record_err_sq = err_sq;

			bool improve = true;
			while (improve) {
				improve = false;
				for (int i = 0; i < prosp_coords.size(); ++i) {
					int j = random() % 2;
					prosp_coords = coords;
					coord orig = prosp_coords[i];
					cout << "iter ORIG " << counter << ": " << sq_hack_error(prosp_coords,
							distances) << endl;
					for (prosp_coords[i][j] = orig[j] * 0.15;
						prosp_coords[i][j] < orig[j] * 2.5;
						prosp_coords[i][j] += orig[j] * 0.001) {
						double modified = sq_hack_error(prosp_coords,
								distances);
						if (modified < err_sq) {
							cout << "iter " << counter << ": " << sq_hack_error(prosp_coords,
									distances) << endl;
							coords = prosp_coords;
							record = modified;
							record_err_sq = modified;
							err_sq = modified;
							improve = true;
						}
					}
				}
			}
		}
	}

	cout << "After all that, the record is " << record << " and mean was " <<
		mean/(double)rounds << " with worst " << worst << endl;

	vector<coord> unnorm = coords;

	/*coord minimum = coords[0], maximum = coords[0];
	    for (counter = 0; counter < coords.size(); ++counter)
	            for (sec = 0; sec < coords[counter].size(); ++sec) {
	                    minimum[sec] = min(minimum[sec], coords[counter][sec]);
	                    maximum[sec] = max(maximum[sec], coords[counter][sec]);
	            }

	    for (counter = 0; counter < coords.size(); ++counter)
	            for (sec = 0; sec < coords[counter].size(); ++sec) {
	                    coords[counter][sec] -= minimum[sec];
	                    //coords[counter][sec] /= (maximum[0]-minimum[0]);
	            }*/

	// Rotashon
	double len_record = -INFINITY;
	pair<int, int> recordholder(0, 0);
	for (counter = 0; counter < coords.size(); ++counter)
		for (sec = counter+1; sec < coords.size(); ++sec) {
			double th_dist = dist(unnorm[counter], unnorm[sec], 2);
			if (th_dist > len_record) {
				len_record = th_dist;
				recordholder.first = counter;
				recordholder.second = sec;
			}
		}
	cout << "Longest distance is between " << recordholder.first << "(" <<
		kv_methods[recordholder.first].first << ") and " << recordholder.second <<
		"(" << kv_methods[recordholder.second].first << ") at " << len_record <<
		endl;
	cout << endl;

	coord pos_rec = unnorm[recordholder.first],
		  adv_rec = unnorm[recordholder.second];

	double angle = atan2(adv_rec[1] - pos_rec[1],
			adv_rec[0] - pos_rec[0]);

	for (counter = 0; counter < unnorm.size(); ++counter)
		rotate_by(unnorm[counter][0], unnorm[counter][1], cos(-angle),
			sin(-angle));

	coords = unnorm;

	coord minimum = coords[0], maximum = coords[0];
	for (counter = 0; counter < coords.size(); ++counter)
		for (sec = 0; sec < coords[counter].size(); ++sec) {
			minimum[sec] = min(minimum[sec], coords[counter][sec]);
			maximum[sec] = max(maximum[sec], coords[counter][sec]);
		}

	for (counter = 0; counter < coords.size(); ++counter)
		for (sec = 0; sec < coords[counter].size(); ++sec) {
			coords[counter][sec] -= minimum[sec];
			coords[counter][sec] /= (maximum[0]-minimum[0]) * 1.05;
			coords[counter][sec] += 0.01;
		}

	cout << "Specified (real) distances:" << endl;
	for (counter = 0; counter < distances.size(); ++counter) {
		for (sec = 0; sec < distances[counter].size(); ++sec)
			cout << s_right_padded(dtos(distances[counter][sec]),
					2) << " ";
		cout << endl;
	}
	cout << endl;

	cout << "Reconstructed distances:" << endl;
	for (counter = 0; counter < distances.size(); ++counter) {
		cout << s_padded(kv_methods[counter].first, 4) << " ";
		double error = 0;
		for (sec = 0; sec < distances[counter].size(); ++sec) {
			double recdst = dist(unnorm[counter], unnorm[sec], 2);
			error += recdst - distances[counter][sec];
			string full = dtos(round(recdst*10)/10.0);
			int pos = full.find('.');
			string before, after;
			if (pos == -1) {
				before = full;
			} else {
				before = full.substr(0, pos);
				after = full.substr(pos, full.size());
			}
			cout << s_right_padded(before, 2) << s_padded(after, 2) << " ";
		}
		cout << "| " << dtos(round(error*10)/10.0) << endl;
	}
	cout << endl;

	cout << "Normalized coordinates: " << endl;
	for (counter = 0; counter < coords.size(); ++counter) {
		copy(coords[counter].begin(), coords[counter].end(),
			ostream_iterator<double>(cout, " "));
		cout << "\t" << kv_methods[counter].first << endl;
	}

	cout << "Raw coordinates: " << endl;
	for (counter = 0; counter < coords.size(); ++counter) {
		copy(unnorm[counter].begin(), unnorm[counter].end(),
			ostream_iterator<double>(cout, " "));
		cout << "\t" << kv_methods[counter].first << endl;
	}
}
