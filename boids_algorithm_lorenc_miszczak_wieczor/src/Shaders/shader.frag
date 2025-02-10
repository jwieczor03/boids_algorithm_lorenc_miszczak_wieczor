#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec4 FragPosLightSpace;
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;
in vec3 Normal;

uniform sampler2D shadowMap;

uniform vec3 viewPos;
uniform vec3 lightColor;
uniform bool lightingEnabled;
uniform bool shadowEnabled;

uniform sampler2D grassColorMap;
uniform sampler2D rockColorMap;
uniform sampler2D snowColorMap;

uniform sampler2D grassNormalMap;
uniform sampler2D rockNormalMap;
uniform sampler2D snowNormalMap;

const vec3 lightPos = vec3(8.2, 200.0, -10.0);

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    vec3 lightDir = normalize(TangentLightPos - TangentFragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    if (projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

void main() {
    vec2 texCoords = FragPos.xz * 0.05;

    vec3 grassColor = texture(grassColorMap, texCoords).rgb;
    vec3 rockColor  = texture(rockColorMap, texCoords).rgb;
    vec3 snowColor  = texture(snowColorMap, texCoords).rgb;

    vec3 grassNormal = texture(grassNormalMap, texCoords).rgb * 2.0 - 1.0;
    vec3 rockNormal  = texture(rockNormalMap, texCoords).rgb * 2.0 - 1.0;
    vec3 snowNormal  = texture(snowNormalMap, texCoords).rgb * 2.0 - 1.0;

    float h = (FragPos.y + 16.0) / 32.0;

    float grassLimit = 0.3;
    float rockStart = 0.3;
    float snowStart = 0.7;

    float grassWeight = smoothstep(0.0, grassLimit, h);
    float rockWeight = smoothstep(rockStart, snowStart, h);
    float snowWeight = smoothstep(snowStart, 1.0, h);

    vec3 finalColor = mix(grassColor, rockColor, vec3(rockWeight));
    finalColor = mix(finalColor, snowColor, vec3(snowWeight));

    vec3 blendedNormal = normalize(mix(grassNormal, rockNormal, vec3(rockWeight)));
    blendedNormal = normalize(mix(blendedNormal, snowNormal, vec3(snowWeight)));


    if (!lightingEnabled) {
        FragColor = vec4(finalColor, 1.0);
        return;
    }

    vec3 lightDir = normalize(TangentLightPos - TangentFragPos);
    float diff = max(dot(lightDir, blendedNormal), 0.0);

    vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(blendedNormal, halfwayDir), 0.0), 64.0);

    float shadow = shadowEnabled ? ShadowCalculation(FragPosLightSpace, blendedNormal) : 0.0;

    vec3 ambient = 0.2 * lightColor * finalColor;
    vec3 diffuse = diff * lightColor * finalColor;
    vec3 specular = spec * lightColor;

    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * lightColor;
    FragColor = vec4(result, 1.0);
}
