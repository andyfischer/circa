//
//  Shader.fsh
//  circa
//
//  Created by Paul Hodge on 7/12/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}
