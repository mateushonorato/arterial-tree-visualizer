#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;
in vec3 GouraudColor;
uniform int lightingMode; // 0=Phong, 1=Gouraud, 2=Flat
uniform int selectedSegmentID;
flat in int vSegmentID;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float alpha;
out vec4 FragColor;

void main()
{
    vec3 result = vec3(0.0);
    if (lightingMode == 0) {
        // Phong shading (default): compute lighting per fragment
        float ambientStrength = 0.2;
        vec3 ambient = ambientStrength * Color;
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * Color;
        float specularStrength = 0.5;
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * vec3(1.0);
        result = ambient + diffuse + specular;
    } else if (lightingMode == 1) {
        // Gouraud shading: use interpolated color
        result = GouraudColor;
    } else if (lightingMode == 2) {
        // Flat shading: use geometric normal
        float ambientStrength = 0.2;
        vec3 ambient = ambientStrength * Color;
        vec3 flatNormal = normalize(cross(dFdx(FragPos), dFdy(FragPos)));
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(flatNormal, lightDir), 0.0);
        vec3 diffuse = diff * Color;
        float specularStrength = 0.5;
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, flatNormal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * vec3(1.0);
        result = ambient + diffuse + specular;
    }
    // Highlight selected segment
    if (selectedSegmentID != -1 && vSegmentID == selectedSegmentID) {
        result = mix(result, vec3(1.0, 1.0, 0.0), 0.6);
        result *= 1.4;
    }
    FragColor = vec4(result, alpha);
}
