uniform mat4 modelview;
uniform mat4 projection;
attribute vec4 position;
attribute vec3 normal;
varying vec3 frag_eye_normal;
varying vec3 frag_eye_position;

void main() {
    frag_eye_normal = (modelview * vec4(normal, 0.0)).xyz;
    frag_eye_position = (modelview * position).xyz;
    gl_Position = projection * modelview * position;
}