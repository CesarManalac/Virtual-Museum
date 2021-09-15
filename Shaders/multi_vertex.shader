#version 330 core
layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec2 a_UV;
layout(location = 2) in vec3 a_Normal;

out vec3 FragPos;
out vec3 Normal;
out vec2 UV;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_normal;

void main()
{
	FragPos = vec3(u_model * vec4(a_Pos, 1.0));
	Normal = normalize(mat3(u_normal) * a_Normal);
	UV = a_UV;

	gl_Position = u_projection * u_view * vec4(FragPos, 1.0);
}