
#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "glad/glad.h"

class Texture {
public:
	GLubyte* pixels;
	int width;
	int height;
	int numChannels;

	Texture() {}
	~Texture();

	Texture(const int& w, const int& h, const int nc, GLubyte* p);

};

#endif // TEXTURE_H_