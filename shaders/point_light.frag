#version 330 core

layout(location = 0) out vec4 frag_color;

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};


uniform Material material;
uniform Light light;

uniform vec3 viewPos;

in vec3 fragPos;
in vec3 normal;
in vec2 v_tex_coord;

void main()
{
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, v_tex_coord));
  	
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.diffuse * vec3(texture(material.diffuse, v_tex_coord));
    
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * light.specular * vec3(texture(material.specular, v_tex_coord));

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
        
    vec3 result = ambient + diffuse + specular;

    frag_color = vec4(result, 1);
}
