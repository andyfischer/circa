// With thanks to Philip Rideout for example code. See: http://prideout.net/bloom/index.php

uniform sampler2D source;
uniform float coefficients[25];
uniform float offset;

void main(void)
{
    float d = 0.1;
    float left = gl_TexCoord[0].s - offset - offset;
    float top = gl_TexCoord[0].t - offset - offset;
    vec2 tc = vec2(left, top);
    vec4 c = vec4(0, 0, 0, 0);

    c += coefficients[ 0] * texture2D(source, tc); tc.x += offset;
    c += coefficients[ 1] * texture2D(source, tc); tc.x += offset;
    c += coefficients[ 2] * texture2D(source, tc); tc.x += offset;
    c += coefficients[ 3] * texture2D(source, tc); tc.x += offset;
    c += coefficients[ 4] * texture2D(source, tc); tc.y += offset;
    tc.x = left;
    c += coefficients[ 5] * texture2D(source, tc); tc.x += offset;
    c += coefficients[ 6] * texture2D(source, tc); tc.x += offset;
    c += coefficients[ 7] * texture2D(source, tc); tc.x += offset;
    c += coefficients[ 8] * texture2D(source, tc); tc.x += offset;
    c += coefficients[ 9] * texture2D(source, tc); tc.y += offset;
    tc.x = left;
    c += coefficients[10] * texture2D(source, tc); tc.x += offset;
    c += coefficients[11] * texture2D(source, tc); tc.x += offset;
    c += coefficients[12] * texture2D(source, tc); tc.x += offset;
    c += coefficients[13] * texture2D(source, tc); tc.x += offset;
    c += coefficients[14] * texture2D(source, tc); tc.y += offset;
    tc.x = left;
    c += coefficients[15] * texture2D(source, tc); tc.x += offset;
    c += coefficients[16] * texture2D(source, tc); tc.x += offset;
    c += coefficients[17] * texture2D(source, tc); tc.x += offset;
    c += coefficients[18] * texture2D(source, tc); tc.x += offset;
    c += coefficients[19] * texture2D(source, tc); tc.y += offset;
    tc.x = left;
    c += coefficients[20] * texture2D(source, tc); tc.x += offset;
    c += coefficients[21] * texture2D(source, tc); tc.x += offset;
    c += coefficients[22] * texture2D(source, tc); tc.x += offset;
    c += coefficients[23] * texture2D(source, tc); tc.x += offset;
    c += coefficients[24] * texture2D(source, tc);

    gl_FragColor = c;
}
