#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <random>
#include <algorithm>
#include <cmath>
#include <boost/algorithm/string/trim.hpp>

#include "vec3.h"
#include "color.h"

/*
	A utils file I made for another project some other time
	 and figured it would be useful here.
	Used mostly for string manipulation
*/

using namespace std;

/*
	Helper function to put vector of rgb values to ppm file
*/
static void put_all_normalized(ofstream& outf, vector<Color>& pixels, int num_colors)
{
	string s = "";
	for (auto& c : pixels)
	{
		int r = (int)(c.r*num_colors);
		int g = (int)(c.g*num_colors);
		int b = (int)(c.b*num_colors);
		s += to_string(r) + " " + to_string(g) + " " + to_string(b) + "\n";
	}
	outf << s;
}

/*
	Helper function to put the rgb values to a ppm file
*/
static void put(ofstream& outf, float r, float g, float b)
{
	int ir = (int)r;
	int ig = (int)g;
	int ib = (int)b;
	outf <<
		ir << " " <<
		ig << " " <<
		ib << "\n";
}

static bool is_number(string s, vector<char>& valid_chars)
{
	/* iterate through each char in the string */
	for (auto& c : s)
	{	
		/* check that it is a digit */
		if (!isdigit(c))
		{
			bool okay = false;
			/* if its not, see if char is in acceptable non-digits */
			for (auto& oc : valid_chars)
			{
				if (c == oc) okay = true;
			}
			/* if not, return failure */
			if (!okay)
			{
				return false;
			}
		}
	}
	return true;
}

static void err_msg(string msg)
{
    cerr << msg;
    exit(EXIT_FAILURE);
}

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

static string& trim(string& s, const char* t = " \t\n\r\f\v")
{
	s.erase(0, s.find_first_not_of(t));
	s.erase(s.find_last_not_of(t)+1);
	return s;
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

	for (vector<string>::iterator iter = tokens.begin(); iter != tokens.end(); iter) {
		boost::algorithm::trim((*iter));
		if ((*iter) == "") 
		{
			iter = tokens.erase(iter);
		}
		else 
		{
			iter++;
		}
        if (iter == tokens.end()) 
		{
            break;
        }
    }

	return tokens;
}

#endif