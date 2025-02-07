#pragma once
#include "GL/glew.h"
#include <vector>
#include <glm.hpp>

namespace Terrain {
    void initTerrain();
    void drawTerrain(GLuint program, const glm::mat4& viewMatrix, const glm::mat4& projMatrix, const glm::vec3& color);
    void shutdownTerrain();
}