//
//  Shader.vsh
//  WeatherApp
//
//  Created by Andrew Fischer on 1/28/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

attribute vec4 vertex;
attribute vec2 tex_coord;

uniform mat4 modelViewProjectionMatrix;
uniform mat3 normalMatrix;

varying lowp vec4 colorVarying;
varying vec2 TexCoordToFrag;

void main()
{
/*
    vec3 eyeNormal = normalize(normalMatrix * normal);
    vec3 lightPosition = vec3(0.0, 0.0, 1.0);
    vec4 diffuseColor = vec4(0.4, 0.4, 1.0, 1.0);
    
    float nDotVP = max(0.0, dot(eyeNormal, normalize(lightPosition)));
    
    colorVarying = diffuseColor * nDotVP;
  */  
    colorVarying = vec4(0.4, 0.4, 1.0, 1.0); // fixme
    gl_Position = modelViewProjectionMatrix * vertex;
    TexCoordToFrag = tex_coord;
}
