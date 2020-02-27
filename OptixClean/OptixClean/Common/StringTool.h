#pragma once
#include <cstdlib>
#include <string>

class StringTool {
public:
	static std::string int2string(int integer) {
		char str[20];
		itoa(integer, str, 10);
		return std::string(str);
	}
	

};