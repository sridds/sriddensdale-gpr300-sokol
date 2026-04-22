#version 410

out vec4 FragColor;

// varyings
in vec2 vs_texcoord;

// uniforms
uniform sampler2D screen;
uniform vec2 resolution;
uniform float time;
uniform float strength;
uniform float scale;

// inspired by https://www.shadertoy.com/view/wlGGWw
float rand(vec2 uv, float t)
{
    return fract(sin(dot(uv, vec2(1225.6548, 321.8942))) * 4251.4865 + t);
}

void main()
{
    vec2 res = vec2(1.0) / resolution;
    vec2 uv = vs_texcoord;

    vec2 offset = (rand(uv, time) - 0.5) * 2.0 * res * scale;

    vec3 noise = texture(screen, uv + offset).rgb;
    vec3 original = texture(screen, uv).rgb;

    FragColor = vec4(mix(original, noise, strength), 1.0);
}