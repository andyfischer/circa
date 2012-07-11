
uniform vec4 color;

#ifdef GL_ES

precision mediump float;

#endif

void main()
{
    gl_FragColor = color;
}
