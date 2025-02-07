#version 330 core

out vec4 FragColor;

in float Height; // Wysoko�� wierzcho�ka
in vec3 Position;

uniform sampler2D grassColor;
uniform sampler2D rockColor;
uniform sampler2D snowColor;

void main()
{
    // Normalizacja wysoko�ci do przedzia�u [0,1]
    float h = (Height + 16.0) / 32.0; // Zakres -16 do 16 ? 0 do 1

    // Przedzia�y wysoko�ci dla tekstur
    float grassLimit = 0.3;  // poni�ej 30% wysoko�ci
    float rockStart = 0.3;   // od 30% do 70% wysoko�ci
    float snowStart = 0.7;   // powy�ej 70% wysoko�ci

    // Pobranie tekstur
    vec2 texCoords = Position.xz * 0.05; // Skalowanie wsp�rz�dnych tekstury
    vec4 grassTex = texture(grassColor, texCoords);
    vec4 rockTex = texture(rockColor, texCoords);
    vec4 snowTex = texture(snowColor, texCoords);

    // Blendowanie wysoko�ciowe
    float grassWeight = smoothstep(0.0, grassLimit, h);
    float rockWeight = smoothstep(rockStart, snowStart, h);
    float snowWeight = smoothstep(snowStart, 1.0, h);

    vec4 finalColor = mix(grassTex, rockTex, rockWeight);
    finalColor = mix(finalColor, snowTex, snowWeight);

    FragColor = finalColor;
}
