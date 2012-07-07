//
//  Shader.fsh
//  WeatherApp
//
//  Created by Andrew Fischer on 1/28/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

varying lowp vec4 colorVarying;

varying lowp vec2 TexCoordToFrag;
uniform sampler2D Sampler;

void main()
{
    //gl_FragColor = .5*colorVarying + .5*texture2D(Sampler, TexCoordToFrag);
    //gl_FragColor = colorVarying;
    gl_FragColor = colorVarying * texture2D(Sampler, TexCoordToFrag).a;
}
