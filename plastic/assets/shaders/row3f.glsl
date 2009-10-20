// With thanks to Philip Rideout for example code. See: http://prideout.net/bloom/index.php

uniform sampler2D source;
uniform float coefficients[3];
uniform float offsetx;
uniform float offsety;

void main(void)
{
    vec4 c;
    vec2 tc = gl_TexCoord[0].st;
    vec2 offset = vec2(offsetx, offsety);

    c  = coefficients[0] * texture2D(source, tc - offset);
    c += coefficients[1] * texture2D(source, tc);
    c += coefficients[2] * texture2D(source, tc + offset);

    gl_FragColor = c;
}
