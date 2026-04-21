#version 410 core

layout(location = 0) out vec3 gPosition; // Worldspace position
layout(location = 1) out vec3 gNormal; // Worldspace normal
layout(location = 2) out vec3 gAlbedoSpec;

in Surface {
    vec3 WorldPos;
    vec2 TexCoord;
    vec3 WorldNormal;
} fs_in;

uniform sampler2D _MainTex;

void main()
{
    gPosition = fs_in.WorldPos;
    gAlbedoSpec = texture(_MainTex, fs_in.TexCoord).rgb;
    gNormal = normalize(fs_in.WorldNormal);
}