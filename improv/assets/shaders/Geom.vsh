
attribute vec4 vertex;
attribute vec2 tex_coord;

uniform mat4 modelViewProjectionMatrix;

void main()
{
    gl_Position = modelViewProjectionMatrix * vertex;
}
