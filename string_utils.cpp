#include "string_utils.h"

std::vector<std::string> split(std::string s, char delimiter) {
	std::vector<std::string> splits;
	std::string substring = "";
	for (char c : s) {
		if (c == delimiter) {
			splits.push_back(substring);
			substring = "";
		}
		else substring.push_back(c);
	}
	splits.push_back(substring);
	return splits;
}