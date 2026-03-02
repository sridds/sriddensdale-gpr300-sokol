#version 410 core

precision mediump float;

out vec4 FragColor;

// varyings
in vec2 vs_texcoord;

// uniforms
uniform sampler2D screen;

void main()
{
  vec3 color = texture(screen, vs_texcoord).rgb;
  FragColor = vec4(color, 1.0);
}