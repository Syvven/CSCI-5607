#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <random>

/*
	A utils file I made for another project some other time
	 and figured it would be useful here.
	Used mostly for string manipulation
*/

using namespace std;

static float arbitraryRand(float low, float high)
{
	return (low + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (high - low))));
}

static string create_ppm_header(string type, int width, int height, int colors)
{
	ostringstream s;
	s <<
		type << "\n" <<
		width << "\n" <<
		height << "\n" <<
		colors << "\n";

	string out(s.str());

	return out;
}

static string eraseFromString(string str, char delim) {
	string my_str(str);
	my_str.erase(remove(my_str.begin(), my_str.end(), delim), my_str.end());
	return my_str;
}

static vector<string> split(string str, string delim) {
	vector<string> tokens;

	size_t pos = 0;
	string token;
	while ((pos = str.find(delim)) != string::npos) {
		token = str.substr(0, pos);

		tokens.push_back(token);

		str.erase(0, pos + delim.length());
	}

	tokens.push_back(str);

	return tokens;
}

#endif