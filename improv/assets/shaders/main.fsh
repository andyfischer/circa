
#ifdef GL_ES
precision mediump float;
precision lowp float;
#endif

uniform sampler2D sampler;
uniform vec4 color;

varying vec2 TexCoordToFrag;

void main()
{
    gl_FragColor = color * texture2D(sampler, TexCoordToFrag).a;
    
}
