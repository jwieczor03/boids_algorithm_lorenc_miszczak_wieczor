#pragma once
#include "GL/glew.h"
#include <vector>
#include <glm.hpp>

namespace Skybox {
    void initSkybox();
    void drawSkybox(GLuint program, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
    void shutdownSkybox();
}