#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include "GL/glew.h"
#include "../Headers/Shader_Loader.h"
#include "../Headers/Render_Utils.h"

#include "Terrain.hpp"
#include "TextureLoader.hpp"
#include "Skybox.hpp"
#include "ShadowMap.hpp"
#include "Headers/Boids.h"  // Dodaj odpowiedni include
#include <cstdlib>  // Dla rand()
#include <ctime> 
BoidSystem* boidSystem = nullptr;
GLuint boidsShader;



namespace fs = std::filesystem;
Core::Shader_Loader shaderLoader;
bool lightingEnabled = true;
bool shadowEnabled = true;


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
GLuint shadowFBO, shadowMap;
GLuint shadowShader;
GLuint qVAO, qVBO;
GLuint shadowDebugShader;
const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;



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


std::tuple<glm::mat4, glm::mat4, glm::mat4> createLightSpaceMatrix() {
    float terrainHalfWidth = 878;
    float terrainHalfHeight = 1312;

    glm::mat4 lightProjection = glm::ortho(
        -terrainHalfWidth,
        terrainHalfWidth,
        -terrainHalfHeight,
        terrainHalfHeight,
        0.1f,
        300.0f
    );

    glm::vec3 lightPos = glm::vec3(-100.0f, 200.0f, 10.0f);
    glm::vec3 lightDir = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));

    glm::mat4 lightView = glm::lookAt(
        lightPos,
        lightPos + lightDir,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    return std::make_tuple(lightSpaceMatrix, lightView, lightProjection);
}



// Obsługa Wejścia
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const float cameraSpeed = 5.0f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        lightingEnabled = !lightingEnabled; // Przełącz stan
        std::cout << "Lighting: " << (lightingEnabled ? "ON" : "OFF") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
        shadowEnabled = !shadowEnabled; // Przełącz stan
        std::cout << "Shadow: " << (shadowEnabled ? "ON" : "OFF") << std::endl;
    }
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

void setupDebugQuad()
{
    float quadVertices[] = {
        // Pozycje    // TexCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &qVAO);
    glGenBuffers(1, &qVBO);
    glBindVertexArray(qVAO);
    glBindBuffer(GL_ARRAY_BUFFER, qVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


// Zwalnianie zasobów (shaderów, terenu)
void shutdown(GLFWwindow* window) {

    shaderLoader.DeleteProgram(program);
    Terrain::shutdownTerrain();
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteTextures(1, &cubemapTexture);
    shaderLoader.DeleteProgram(skyboxShader);
    delete boidSystem;
    glDeleteProgram(boidsShader);
}

void renderShadowMap() {
    // 1. Renderuj do shadow map

    glm::mat4 lightSpaceMatrix, lightView, lightProjection;

    std::tie(lightSpaceMatrix, lightView, lightProjection) = createLightSpaceMatrix();

    glUseProgram(shadowShader);
    glUniformMatrix4fv(glGetUniformLocation(shadowShader, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Shadow Map Framebuffer Error: " << status << std::endl;
    }

    glClear(GL_DEPTH_BUFFER_BIT);
    Terrain::drawTerrain(shadowShader, lightView, lightProjection, glm::vec3(0.75f), cameraPos, lightingEnabled);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderSceneWithShadows() {
    glViewport(0, 0, 800, 600); // Ustawienia widoku
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = createCameraMatrix();
    glm::mat4 projection = createPerspectiveMatrix();

    glUniformMatrix4fv(glGetUniformLocation(boidsShader, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(boidsShader, "view"), 1, GL_FALSE, &view[0][0]);

    glm::mat4 lightSpaceMatrix, lightView, lightProjection;

    std::tie(lightSpaceMatrix, lightView, lightProjection) = createLightSpaceMatrix();

    glUseProgram(program);
    glUniformMatrix4fv(glGetUniformLocation(program, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);


    glActiveTexture(GL_TEXTURE15);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glUniform1i(glGetUniformLocation(program, "shadowMap"), 15);
    glUniform1i(glGetUniformLocation(program, "shadowEnabled"), shadowEnabled);

    // Rysowanie terenu z uwzględnieniem cieni
    Terrain::drawTerrain(program, view, projection, glm::vec3(0.75f), cameraPos, lightingEnabled);

}


void renderSkybox() {
    glDepthFunc(GL_LEQUAL);
    glm::mat4 viewWithoutTranslation = glm::mat4(glm::mat3(createCameraMatrix()));
    Skybox::drawSkybox(skyboxShader, viewWithoutTranslation, createPerspectiveMatrix());
    glDepthFunc(GL_LESS);
}

// Inicjalizacja sceny
void init(GLFWwindow* window) {

    glEnable(GL_DEPTH_TEST);
    //shadowDebugShader = shaderL
    // oader.CreateProgram("src/Shaders/shadow_debug.vert", "src/Shaders/shadow_debug.frag");

    boidSystem = new BoidSystem(300, 3);
    program = shaderLoader.CreateProgram("src/Shaders/shader.vert", "src/Shaders/shader.frag");
    skyboxShader = shaderLoader.CreateProgram("src/Shaders/skybox.vert", "src/Shaders/skybox.frag");
    boidsShader = shaderLoader.CreateProgram("src/Shaders/boids.vert", "src/Shaders/boids.frag");
    InitShadowMap(shadowFBO, shadowMap, SHADOW_WIDTH, SHADOW_HEIGHT);
    shadowShader = shaderLoader.CreateProgram("src/Shaders/shadow.vert", "src/Shaders/shadow.frag");
    Skybox::initSkybox();
    Terrain::initTerrain();
    setupDebugQuad();

}


void renderShadowMapDebug()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shadowDebugShader); // Użyj shaderów debugujących
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glUniform1i(glGetUniformLocation(shadowDebugShader, "shadowMap"), 0);

    glBindVertexArray(qVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}


// Funkcja rysująca scenę (teren)
void renderScene(GLFWwindow* window) {

    renderShadowMap();
    //renderShadowMapDebug(); // 1. Renderowanie shadow mapy
    renderSceneWithShadows();    // 2. Renderowanie sceny z cieniami
    renderSkybox();              // 3. Renderowanie skyboxa
    boidSystem->update();

    glUseProgram(boidsShader);
    glm::mat4 view = createCameraMatrix();
    glm::mat4 projection = createPerspectiveMatrix();
    boidSystem->draw(view, projection, boidsShader);


}


// G³ówna pêtla renderuj¹ca
void renderLoop(GLFWwindow* window) {

    while (!glfwWindowShouldClose(window)) {
        printf("Camerapos: %f, %f, %f, CameraDir: %f, %f, %f\n", cameraPos.x, cameraPos.y, cameraPos.z, cameraFront.x, cameraFront.y, cameraFront.z);
        processInput(window);
        renderScene(window);
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
}