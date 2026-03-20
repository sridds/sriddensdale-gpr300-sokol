#version 410

out vec4 FragColor;

// varyings
in vec2 vs_texcoord;

// uniforms
uniform sampler2D screen;
uniform float strength;
uniform float resolution;

void main()
{
    vec3 color = texture(screen, vs_texcoord).rgb;
    float line = mod(floor(vs_texcoord.y * resolution), 2.0);

    FragColor = vec4(color * (1.0 - line * strength), 1.0);
}