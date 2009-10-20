// With thanks to Philip Rideout for example code. See: http://prideout.net/bloom/index.php

uniform vec3 diffuse;
uniform vec3 specular;
uniform vec3 ambient;
uniform float shininess;
uniform vec3 hhat;
uniform vec3 vp;
varying vec3 normal;

void main(void)
{
    float df = max(0.0, dot(vp, normal));
    vec3 color = ambient + diffuse * df;

    float sf = df <= 0.0 ? 0.0 : max(0.0, dot(hhat, normal));
    sf = pow(sf, shininess);
    color += specular * sf;

    gl_FragColor = vec4(color, 1.0);

}
