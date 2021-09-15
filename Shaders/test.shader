#version 330 core
uniform vec3 u_color;
out vec4 FragColor;
in vec3 FragPos;
in vec2 UV;
in vec3 Normal;
in mat3 TBN;
uniform sampler2D texture_diffuse;
uniform sampler2D texture_normal;
uniform sampler2D first_diffuse;
//uniform sampler2D second_diffuse;
//uniform sampler2D third_diffuse;

uniform vec3 u_light_pos;
//uniform vec3 u_light_dir;
uniform vec3 u_camera_pos;
uniform vec3 u_ambient_color;

uniform bool u_lit;
uniform bool u_multi;
uniform bool u_normals;
float attenuate(float value, float maximum)
{
    float clampedValue = min(value, maximum);
    return 1.0 / (pow(5 * clampedValue / maximum, 2) + 1);
}

void main()
{
    if (u_lit)
    {
        vec3 lightVector = normalize(u_light_pos - FragPos);

        float distance = length(u_light_pos - FragPos);
        float gradient = attenuate(distance, 30.0);

        vec3 lightColor = vec3(1.0, 1.0, 1.0);

        vec3 tbnNormal = texture(texture_normal, UV).rgb;
        tbnNormal = tbnNormal * 2.0 - 1.0; // converting from pixel to vector
        tbnNormal = TBN * tbnNormal;

        if (!u_normals) {
            tbnNormal = Normal;
        }

        float specularStrength = 0.3;
        vec3 viewDir = normalize(u_camera_pos - FragPos);
        vec3 reflectDir = reflect(-lightVector, tbnNormal);

        float diff = max(dot(tbnNormal, lightVector), 0.0);

        float spec = pow(max(dot(reflectDir, viewDir), 0.0), 4);

        vec3 specular = specularStrength * spec * lightColor;

        vec3 diffuse = diff * lightColor;
        vec3 ambient = u_ambient_color * lightColor;

        if (u_multi)
            FragColor = vec4(ambient + (diffuse + specular) * gradient, 1.0) * texture(texture_diffuse, UV) + texture(first_diffuse, UV);
        else
            FragColor = vec4(ambient + (diffuse + specular) * gradient, 1.0) * texture(texture_diffuse, UV);
    }

    else
    {
        if (u_multi)
            FragColor = texture(texture_diffuse, UV) + texture(first_diffuse, UV);
        else
            FragColor = texture(texture_diffuse, UV);
    }
}