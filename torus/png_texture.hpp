/*
 * png_texture.hpp
 *
 *  Created on: Oct 18, 2014
 *      Author: adam
 */

#ifndef PNG_TEXTURE_HPP_
#define PNG_TEXTURE_HPP_

#include <GLES/gl.h>

int png_texture_load(const char * file_name, int * width, int * height,
		GLuint texture);

#endif /* TORUS_PNG_TEXTURE_HPP_ */
