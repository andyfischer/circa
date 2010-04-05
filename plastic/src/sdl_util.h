// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#pragma once

GLenum get_texture_format(SDL_Surface *surface);
GLuint load_surface_to_texture(SDL_Surface *surface);
std::string surface_to_string(SDL_Surface* surface);
