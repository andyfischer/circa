// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved

#ifndef PLASTIC_TEXTURES_INCLUDED
#define PLASTIC_TEXTURES_INCLUDED

GLenum get_texture_format(SDL_Surface *surface);
GLuint load_image_to_texture(std::string const& filename, circa::Term *errorListener);
GLuint load_surface_to_texture(SDL_Surface *surface);

namespace textures {

void setup(circa::Branch& branch);

} // namespace textures

#endif
