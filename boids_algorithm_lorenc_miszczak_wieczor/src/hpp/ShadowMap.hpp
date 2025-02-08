#pragma once
#include <GL/glew.h>

// Inicjalizacja shadow mapy
bool InitShadowMap(GLuint& FBO, GLuint& shadowMap, unsigned int width, unsigned int height);

// Powi¹zanie shadow mapy do zapisu
void BindShadowMapForWriting(GLuint FBO, unsigned int width, unsigned int height);

// Powi¹zanie shadow mapy do odczytu
void BindShadowMapForReading(GLuint shadowMap, GLenum textureUnit);
