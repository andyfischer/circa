// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#ifndef PLASTIC_IMAGE_INCLUDED
#define PLASTIC_IMAGE_INCLUDED

GLenum get_texture_format(SDL_Surface *surface);
GLuint load_image_to_texture(const char* filename, circa::Term* errorListener);
GLuint load_surface_to_texture(SDL_Surface *surface);

bool has_indexed_color(SDL_Surface* surface);

// Destroys the input surface and returns a new one.
SDL_Surface* convert_indexed_color_to_true_color(SDL_Surface* surface);

namespace image {

void setup(circa::Branch& branch);

} // namespace image

#endif

