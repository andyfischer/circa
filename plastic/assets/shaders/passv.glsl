// With thanks to Philip Rideout for example code. See: http://prideout.net/bloom/index.php

void main(void)
{
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position    = ftransform();
}
