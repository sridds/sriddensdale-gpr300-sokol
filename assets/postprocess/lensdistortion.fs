#version 410 

out vec4 FragColor;

// varyings
in vec2 vs_texcoord;

// uniforms
uniform sampler2D screen;
uniform float strength;

// based on:
// https://www.shadertoy.com/view/DslfR4
void main()
{
    vec2 uv = vs_texcoord;

    float k1 = 0.3;
    float k2 = 0.1;
    float k3 = 0.0;
    float k4 = 0.0;
    float p1 = 0.0;
    float p2 = 0.0;

    uv = uv * 2.0 - 1.0;

    float x2 = uv.x * uv.x;
    float y2 = uv.y * uv.y;
    float xy2 = uv.x * uv.y;
    float r2 = x2 + y2;

    float rCoeff = 1.0 + (((k4 * r2 + k3) * r2 + k2) * r2 + k1) * r2;
    float tx = p1 * (r2 + 2.0 * x2) + p2 * xy2;
    float ty = p2 * (r2 + 2.0 * y2) + p1 * xy2;

    uv.x = uv.x * rCoeff + tx;
    uv.y = uv.y * rCoeff + ty;

    float scale = abs(k1) < 1.0 ? 1.0 - abs(k1) : 1.0 / (k1 + 1.0);
    uv /= scale;
    uv = uv * 0.5 + 0.5;

    uv = mix(vs_texcoord, uv, strength);
    FragColor = vec4(texture(screen, uv).rgb, 1.0);
}