#version 410

out vec4 FragColor;

// varyings
in vec2 vs_texcoord;

// uniforms
uniform sampler2D screen;
uniform vec2 resolution;
uniform float strength;

const float pi = atan(1.0) * 4.0;
const int kernel = 35;
const float sigma = float(kernel) * .25;

// based on https://www.shadertoy.com/view/4tSyzy
float gaussian(vec2 i)
{
    return (1.0 / (2.0 * pi * sigma * sigma)) * exp(-((i.x * i.x + i.y * i.y) / (2.0 * sigma * sigma)));
}

vec3 blur(sampler2D sp, vec2 uv, vec2 scale)
{
    vec3 col = vec3(0.0);
    float accum = 0.0;

    for (int x = -kernel / 2; x < kernel / 2; x++)
    {
        for (int y = -kernel / 2; y < kernel / 2; y++)
        {
            vec2 offset = vec2(x, y);
            float weight = gaussian(offset);
            col += texture(sp, uv + scale * offset).rgb * weight;
            accum += weight;
        }
    }

    return col / accum;
}
void main()
{
    vec3 blurred = blur(screen, vs_texcoord, vec2(1.0) / resolution);
    vec3 original = texture(screen, vs_texcoord).rgb;

    FragColor = vec4(mix(original, blurred, strength), 1.0);
}