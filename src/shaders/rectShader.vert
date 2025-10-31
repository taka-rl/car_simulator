#version 330 core
layout (location = 0) in vec3 aPos;
uniform vec2 uOffset;
uniform vec2 uScale;
uniform float uYaw;
void main() {
   // 1: Scale the unit quad to desired size
   vec2 p = aPos.xy * uScale;
   
   // 2: Roate about the quad center
   float c = cos(uYaw);
   float s = sin(uYaw);

   // Rotation equation
   mat2 R = mat2(c, -s,
                  s, c);
   p = R * p;

   // 3: Translate to final position
   p += uOffset;

   gl_Position = vec4(p, 0.0, 1.0);
}