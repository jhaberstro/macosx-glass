void main() {
    float thickness = gl_FragCoord.z;
    if (gl_FrontFacing == false) {
        thickness = -thickness;
    }
    
    gl_FragColor = vec4(thickness, 0.0, 0.0, 0.0);
}