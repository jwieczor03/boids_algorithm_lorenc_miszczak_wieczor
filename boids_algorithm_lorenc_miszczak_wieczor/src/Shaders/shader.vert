#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec3 aBitangent;

out vec3 FragPos;
out vec4 FragPosLightSpace;
out vec3 TangentLightPos;
out vec3 TangentViewPos;
out vec3 TangentFragPos;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;
uniform vec3 viewPos;

const vec3 lightPos = vec3(8.2, 200.0, -10.0);

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    mat3 TBN = transpose(mat3(T, B, N));    
    TangentLightPos = TBN * lightPos;
    TangentViewPos  = TBN * viewPos;
    TangentFragPos  = TBN * FragPos;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0); 
}
