#include "Headers/Shader_Loader.h" 
#include<iostream>
#include<fstream>
#include<vector>
#include<filesystem>

using namespace Core;
namespace fs = std::filesystem;

Shader_Loader::Shader_Loader(void) {}
Shader_Loader::~Shader_Loader(void) {}

void listFilesInDirectory(const std::string& path) {
	std::cout << "Files in directory " << path << ":" << std::endl;
	for (const auto& entry : fs::directory_iterator(path)) {
		std::cout << entry.path() << std::endl;
	}
}

std::string Shader_Loader::ReadShader(const char* filename)
{

	std::string shaderCode;
	std::ifstream file(filename, std::ios::in);

	std::cout << "Attempting to read file: " << filename << std::endl;

	if (!file.good())
	{
		std::cout << "Can't read file " << filename << std::endl;
		listFilesInDirectory(fs::current_path().string());
		std::terminate();
	}

	file.seekg(0, std::ios::end);
	shaderCode.resize((unsigned int)file.tellg());
	file.seekg(0, std::ios::beg);
	file.read(&shaderCode[0], shaderCode.size());
	file.close();
	return shaderCode;
}

GLuint Shader_Loader::CreateShader(GLenum shaderType, std::string
	source, const char* shaderName)
{

	int compile_result = 0;

	GLuint shader = glCreateShader(shaderType);
	const char* shader_code_ptr = source.c_str();
	const int shader_code_size = source.size();

	glShaderSource(shader, 1, &shader_code_ptr, &shader_code_size);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_result);

	//sprawdz bledy
	if (compile_result == GL_FALSE)
	{

		int info_log_length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector<char> shader_log(info_log_length);
		glGetShaderInfoLog(shader, info_log_length, NULL, &shader_log[0]);
		std::cout << "ERROR compiling shader: " << shaderName << std::endl << &shader_log[0] << std::endl;
		return 0;
	}

	return shader;
}

GLuint Shader_Loader::CreateProgram(const char* vertexShaderFilename,
	const char* fragmentShaderFilename)
{

	//wczytaj shadery
	std::string vertex_shader_code = ReadShader(vertexShaderFilename);
	std::string fragment_shader_code = ReadShader(fragmentShaderFilename);

	GLuint vertex_shader = CreateShader(GL_VERTEX_SHADER, vertex_shader_code, "vertex shader");
	GLuint fragment_shader = CreateShader(GL_FRAGMENT_SHADER, fragment_shader_code, "fragment shader");

	int link_result = 0;
	//stworz shader
	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_result);
	//sprawdz bledy w linkerze
	if (link_result == GL_FALSE)
	{

		int info_log_length = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector<char> program_log(info_log_length);
		glGetProgramInfoLog(program, info_log_length, NULL, &program_log[0]);
		std::cout << "Shader Loader : LINK ERROR" << std::endl << &program_log[0] << std::endl;
		return 0;
	}

	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	return program;
}

void Shader_Loader::DeleteProgram(GLuint program)
{
	glDeleteProgram(program);
}
