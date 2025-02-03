#pragma once
#include "GL/glew.h"
#include <vector>
#include <glm.hpp>

namespace Terrain {
    void initTerrain();
    void drawTerrain(GLuint program, const glm::mat4& viewProjectionMatrix, const glm::vec3& color);
    void shutdownTerrain();
}