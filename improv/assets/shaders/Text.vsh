
attribute vec4 vertex;
attribute vec2 tex_coord;

uniform mat4 modelViewProjectionMatrix;

varying vec2 TexCoordToFrag;

void main()
{
    gl_Position = modelViewProjectionMatrix * vertex;
    TexCoordToFrag = tex_coord;
}
