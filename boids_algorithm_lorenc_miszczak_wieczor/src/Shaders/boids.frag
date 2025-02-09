 #version 330 core
out vec4 FragColor;

uniform vec3 color;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform bool lightingEnabled;

const vec3 lightPos = vec3(-9.0, 200.0, 10.0);
const vec3 lightColor = vec3(0.98, 0.9, 0.823);

void main() {

    if (!lightingEnabled) {
        FragColor = vec4(color, 1.0);
        return;
    }
    vec3 norm = normalize(Normal);

    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(lightDir, norm), 0.0);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0);

    vec3 ambient = 0.2 * lightColor * color;
    vec3 diffuse = diff * lightColor * color;
    vec3 specular = spec * lightColor;


    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
