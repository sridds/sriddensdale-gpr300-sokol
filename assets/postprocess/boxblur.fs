#version 410

out vec4 FragColor;

in vec2 vs_texcoord;

uniform sampler2D screen;
uniform vec2 resolution;
uniform float strength;

// based on https://www.shadertoy.com/view/MddGzs
void main()
{
    const int kernelSize = 5;
    vec3 avg = vec3(0.0);

    // average over the kernels
    for (int i = -kernelSize; i <= kernelSize; i++)
    {
        for (int j = -kernelSize; j <= kernelSize; j++)
        {
            vec2 offset = vec2(float(i), float(j)) / resolution;
            avg += texture(screen, vs_texcoord + offset).rgb;
        }
    }

    int area = (2 * kernelSize + 1) * (2 * kernelSize + 1);
    vec3 blurred = avg / vec3(area);
    vec3 original = texture(screen, vs_texcoord).rgb;

    FragColor = vec4(mix(original, blurred, strength), 1.0);
}