#version 410

out vec4 FragColor;

in vec2 vs_texcoord;

uniform sampler2D screen;
uniform float strength;
uniform float resolution;

void main()
{
    vec3 color = texture(screen, vs_texcoord).rgb;
    float dist = distance(vs_texcoord, vec2(0.5));
    float vignette = smoothstep(resolution, resolution - strength, dist);
    
    FragColor = vec4(color * vignette, 1.0);
}