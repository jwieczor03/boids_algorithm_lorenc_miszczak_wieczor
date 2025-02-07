#version 330 core
layout (location = 0) in vec3 aPos;

out float Height;
out vec3 Position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    Height = aPos.y;
    Position = (model * vec4(aPos, 1.0)).xyz; // Wsp�rz�dne w �wiecie
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
