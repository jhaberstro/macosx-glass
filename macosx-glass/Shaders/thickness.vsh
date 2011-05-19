uniform mat4 mvp;
attribute vec4 position;

void main() {
    gl_Position = mvp * position;
}