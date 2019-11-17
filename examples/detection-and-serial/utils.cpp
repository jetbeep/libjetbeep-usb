#include "utils.hpp"

using namespace std;

string Utils::toLowerCase(const string& input) {
	string result = input;

	transform(result.begin(), result.end(), result.begin(), (int (*)(int))std::tolower);

	return result;
}