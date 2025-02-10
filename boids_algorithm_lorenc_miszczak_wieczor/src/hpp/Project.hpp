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
#include "Headers/Boids.h" 
#include <cstdlib> 
#include <ctime> 
namespace fs = std::filesystem;
Core::Shader_Loader shaderLoader;
bool lightingEnabled = true;
bool shadowEnabled = true;
bool normalMappingEnabled = true;
bool attractionMode = true; // true - przyciąganie, false - odpychanie


// Parametry kamery
glm::vec3 cameraPos = glm::vec3(8.2f, 9.8f, -10.0f);  //Pozycja kamery
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); //Wektor patrzenia kamery
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //Wektor góry kamery

// Pozycja światła
glm::vec3 lightPos = glm::vec3(8.2f, 50.0f, -10.0f);


// Zmienne pomocnicze dla obsługi myszy
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 800.0f / 2.0f;
float lastY = 600.0f / 2.0f;

// Zmienne globalne
GLuint program;
GLuint skyboxVAO, skyboxVBO;
GLuint cubemapTexture;
GLuint skyboxShader;
GLuint shadowFBO, shadowMap;
GLuint shadowShader;
GLuint qVAO, qVBO;
//GLuint shadowDebugShader; //Do debugowania shadow mapy
const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
BoidSystem* boidSystem = nullptr;
GLuint boidsShader;
bool cameraControl = true; // Tryb sterowania kamerą (true = sterowanie kamerą, false = interakcja z boidami)


// Ustawienia rzutowania
float aspectRatio = 1.f;
const float fov = glm::radians(90.0f);


// Funkcja tworząca macierz widoku na podstawie pozycji i kierunku kamery
glm::mat4 createCameraMatrix() {

    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

// Funkcja tworząca macierz perspektywy
glm::mat4 createPerspectiveMatrix() {

    float nearPlane = 0.05f;
    float farPlane = 1000.0f;
    return glm::perspective(fov, aspectRatio, nearPlane, farPlane);
}


// Funkcja tworząca macierz widoku światła do shadow mapy
std::tuple<glm::mat4, glm::mat4, glm::mat4> createLightSpaceMatrix() {
    float terrainHalfWidth = 878;
    float terrainHalfHeight = 1312;

    glm::mat4 lightProjection = glm::ortho(
        -terrainHalfWidth,
        terrainHalfWidth,
        -terrainHalfHeight,
        terrainHalfHeight,
        0.05f,
        1000.0f
    );

    glm::vec3 lightDir = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));

    glm::mat4 lightView = glm::lookAt(
        lightPos,
        lightPos + lightDir,
        glm::vec3(1.0f, 0.0f, 0.0f)
    );
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    return std::make_tuple(lightSpaceMatrix, lightView, lightProjection);
}


// Obsługa Wejścia
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Sprawdzenie, czy użytkownik trzyma SHIFT
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        cameraControl = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // Pokazujemy kursor
    }
    else {
        cameraControl = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Ukrywamy kursor
    }


    if (cameraControl) { // Tylko jeśli kontrolujemy kamerę
        const float cameraSpeed = 5.0f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }

        
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        lightingEnabled = !lightingEnabled;
        std::cout << "Lighting: " << (lightingEnabled ? "ON" : "OFF") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
        shadowEnabled = !shadowEnabled;
        std::cout << "Shadow: " << (shadowEnabled ? "ON" : "OFF") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) { // Toggle normal mapping
        normalMappingEnabled = !normalMappingEnabled;
        std::cout << "Normal Mapping: " << (normalMappingEnabled ? "ON" : "OFF") << std::endl;
    }
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        attractionMode = false; // LPM - odpychanie
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        attractionMode = true; // PPM - przyciąganie
    }
}


// Callback obsługi ruchu myszy – aktualizacja kierunku patrzenia kamery
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {

    if (!cameraControl) return;// Jeśli nie sterujemy kamerą, nie ruszamy myszą

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

glm::vec3 getMouseWorldPosition(GLFWwindow* window) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Przekształcenie współrzędnych ekranu na zakres [-1,1]
    float x = (2.0f * xpos) / width - 1.0f;
    float y = 1.0f - (2.0f * ypos) / height; // OpenGL ma odwrotną oś Y
    float z = 1.0f; // Zakładamy rzutowanie w kierunku obserwacji

    glm::mat4 view = createCameraMatrix();
    glm::mat4 projection = createPerspectiveMatrix();

    glm::vec4 ray_clip = glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
    glm::vec3 ray_wor = glm::vec3(glm::inverse(view) * ray_eye);
    ray_wor = glm::normalize(ray_wor);

    // Obliczenie punktu na płaszczyźnie Y=0
    float t = -cameraPos.y / ray_wor.y;
    glm::vec3 intersectionPoint = cameraPos + t * ray_wor;

    return intersectionPoint;
}


