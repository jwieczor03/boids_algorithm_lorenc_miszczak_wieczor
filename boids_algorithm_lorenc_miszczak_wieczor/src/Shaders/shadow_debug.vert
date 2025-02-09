#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D shadowMap;
uniform float near_plane;
uniform float far_plane;

float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // Konwersja do NDC
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main() {             
    float depthValue = texture(shadowMap, TexCoords).r;
    float linearDepth = LinearizeDepth(depthValue) / far_plane; // Skalowanie do zakresu 0.0-1.0
    FragColor = vec4(vec3(linearDepth), 1.0); // Odcienie szaroœci
}