#version 330 core

uniform vec2 resolution;
uniform vec2 center;
uniform float halfHeight;
uniform int n;
uniform int maxIter;

out vec4 fragColor;

const float TAU = 6.2831853;

vec2 cmul(vec2 p, vec2 q) {
  return vec2(p.x * q.x - p.y * q.y,
              p.x * q.y + p.y * q.x);
}

vec2 cdiv(vec2 p, vec2 q) {
  return vec2(p.x * q.x + p.y * q.y,
              p.y * q.x - p.x * q.y) / dot(q, q);
}

void main() {
    vec2 ndc = gl_FragCoord.xy / resolution * 2.0 - 1.0;
    float aspect = resolution.x / resolution.y;
    vec2 z = center + ndc * vec2(halfHeight * aspect, halfHeight);

    int root = -1;      
    int iter = 0;

    for (int i = 0; i < maxIter; ++i) {
        vec2 p = vec2(1.0, 0.0);
        for (int j = 0; j < n - 1; ++j)
            p = cmul(p, z);
        vec2 zn = cmul(p, z);

        vec2 df = float(n) * p;
        if (dot(df, df) < 1e-12) break;

        z -= cdiv(zn - vec2(1.0, 0.0), df);

        float angle = atan(z.y, z.x);
        int k = int(mod(round(angle / (TAU / float(n))), float(n)));
        float ra = TAU * float(k) / float(n);
        if (length(z - vec2(cos(ra), sin(ra))) < 1e-3) {
            root = k;
            iter = i;
            break;
        }
    }

    if (root < 0) {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    float hue = float(root) / float(n);
    vec3 col = 0.5 + 0.5 * cos(TAU * (hue + vec3(0.0, 0.33, 0.67)));
    float shade = 1.0 - float(iter) / float(maxIter);
    fragColor = vec4(col * shade, 1.0);
}