//Dodatkowa funkjca do debugowania shadow mapy
//void renderQuad()
//{
//    if (qVAO == 0)
//    {
//        float quadVertices[] = {
//            // positions        // texture Coords
//            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
//            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
//             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
//             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
//        };
//        // setup plane VAO
//        glGenVertexArrays(1, &qVAO);
//        glGenBuffers(1, &qVBO);
//        glBindVertexArray(qVAO);
//        glBindBuffer(GL_ARRAY_BUFFER, qVBO);
//        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
//        glEnableVertexAttribArray(0);
//        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
//        glEnableVertexAttribArray(1);
//        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
//    }
//    glBindVertexArray(qVAO);
//    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//    glBindVertexArray(0);
//}

//Dodatkowa funkcja do debugowania shadow mapy
//void renderShadowMapDebug()
//{
//    glDisable(GL_DEPTH_TEST);
//    glViewport(0, 0, 800, 600);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//    glUseProgram(shadowDebugShader); // Użyj shaderów debugujących
//
//    float nearPlane = 0.05f;
//    float farPlane = 1000.0f;
//    glUniform1f(glGetUniformLocation(shadowDebugShader, "near_plane"), nearPlane);
//    glUniform1f(glGetUniformLocation(shadowDebugShader, "far_plane"), farPlane);
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, shadowMap);
//    glUniform1i(glGetUniformLocation(shadowDebugShader, "depthMap"), 0);
//    
//    renderQuad();
//    glEnable(GL_DEPTH_TEST);
//}

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

// Renderowanie shadow mapy
void renderShadowMap() {

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
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glCullFace(GL_FRONT);

    boidSystem->draw(lightView, lightProjection, cameraPos, lightingEnabled, shadowShader);
    Terrain::drawTerrain(shadowShader, lightView, lightProjection, glm::vec3(0.75f), cameraPos, lightingEnabled);


    glCullFace(GL_BACK);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//Renderowanie sceny z uwzględnieniem cieni
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
    glUniform1i(glGetUniformLocation(program, "normalMappingEnabled"), normalMappingEnabled);

    // Rysowanie terenu z uwzględnieniem cieni
    Terrain::drawTerrain(program, view, projection, glm::vec3(0.75f), cameraPos, lightingEnabled);

    glUseProgram(boidsShader);
    glUniformMatrix4fv(glGetUniformLocation(boidsShader, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);


    glActiveTexture(GL_TEXTURE15);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glUniform1i(glGetUniformLocation(boidsShader, "shadowMap"), 15);
    glUniform1i(glGetUniformLocation(boidsShader, "shadowEnabled"), shadowEnabled);
    boidSystem->draw(view, projection, cameraPos, lightingEnabled, boidsShader);

}
// Renderowanie skyboxa
void renderSkybox() {
    glDepthFunc(GL_LEQUAL);
    glm::mat4 viewWithoutTranslation = glm::mat4(glm::mat3(createCameraMatrix()));
    Skybox::drawSkybox(skyboxShader, viewWithoutTranslation, createPerspectiveMatrix());
    glDepthFunc(GL_LESS);
}

// Inicjalizacja sceny, ładowanie shaderów
void init(GLFWwindow* window) {

    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    glfwSetMouseButtonCallback(window, mouse_button_callback); // Rejestracja obsługi kliknięć myszy
	glfwSetCursorPosCallback(window, mouse_callback); // Rejestracja obsługi ruchu myszy
    //shadowDebugShader = shaderLoader.CreateProgram("src/Shaders/shadow_debug.vert", "src/Shaders/shadow_debug.frag");

    boidSystem = new BoidSystem(300, 3);
    program = shaderLoader.CreateProgram("src/Shaders/shader.vert", "src/Shaders/shader.frag");
    skyboxShader = shaderLoader.CreateProgram("src/Shaders/skybox.vert", "src/Shaders/skybox.frag");
    boidsShader = shaderLoader.CreateProgram("src/Shaders/boids.vert", "src/Shaders/boids.frag");
    InitShadowMap(shadowFBO, shadowMap, SHADOW_WIDTH, SHADOW_HEIGHT);
    shadowShader = shaderLoader.CreateProgram("src/Shaders/shadow.vert", "src/Shaders/shadow.frag");
    Skybox::initSkybox();
    Terrain::initTerrain();

}


// Funkcja rysująca scenę (teren)
void renderScene(GLFWwindow* window) {
    boidSystem->update(window);       //Aktualizacja boidów
    renderShadowMap();          //Renderowanie shadow map  

    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "normalMappingEnabled"), normalMappingEnabled);

    //renderShadowMapDebug();   //Do visual debugowania shadow mapy
    renderSceneWithShadows();    //Renderowanie sceny z cieniami
    renderSkybox();              //Renderowanie skyboxa


}



// Główna pętla renderująca
void renderLoop(GLFWwindow* window) {

    while (!glfwWindowShouldClose(window)) {
        //printf("Camerapos: %f, %f, %f, CameraDir: %f, %f, %f\n", cameraPos.x, cameraPos.y, cameraPos.z, cameraFront.x, cameraFront.y, cameraFront.z);
        processInput(window);

        // Interakcja z boidami tylko gdy kamera NIE jest sterowana
        if (!cameraControl && (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ||
            glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)) {
            glm::vec3 mouseWorldPos = getMouseWorldPosition(window);
            boidSystem->applyMouseForce(mouseWorldPos, attractionMode);
        }


        renderScene(window);
        printf("Max Speed: %f\n", boidSystem->getMaxSpeed());
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
}
