#version 330 core

layout(location = 0) in float t;
uniform mat4 viewProj;
uniform float xMin;
uniform float xMax;
uniform float a;
uniform float b;
uniform float c;
uniform float d;
out vec2 worldPos;

void main(){
  float x = mix(xMin, xMax, t);
  float y = a * x * x * x + b * x * x + c * x + d;
  gl_Position = viewProj * vec4(x, y, 0.0, 1.0);
}
