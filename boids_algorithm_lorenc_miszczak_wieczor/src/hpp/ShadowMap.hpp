#pragma once
#include <GL/glew.h>

// Inicjalizacja shadow mapy
bool InitShadowMap(GLuint& FBO, GLuint& shadowMap, unsigned int width, unsigned int height);

