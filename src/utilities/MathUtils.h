#pragma once

inline constexpr float PI = 3.14159265358979323846f;

// Angle helpers using PI above, wrap to (-PI, PI].
inline float wrapPi(float a){ while(a<=-PI)a+=2*PI; while(a>PI)a-=2*PI; return a; }
inline float lerpAngle(float a,float b,float t){ float d=wrapPi(b-a); return wrapPi(a + d*t); }
