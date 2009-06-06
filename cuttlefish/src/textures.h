// Copyright 2008 Paul Hodge

#ifndef CUTTLEFISH_TEXTURES_INCLUDED
#define CUTTLEFISH_TEXTURES_INCLUDED

GLuint load_image_to_texture(std::string const& filename, circa::Term *errorListener);

namespace textures {

void register_functions(circa::Branch& branch);

}

#endif
