#ifndef BOIDS_H
#define BOIDS_H

#include <glm.hpp>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Boid {
public:
    Boid(glm::vec3 position, glm::vec3 velocity, int group, glm::vec3 color);

    void updatePosition(const glm::vec3& newPos);
    glm::vec3 getPosition() const;
    glm::vec3 getVelocity() const;
    void setVelocity(const glm::vec3& velocity);
    void draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos, bool lightingEnabled, GLuint shader) const;
    glm::vec3 getColor() const;
    glm::vec3 position;
    glm::vec3 velocity;

private:
    int group;
    glm::vec3 color;

    static GLuint VAO, VBO;
    static const float BOID_VERTICES[63];
    static void initializeGeometry();
};

class BoidSystem {
public:
    BoidSystem(int numBoids, int numGroups);
    void handleInput(GLFWwindow* window);
    void update(GLFWwindow* window);
    void draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos, bool lightingEnabled, GLuint shader) const;
    float getMaxSpeed() const;
    void applyMouseForce(glm::vec3 mousePos, bool attract);

private:
    std::vector<Boid> boids;
    float SEPARATION_RADIUS = 1.0f;
    float ALIGNMENT_RADIUS = 3.0f;
    float COHESION_RADIUS = 3.0f;
    float MAX_SPEED = 1.0f;
    const float BOUNDARY = 50.0f;

    glm::vec3 separation(const Boid& boid) const;
    glm::vec3 alignment(const Boid& boid) const;
    glm::vec3 cohesion(const Boid& boid) const;
    void keepWithinBounds(Boid& boid);
    glm::vec3 limit(glm::vec3 vec, float max) const;
    bool checkCollision(const Boid& a, const Boid& b) const;
    void resolveCollisions(); 
};

#endif // BOIDS_H
