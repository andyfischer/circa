
uniform sampler2D sampler;
uniform vec4 color;

#ifdef GL_ES

precision mediump float;
varying lowp vec2 TexCoordToFrag;

#else

varying vec2 TexCoordToFrag;

#endif

void main()
{
    gl_FragColor = color * texture2D(sampler, TexCoordToFrag).a;
}
