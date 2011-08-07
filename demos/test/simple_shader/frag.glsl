uniform float elapsed;
uniform float mouseX;
uniform float mouseY;

void main()
{
    float dist1 = distance(gl_FragCoord.xy, vec2(300.0,300.0));
    float dist2 = distance(gl_FragCoord.xy, vec2(400.0,200.0));
    float dist3 = distance(gl_FragCoord.xy, vec2(mouseX,mouseY));

    float radius = elapsed;
    float radius2 = mod(elapsed + 20.0, 100.0);
    float radius3 = mod(elapsed + 40.0, 100.0);

    vec4 color;
    if (dist1 > radius && dist1 < (radius*1.1)) {
        color = vec4(1,1,1,1);
    } else if (dist2 > radius2 && dist2 < (radius2*1.1)) {
        color = vec4(1,1,1,1);
    } else if (dist3 > radius3 && dist3 < (radius3*1.1)) {
        color = vec4(1,1,1,1);
    } else {
        color = vec4(0,0,0,1);
    }
    
	gl_FragColor = color;
}
