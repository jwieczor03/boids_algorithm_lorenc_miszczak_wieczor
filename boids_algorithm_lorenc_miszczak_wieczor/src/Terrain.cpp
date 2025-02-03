#include "hpp/Terrain.hpp"
#include "GL/glew.h"
#include <glm.hpp>

namespace Terrain {
    GLuint VAO, VBO;
    int vertexCount;

    void initTerrain() {
        const int GRID_SIZE = 100;
        const float STEP = 0.1f;
        const float SCALE = 5.0f;
        std::vector<float> vertices;

        for (int i = 0; i < GRID_SIZE; ++i) {
            for (int j = 0; j < GRID_SIZE; ++j) {
                float x0 = -SCALE + i * STEP;
                float z0 = -SCALE + j * STEP;
                float x1 = x0 + STEP;
                float z1 = z0 + STEP;

                // Triangle 1
                vertices.insert(vertices.end(), { x0, 0.0f, z0 });
                vertices.insert(vertices.end(), { x1, 0.0f, z0 });
                vertices.insert(vertices.end(), { x0, 0.0f, z1 });

                // Triangle 2
                vertices.insert(vertices.end(), { x1, 0.0f, z0 });
                vertices.insert(vertices.end(), { x1, 0.0f, z1 });
                vertices.insert(vertices.end(), { x0, 0.0f, z1 });
            }
        }

        vertexCount = static_cast<int>(vertices.size()) / 3;

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    }

    void drawTerrain(GLuint program, const glm::mat4& viewProjectionMatrix, const glm::vec3& color) {
        glUseProgram(program);
        glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, &viewProjectionMatrix[0][0]);
        glUniform3fv(glGetUniformLocation(program, "color"), 1, &color[0]);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glUseProgram(0);
    }

    void shutdownTerrain() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
}