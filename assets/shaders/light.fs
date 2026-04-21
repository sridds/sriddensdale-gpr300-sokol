#version 410 core

// varyings
out vec4 FragColor;

// uniforms
uniform vec3 color;

void main()
{
    FragColor = vec4(color, 1.0);
}