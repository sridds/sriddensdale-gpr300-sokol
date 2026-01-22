#version 450

out vec4 FragColor; // the color of this fragment

in Surface{
    vec3 Normal;
    vec2 TexCoord;
} fs_in;

uniform sampler2D _mainTex; // 2D texture sampler

void main(){
    FragColor = texture(_mainTex, fs_in.TexCoord);
}