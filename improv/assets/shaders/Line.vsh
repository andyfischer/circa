//
//  Geometry.vsh
//  WeatherApp
//
//  Created by Andrew Fischer on 1/28/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

attribute vec4 vertex;

uniform mat4 modelViewProjectionMatrix;

void main()
{
    gl_Position = modelViewProjectionMatrix * vertex;
}