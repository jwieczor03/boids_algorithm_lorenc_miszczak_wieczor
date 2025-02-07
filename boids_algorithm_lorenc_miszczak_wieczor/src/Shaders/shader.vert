#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal; // Normalne wczytane z VAO

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0)); // Pozycja w œwiecie
    Normal = mat3(transpose(inverse(model))) * aNormal; // Transformacja normalnych

    gl_Position = projection * view * vec4(FragPos, 1.0);
}
