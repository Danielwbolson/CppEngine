
#include "Texture.h"

Texture::~Texture() {
	delete pixels;
}

Texture::Texture(const int& w, const int& h, const int nc, GLubyte* p) {
	width = w;
	height = h;
	numChannels = nc;
	pixels = p;
}