#version 410 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D gPositions;
uniform sampler2D gNormals;
uniform sampler2D gAlbedoSpec;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 spec;
    float shininess;
};

struct PointLight {
    vec3 position;
    float radius;
    vec4 color;
};

#define MAX_POINT_LIGHTS 64
uniform PointLight _PointLights[MAX_POINT_LIGHTS];

uniform Material material;
uniform vec3 camera_position;

// linear falloff helper
float attenuateLinear(float distance, float radius)
{
    return clamp((radius - distance) / radius, 0.0, 1.0);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 pos)
{
    // Diffuse
    vec3 diff = light.position - pos;
    // Direction toward light position
    vec3 toLight = normalize(diff);
    vec3 viewDir = normalize(camera_position - pos);
    vec3 halfwayDir = normalize(toLight + viewDir);

    float NdotL = max(dot(normal, toLight), 0.0);
    float NdotH = max(dot(normal, halfwayDir), 0.0);

    // Attenuation
    float attenuation = attenuateLinear(length(diff), light.radius);
    vec3 diffuse = NdotL * material.diffuse * light.color.rgb;
    vec3 spec = pow(NdotH, material.shininess) * material.spec * light.color.rgb;

    return (diffuse + spec) * attenuation;
}

void main()
{
    vec3 pos = texture(gPositions, TexCoords).rgb;
    vec3 normal = texture(gNormals, TexCoords).rgb;
    vec3 albedo = texture(gAlbedoSpec, TexCoords).rgb;

    // add up light
    vec3 totalLight;

    totalLight += material.ambient;
    for (int i = 0; i < MAX_POINT_LIGHTS; i++)
    {
        totalLight += calcPointLight(_PointLights[i], normal, pos);
    }

    FragColor = vec4(albedo * totalLight, 1.0);
}