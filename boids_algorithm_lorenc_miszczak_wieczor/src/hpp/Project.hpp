
#include "GL/glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>

#include "../Headers/Shader_Loader.h"
#include "../Headers/Render_Utils.h"

#include "Terrain.hpp"
#include "TextureLoader.hpp"
#include "Skybox.hpp"


namespace fs = std::filesystem;
Core::Shader_Loader shaderLoader;

// Parametry kamery
glm::vec3 cameraPos = glm::vec3(8.2f, 9.8f, -10.0f);  //Pozycja kamery
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); //Wektor patrzenia kamery
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //Wektor góry kamery

// Światło
glm::vec3 lightPos = glm::vec3(8.2f, 9.8f, -10.0f); 


// Zmienne pomocnicze dla obsługi myszy
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 800.0f / 2.0f;
float lastY = 600.0f / 2.0f;

// Identyfikator programu shaderowego
GLuint program;
// Zmienne globalne
GLuint skyboxVAO, skyboxVBO;
GLuint cubemapTexture;
GLuint skyboxShader;


// Ustawienia rzutowania
float aspectRatio = 1.f;
const float fov = glm::radians(90.0f);

// Funkcja tworz¹ca macierz widoku na podstawie pozycji i kierunku kamery
glm::mat4 createCameraMatrix() {

    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

// Funkcja tworząca macierz perspektywy
glm::mat4 createPerspectiveMatrix() {

    float nearPlane = 0.05f;
    float farPlane = 1000.0f;
    return glm::perspective(fov, aspectRatio, nearPlane, farPlane);
}


// Obsługa Wejścia
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const float cameraSpeed = 0.2f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// Callback obs³ugi ruchu myszy – aktualizacja kierunku patrzenia kamery
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Odwrócone, gdy¿ wspó³rzêdne y rosn¹ od do³u do góry
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Ograniczenie k¹ta pitch, by zapobiec "przewróceniu" kamery
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// Zwalnianie zasobów (shaderów, terenu)
void shutdown(GLFWwindow* window) {

    shaderLoader.DeleteProgram(program);
    Terrain::shutdownTerrain();
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteTextures(1, &cubemapTexture);
    shaderLoader.DeleteProgram(skyboxShader);
}

// Inicjalizacja sceny
void init(GLFWwindow* window) {

    glEnable(GL_DEPTH_TEST);
    program = shaderLoader.CreateProgram("src/Shaders/shader.vert", "src/Shaders/shader.frag");
    skyboxShader = shaderLoader.CreateProgram("src/Shaders/skybox.vert", "src/Shaders/skybox.frag");
    Skybox::initSkybox();
    Terrain::initTerrain();


}

// Funkcja rysująca scenę (teren)
void renderScene(GLFWwindow* window) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Renderowanie terenu 
    glm::mat4 viewProjection = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 view = createCameraMatrix();
    glm::mat4 projection = createPerspectiveMatrix();


    glDepthFunc(GL_LEQUAL);
    glm::mat4 viewWithoutTranslation = glm::mat4(glm::mat3(createCameraMatrix()));
    Skybox::drawSkybox(skyboxShader, viewWithoutTranslation, projection);

    glDepthFunc(GL_LESS);//Domyślne ustawienia głębi

    Terrain::drawTerrain(program, view, projection, glm::vec3(0.75f));



}

// G³ówna pêtla renderuj¹ca
void renderLoop(GLFWwindow* window) {

    while (!glfwWindowShouldClose(window)) {
        printf("Camerapos: %f, %f, %f\n", cameraPos.x, cameraPos.y, cameraPos.z);
        processInput(window);
        renderScene(window);
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
}
