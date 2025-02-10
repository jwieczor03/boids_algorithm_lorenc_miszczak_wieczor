#version 330 core
out vec4 FragColor;

uniform vec3 color;

in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;
uniform sampler2D shadowMap;

uniform vec3 viewPos;
uniform bool lightingEnabled;
uniform bool shadowEnabled;

const vec3 lightPos = vec3(8.2, 200.0, -10.0);
const vec3 lightColor = vec3(0.98, 0.9, 0.823);

float ShadowCalculation(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        

    return shadow;
}

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

    float shadow = ShadowCalculation(FragPosLightSpace);


    if (!shadowEnabled) {
            shadow = 0.0;
        }

    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * lightColor; 
    FragColor = vec4(result, 1.0); 
}
