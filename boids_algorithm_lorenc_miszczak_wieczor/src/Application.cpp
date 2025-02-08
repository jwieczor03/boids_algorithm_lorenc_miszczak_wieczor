#include "GL/glew.h"

#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"

#include <iostream>
#include <cmath>
#include "hpp/Project.hpp"



int main(int argc, char** argv)
{
	// inicjalizacja glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// tworzenie okna za pomoca glfw
	GLFWwindow* window = glfwCreateWindow(800, 600, "Boids ", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	//ruszanie kamer¹
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

	// ladowanie OpenGL za pomoca glewa
	glewInit();
	glViewport(0, 0, 800, 600);

	init(window);

	// uruchomienie glownej petli
	renderLoop(window);

	shutdown(window);
	glfwTerminate();
	return 0;
}
