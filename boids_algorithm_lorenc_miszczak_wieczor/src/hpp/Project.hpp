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
glm::vec3 cameraPos = glm::vec3(2.0f, 2.0f, 2.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// Zmienne pomocnicze dla obs³ugi myszy
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

// Funkcja tworz¹ca macierz perspektywy
glm::mat4 createPerspectiveMatrix() {
    float nearPlane = 0.05f;
    float farPlane = 20.0f;
    return glm::perspective(fov, aspectRatio, nearPlane, farPlane);
}

// Funkcja rysuj¹ca scenê – w tym przypadku teren
void renderScene(GLFWwindow* window) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Renderowanie terenu (zwyk³e obiekty)
    glm::mat4 viewProjection = createPerspectiveMatrix() * createCameraMatrix();
    Terrain::drawTerrain(program, viewProjection, glm::vec3(0.75f));

    glDepthFunc(GL_LEQUAL);
    glUseProgram(skyboxShader);

    // Usuñ translacjê z macierzy widoku (tylko rotacja)
    glm::mat4 view = glm::mat4(glm::mat3(createCameraMatrix()));
    glm::mat4 projection = createPerspectiveMatrix();

    glUniform1i(glGetUniformLocation(skyboxShader, "skybox"), 0);
    // Przeka¿ macierze do shadera
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "projection"), 1, GL_FALSE, &projection[0][0]);

    // Renderuj skybox
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS); // Przywróæ domyœlne testowanie g³êbi

    glfwSwapBuffers(window);
}

// Callback zmiany rozmiaru okna – aktualizacja viewportu i wspó³czynnika proporcji
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    aspectRatio = width / float(height);
    glViewport(0, 0, width, height);
}

// Inicjalizacja sceny: ustawienie callbacków, w³¹czenie testu g³êbokoœci, inicjalizacja shaderów i terenu
void init(GLFWwindow* window) {
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glEnable(GL_DEPTH_TEST);
    program = shaderLoader.CreateProgram("src/Shaders/shader.vert", "src/Shaders/shader.frag");
    skyboxShader = shaderLoader.CreateProgram("src/Shaders/skybox.vert", "src/Shaders/skybox.frag");
    Terrain::initTerrain(); ;

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    std::vector<std::string> faces = {
        "src/Textures/Skybox/right.jpg",   // GL_TEXTURE_CUBE_MAP_POSITIVE_X
        "src/Textures/Skybox/left.jpg",    // GL_TEXTURE_CUBE_MAP_NEGATIVE_X
        "src/Textures/Skybox/top.jpg",     // GL_TEXTURE_CUBE_MAP_POSITIVE_Y
        "src/Textures/Skybox/bottom.jpg",  // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
        "src/Textures/Skybox/front.jpg",   // GL_TEXTURE_CUBE_MAP_POSITIVE_Z
        "src/Textures/Skybox/back.jpg"     // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    cubemapTexture = loadCubemap(faces);
}

// Obs³uga wejœcia – klawisze steruj¹ce kamer¹
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const float cameraSpeed = 0.005f;
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

// G³ówna pêtla renderuj¹ca
void renderLoop(GLFWwindow* window) {
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        renderScene(window);
        glfwPollEvents();
    }
}