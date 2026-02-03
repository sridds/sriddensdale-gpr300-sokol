#version 410

precision mediump float;

out vec4 FragColor;

struct Light{
    vec3 color;
    vec3 position;
};

// varyings
in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;

uniform sampler2D texture0;
uniform Light light;
uniform vec3 camera_position;
uniform sampler2D zatoon;

vec3 toonShading(vec3 normal, vec3 frag_pos, vec3 light_pos, vec3 light_color)
{
    vec3 view_dir = normalize(camera_position - frag_pos);
    vec3 light_dir = normalize(light_pos - frag_pos);
    vec3 halfway_dir = normalize(light_dir + view_dir);

    float ndotl = (dot(normal, light_dir) + 1.0) * 0.5;
    float ndoth = max(dot(normal, halfway_dir), 0.0);

    vec3 gradient = texture(zatoon, vec2(ndotl, ndotl)).rgb;
    vec3 out_color = mix(pal.color2, pal.color1, gradient);

    return out_color;
}

void main()
{
    vec3 normal = normalize(vs_normal);
    vec3 object_color = texture(texture0, vs_texcoord).rgb;
    vec3 light_color = toonShading(normal, vs_position, light.position, light.color);
    
    FragColor = vec4(object_color * light_color, 1.0);
}