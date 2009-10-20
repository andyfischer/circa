// With thanks to Philip Rideout for example code. See: http://prideout.net/bloom/index.php

varying vec3 normal;

void main(void)
{
    normal = gl_Normal;
    gl_Position = ftransform();
}
