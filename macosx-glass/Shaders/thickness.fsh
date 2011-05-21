varying vec3 frag_eye_normal;
varying vec3 frag_eye_position;

void main() {
    float thickness = gl_FragCoord.z;
    if (gl_FrontFacing == false) {
        thickness = -thickness;
    }
    
    float cos_theta = abs(dot(normalize(frag_eye_position), normalize(frag_eye_normal)));    
    float fresnel = pow(1.0 - cos_theta, 4.0);
    gl_FragColor = vec4(thickness, fresnel, 0.0, 0.0);
}