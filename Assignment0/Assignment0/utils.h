#include <iostream>
#include <vector>
#include <string>

using namespace std;

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