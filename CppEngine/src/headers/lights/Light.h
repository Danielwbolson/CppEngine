
#ifndef LIGHT_H_
#define LIGHT_H_

#include <string>

struct Light {
	float lum;

	std::string type;
	std::string GetType() { return type; }
};

#endif
