//
//  Shader.fsh
//  glass
//
//  Created by Jedd Haberstro on 5/16/11.
//  Copyright 2011 Student. All rights reserved.
//

uniform sampler2D depth_texture;
uniform vec3 color;
varying vec2 coordVarying;

void main()
{
    vec4 texture_val = texture2D(depth_texture, coordVarying);
    float thickness = abs(texture_val.r);
    if (thickness <= 0.0) {
        discard;
    }
    
    float fresnel = 1.0 - texture2D(depth_texture, coordVarying).g;
    float sigma = 11.0;
    float intensity = exp(-sigma * thickness) * fresnel;
    gl_FragColor = vec4(color * intensity, 1.0);
}
