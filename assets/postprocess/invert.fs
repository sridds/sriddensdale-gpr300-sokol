#version 410

out vec4 FragColor;

// varyings
in vec2 vs_texcoord;

// uniforms
uniform sampler2D screen;

void main()
{
    FragColor = vec4(1.0 - texture(screen, vs_texcoord).rgb, 1.0);
}