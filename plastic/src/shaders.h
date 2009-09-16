// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#ifndef PLASTIC_SHADERS_INCLUDED
#define PLASTIC_SHADERS_INCLUDED

#define SHADER_SUPPORT 0

#if SHADER_SUPPORT

GLuint load_shader(GLenum shaderType, std::string const& glslSource);

#endif

#endif
