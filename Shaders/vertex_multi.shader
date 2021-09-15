#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec3 a_tangent;
layout(location = 4) in vec3 a_bitangent;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 normal;

void main()
{
	FragPos = vec3(model * vec4(aPos, 1.0));
	Normal = normalize(mat3(normal) * aNormal);
	TexCoords = aTexCoords;
	vec3 T = normalize(mat3(normal) * a_tangent);
	vec3 B = normalize(mat3(normal) * a_tangent);

	TBN = mat3(T, B, Normal);

	gl_Position = projection * view * vec4(FragPos, 1.0);
}