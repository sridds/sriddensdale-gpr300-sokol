#version 300 es

precision mediump float;

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

struct Ambient {
  float intensity;
  vec3 color;
};

struct Light {
  vec3 color;
  vec3 position;
};

out vec4 FragColor;

in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;
in vec4 vs_fragLightSpace;

uniform sampler2D texture0;
uniform sampler2D shadowMap;
uniform Material material;
uniform Ambient ambient;
uniform Light light;
uniform vec3 camera_position;
uniform float minBias;
uniform float maxBias;

float ShadowCalculation(vec4 fragPosLightSpace)
{
  // perspective divide
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

  projCoords = projCoords * 0.5 + 0.5;
  float closestDepth = texture(shadowMap, projCoords.xy).r;
  float currentDepth = projCoords.z;

  vec3 normal = normalize(vs_normal);
  vec3 lightDir = normalize(light.position - vs_position);
  float bias = max(maxBias * (1.0 - dot(normal, lightDir)), minBias);

  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

  for(int x = -1; x <= 1; ++x)
  {
    for(int y = -1; y <= 1; ++y)
    {
      float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
      shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
    }
  }
  shadow /= 9.0;

  if(projCoords.z > 1.0)
    shadow = 0.0;

  return shadow;
}

vec3 blinnPhong(vec3 normal, vec3 frag_pos, vec3 light_pos, vec3 light_color) 
{
  vec3 view_dir = normalize(camera_position - frag_pos);
  vec3 light_dir = normalize(light_pos - frag_pos);
  vec3 halfway_dir = normalize(light_dir + view_dir);

  float ndotl = max(dot(normal, light_dir), 0.0);
  float ndoth = max(dot(normal, halfway_dir), 0.0);

  vec3 diffuse = ndotl * material.diffuse;
  vec3 specular = pow(ndoth, material.shininess * 128.0) * material.specular;

  return (diffuse + specular) * light_color;
}

void main()
{
  vec3 normal = normalize(vs_normal);
  vec3 object_color = (normal * 0.5 + 0.5);
  vec3 light_color = blinnPhong(normal, vs_position, light.position, light.color);

  // calculate shadow
  float shadow = ShadowCalculation(vs_fragLightSpace);
  vec3 lighting = ((ambient.color * material.ambient) + (1.0 - shadow) * light_color); // might be wrong

  FragColor = vec4(lighting, 1.0);
}