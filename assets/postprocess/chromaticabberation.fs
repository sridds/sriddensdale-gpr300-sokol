#version 410 

out vec4 FragColor;

// varyings
in vec2 vs_texcoord;

// uniforms
uniform sampler2D screen;
uniform float strength;

void main()
{
    vec2 offset = (vs_texcoord - 0.5) * strength;

    float rChroma = texture(screen, vs_texcoord - offset).r;
    float gChroma = texture(screen, vs_texcoord).g;
    float bChroma = texture(screen, vs_texcoord + offset).b;

    FragColor = vec4(rChroma, gChroma, bChroma, 1.0);
}