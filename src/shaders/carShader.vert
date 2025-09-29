#version 330 core
layout (location = 0) in vec3 aPos;
uniform vec2 uOffset;
void main() {
   vec3 p = aPos + vec3(uOffset, 0.0);
   gl_Position = vec4(p, 1.0);
}