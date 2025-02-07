#include "hpp/Skybox.hpp"
#include "GL/glew.h"
#include <glm.hpp>
#include "hpp/TextureLoader.hpp"
#include <iostream>

namespace Skybox {
    GLuint skyboxVAO, skyboxVBO;
    GLuint cubemapTexture;

    float skyboxVertices[] = {      
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    void initSkybox() {
       
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        
        std::vector<std::string> faces = {
        "src/Textures/Skybox/right.jpg",   // GL_TEXTURE_CUBE_MAP_POSITIVE_X
        "src/Textures/Skybox/left.jpg",    // GL_TEXTURE_CUBE_MAP_NEGATIVE_X
        "src/Textures/Skybox/top.jpg",     // GL_TEXTURE_CUBE_MAP_POSITIVE_Y
        "src/Textures/Skybox/bottom.jpg",  // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
        "src/Textures/Skybox/front.jpg",   // GL_TEXTURE_CUBE_MAP_POSITIVE_Z
        "src/Textures/Skybox/back.jpg"     // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
        };

        cubemapTexture = loadCubemap(faces);

    }

    void drawSkybox(GLuint skyboxShader, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
        glUseProgram(skyboxShader);
        glUniform1i(glGetUniformLocation(skyboxShader, "skybox"), 0);
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "view"), 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);

        // Renderuj skybox
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    void shutdownSkybox() {
        glDeleteVertexArrays(1, &skyboxVAO);
        glDeleteBuffers(1, &skyboxVBO);
    }
}
