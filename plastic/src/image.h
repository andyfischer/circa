// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#pragma once

GLuint load_image_to_texture(circa::EvalContext* cxt, circa::Term* term, const char* filename);

bool has_indexed_color(SDL_Surface* surface);

// Destroys the input surface and returns a new one.
SDL_Surface* convert_indexed_color_to_true_color(SDL_Surface* surface);

namespace image {

void setup(circa::Branch& branch);

} // namespace image
