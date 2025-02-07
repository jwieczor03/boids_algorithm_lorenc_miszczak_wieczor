#pragma once
#include <GL/glew.h>
#include <vector>
#include <string>

GLuint loadCubemap(const std::vector<std::string>& faces);
GLuint loadTexture(const char* path);
