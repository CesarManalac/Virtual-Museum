#version 330 core
out vec4 FragColor;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 diffuse;
	vec3 specular;
};

struct SpotLight {
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;

	float constant;
	float linear;
	float quadratic;

	vec3 diffuse;
	vec3 specular;
};

#define NR_POINT_LIGHTS 1

in vec3 FragPos;
in vec3 Normal;
in vec2 UV;
in mat3 TBN;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_normal;
uniform sampler2D first_diffuse;

uniform vec3 u_view;
uniform vec3 u_camera_pos;
uniform bool u_multi;
uniform bool u_normals;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

uniform Material material;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(u_view - FragPos);

	//vec3 tbnNormal = texture(texture_normal, UV).rgb;
	//tbnNormal = tbnNormal * 2.0 - 1.0; // converting from pixel to vector
	//tbnNormal = normalize(TBN * tbnNormal);
	//if (!u_normals) { //if it doesnt have normals
	//	norm = normalize(Normal);
	//}
	//else {
	//	norm = tbnNormal;
	//}

	vec3 result = CalcDirLight(dirLight, norm, viewDir);

	//for (int i = 0; i < NR_POINT_LIGHTS; i++) {
	//	result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
	//}

	//result += CalcSpotLight(spotLight, norm, FragPos, viewDir);
	FragColor = vec4(result, 1.0);
	//FragColor = vec4(result, 1.0) * texture(texture_diffuse, UV);
	//if (u_multi) {
	//	FragColor = vec4(result, 1.0) * texture(texture_diffuse, UV) + texture(first_diffuse, UV);
	//}


}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);

	float diff = max(dot(normal, lightDir), 0.0);

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

	vec3 ambient = light.ambient * vec3(texture(material.diffuse, UV));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, UV));
	vec3 specular = light.specular * spec * vec3(texture(material.specular, UV));
	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	//vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, UV));
	vec3 specular = light.specular * spec * vec3(texture(material.specular, UV));

	//ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	return (diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);

	float diff = max(dot(normal, lightDir), 0.0);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	//vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, UV));
	vec3 specular = light.specular * spec * vec3(texture(material.specular, UV));

	//ambient *= attenuation * intensity;
	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;
	return (diffuse + specular);
}