#version 330 core
layout (location = 0) in vec3 aPos;
out vec3 TexCoords;
uniform mat4 projection;
uniform mat4 view;

void main() {

    TexCoords = aPos;
    mat4 viewWithoutTranslation = mat4(mat3(view));
    gl_Position = projection * viewWithoutTranslation * vec4(aPos, 1.0);
}