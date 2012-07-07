//
//  Shader.fsh
//  WeatherApp
//
//  Created by Andrew Fischer on 1/28/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

precision mediump float;

uniform sampler2D sampler;
uniform vec4 color;

varying lowp vec2 TexCoordToFrag;

void main()
{
    gl_FragColor = color * texture2D(sampler, TexCoordToFrag).a;
    
}
