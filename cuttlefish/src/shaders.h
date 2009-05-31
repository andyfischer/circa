// Copyright 2008 Andrew Fischer

#ifndef CUTTLEFISH_SHADERS_INCLUDED
#define CUTTLEFISH_SHADERS_INCLUDED

#define SHADER_SUPPORT 0

#if SHADER_SUPPORT

GLuint load_shader(GLenum shaderType, std::string const& glslSource);

#endif

#endif
