#include "UtlityFunc.h"

bool string_to_int(const std::string &str, int &value)
{
	int base = 1;
	value = 0;
	for(int i = str.size() - 1; i >= 0; --i) {
		char c = str[i];
		if(c < '0' || c > '9')	return false;
		value += (c - '0') * base;
		base *= 10;
	}
	return true;
}

bool string_to_unsigned(const std::string &str, size_t &value)
{
	int base = 1;
	value = 0;
	for(int i = str.size() - 1; i >= 0; --i) {
		char c = str[i];
		if(c < '0' || c > '9')	return false;
		value += (c - '0') * base;
		base *= 10;
	}
	return true;
}