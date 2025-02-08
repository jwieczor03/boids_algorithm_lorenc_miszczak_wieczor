
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal; // Normalne wczytane z VAO

out vec3 FragPos;
out vec3 Normal;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0)); // Pozycja w ?wiecie
    Normal = transpose(inverse(mat3(model))) * aNormal; // Transformacja normalnych

    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0); // U�yj pozycji w przestrzeni �wiata
}
