#version 410

out vec4 FragColor;

// varyings
in vec2 vs_texcoord;

// uniforms
uniform sampler2D screen;
uniform float sharpness;
uniform vec2 resolution;
uniform float time;

// based on: https://www.shadertoy.com/view/lslGRr
void main()
{
    vec2 uv = vs_texcoord;
    vec2 kernel = 1.0 / resolution;

    vec3 texA = texture(screen, uv + vec2(-kernel.x, -kernel.y) * 1.5).rgb;
    vec3 texB = texture(screen, uv + vec2( kernel.x, -kernel.y) * 1.5).rgb;
    vec3 texC = texture(screen, uv + vec2(-kernel.x,  kernel.y) * 1.5).rgb;
    vec3 texD = texture(screen, uv + vec2( kernel.x,  kernel.y) * 1.5).rgb;

    vec3 around = 0.25 * (texA + texB + texC + texD);
    vec3 center = texture(screen, uv).rgb;
    vec3 col = center + (center - around) * sharpness;

    FragColor = vec4(col, 1.0);
}