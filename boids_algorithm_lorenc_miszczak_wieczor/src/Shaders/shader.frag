#version 330 core

out vec4 FragColor;

in float Height; // Wysokoœæ wierzcho³ka
in vec3 Position;

uniform sampler2D grassColor;
uniform sampler2D rockColor;
uniform sampler2D snowColor;

void main()
{
    // Normalizacja wysokoœci do przedzia³u [0,1]
    float h = (Height + 16.0) / 32.0; // Zakres -16 do 16 ? 0 do 1

    // Przedzia³y wysokoœci dla tekstur
    float grassLimit = 0.3;  // poni¿ej 30% wysokoœci
    float rockStart = 0.3;   // od 30% do 70% wysokoœci
    float snowStart = 0.7;   // powy¿ej 70% wysokoœci

    // Pobranie tekstur
    vec2 texCoords = Position.xz * 0.05; // Skalowanie wspó³rzêdnych tekstury
    vec4 grassTex = texture(grassColor, texCoords);
    vec4 rockTex = texture(rockColor, texCoords);
    vec4 snowTex = texture(snowColor, texCoords);

    // Blendowanie wysokoœciowe
    float grassWeight = smoothstep(0.0, grassLimit, h);
    float rockWeight = smoothstep(rockStart, snowStart, h);
    float snowWeight = smoothstep(snowStart, 1.0, h);

    vec4 finalColor = mix(grassTex, rockTex, rockWeight);
    finalColor = mix(finalColor, snowTex, snowWeight);

    FragColor = finalColor;
}
