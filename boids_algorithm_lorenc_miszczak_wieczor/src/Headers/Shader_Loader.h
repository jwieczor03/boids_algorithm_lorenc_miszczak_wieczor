#pragma once

#include "GL/glew.h"
#include "freeglut.h"
#include <iostream>

namespace Core
{

	class Shader_Loader
	{
	private:

		std::string ReadShader(const char* filename);
		GLuint CreateShader(GLenum shaderType,
			std::string source,
			const char* shaderName);

	public:

		Shader_Loader(void);
		~Shader_Loader(void);
		GLuint CreateProgram(const char* VertexShaderFilename,
			const char* FragmentShaderFilename);

		void DeleteProgram(GLuint program);

	};
}