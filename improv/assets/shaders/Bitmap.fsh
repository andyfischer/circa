//
//  Shader.fsh
//  WeatherApp
//
//  Created by Andrew Fischer on 1/28/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

precision mediump float;

uniform sampler2D sampler;
uniform sampler2D sampler2;
uniform vec4 color;
uniform float blend;

varying lowp vec2 TexCoordToFrag;

void main()
{
    vec4 t = texture2D(sampler, TexCoordToFrag) * (1.0 - blend)
        + texture2D(sampler2, TexCoordToFrag) * blend;
    gl_FragColor = color * t;
}
