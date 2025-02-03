#pragma once
#include <GL/glew.h>
#include <vector>
#include <string>

GLuint loadCubemap(const std::vector<std::string>& faces);
//GLuint loadTextures(const std::unordered_map<std::string, std::string>& textureFiles,
//    std::unordered_map<std::string, GLuint>& textureIDs);