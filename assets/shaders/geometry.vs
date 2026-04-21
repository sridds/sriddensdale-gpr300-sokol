#version 410 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

// uniforms
uniform mat4 model;
uniform mat4 view_proj;

// varyings
out Surface {
    vec3 WorldPos;
    vec2 TexCoord;
    vec3 WorldNormal;
} vs_out;

void main()
{
    vs_out.WorldPos = vec3(model * vec4(in_position, 1.0));
    vs_out.TexCoord = in_texcoord;
    vs_out.WorldNormal   = transpose(inverse(mat3(model))) * in_normal;
    gl_Position = view_proj * vec4(vs_out.WorldPos, 1.0);
}