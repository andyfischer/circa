// With thanks to Philip Rideout for example code. See: http://prideout.net/bloom/index.php

uniform sampler2D source;
uniform vec4 bkgd;

void main(void)
{
    vec4 t = texture2D(source, gl_TexCoord[0].st);
    gl_FragColor = t + bkgd;
}
