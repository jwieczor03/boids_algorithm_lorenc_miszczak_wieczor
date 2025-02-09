#ifndef BOIDS_H
#define BOIDS_H

#include <glm.hpp>
#include <vector>
#include <GL/glew.h>

class Boid {
public:
    Boid(glm::vec3 position, glm::vec3 velocity, int group, glm::vec3 color);

    void updatePosition(const glm::vec3& newPos);
    glm::vec3 getPosition() const;
    glm::vec3 getVelocity() const;
    void setVelocity(const glm::vec3& velocity);
    void draw(const glm::mat4& view, const glm::mat4& projection, GLuint shader) const;
    glm::vec3 getColor() const;

private:
    glm::vec3 position;
    glm::vec3 velocity;
    int group;
    glm::vec3 color;

    static GLuint VAO, VBO;
    static const float BOID_VERTICES[36];
    static void initializeGeometry();
};

class BoidSystem {
public:
    BoidSystem(int numBoids, int numGroups);
    void update();
    void draw(const glm::mat4& view, const glm::mat4& projection, GLuint shader) const;

private:
    std::vector<Boid> boids;
    const float SEPARATION_RADIUS = 1.0f;
    const float ALIGNMENT_RADIUS = 3.0f;
    const float COHESION_RADIUS = 3.0f;
    const float MAX_SPEED = 1.0f;
    const float BOUNDARY = 50.0f;

    glm::vec3 separation(const Boid& boid) const;
    glm::vec3 alignment(const Boid& boid) const;
    glm::vec3 cohesion(const Boid& boid) const;
    void keepWithinBounds(Boid& boid);
    glm::vec3 limit(glm::vec3 vec, float max) const;
    bool checkCollision(const Boid& a, const Boid& b) const;
    void resolveCollisions(); // Added declaration
};

#endif // BOIDS_H
