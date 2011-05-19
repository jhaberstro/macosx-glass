//
//  Shader.vsh
//  glass
//
//  Created by Jedd Haberstro on 5/16/11.
//  Copyright 2011 Student. All rights reserved.
//

//uniform mat4 mvp;
attribute vec4 position;
attribute vec2 tex_coord;
varying vec2 coordVarying;

void main()
{
    gl_Position = position;
    coordVarying = tex_coord;
}
