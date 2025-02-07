#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform bool lightingEnabled;

uniform sampler2D grassColor;
uniform sampler2D rockColor;
uniform sampler2D snowColor;

void main()
{
    // Normalizacja wektora normalnego
    vec3 norm = normalize(Normal);

    // Kierunek do œwiat³a
    vec3 lightDir = normalize(lightPos - FragPos);

    // Wspó³czynnik dyfuzyjny (Lambert)
    float diff = max(dot(norm, lightDir), 0.0);

    // Kierunek odbicia œwiat³a
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    
    // Sk³adnik specular (Blinn-Phong)
    float specularStrength = 0.5;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32);

    // Mieszanie tekstur zale¿nie od wysokoœci
    float h = (FragPos.y + 16.0) / 32.0; // Normalizacja wysokoœci

    float grassLimit = 0.3;
    float rockStart = 0.3;
    float snowStart = 0.7;

    vec2 texCoords = FragPos.xz * 0.05;
    vec4 grassTex = texture(grassColor, texCoords);
    vec4 rockTex = texture(rockColor, texCoords);
    vec4 snowTex = texture(snowColor, texCoords);

    float grassWeight = smoothstep(0.0, grassLimit, h);
    float rockWeight = smoothstep(rockStart, snowStart, h);
    float snowWeight = smoothstep(snowStart, 1.0, h);

    vec4 finalColor = mix(grassTex, rockTex, rockWeight);
    finalColor = mix(finalColor, snowTex, snowWeight);
        if (!lightingEnabled) {
        FragColor = finalColor; // Jeœli oœwietlenie wy³¹czone, u¿yj samej tekstury
        return;
    }

    // Finalne oœwietlenie Phonga
    vec3 ambient = 0.2 * lightColor * finalColor.rgb;
    vec3 diffuse = diff * lightColor * finalColor.rgb;
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
