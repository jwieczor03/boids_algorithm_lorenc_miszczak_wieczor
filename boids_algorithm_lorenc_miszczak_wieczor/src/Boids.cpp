#include "Headers/boids.h"
#include <cmath>
#include <algorithm>
#include <GL/glew.h>
#include <gtc/matrix_transform.hpp>
#include <ctime>

// Statyczne zmienne klasy Boid
GLuint Boid::VAO = 0;
GLuint Boid::VBO = 0;

const float Boid::BOID_VERTICES[] = {
    // Front triangle
    0.0f, 0.5f, 0.0f,
    -0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, 0.5f,

    // Right triangle
    0.0f, 0.5f, 0.0f,
    0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, -0.5f,

    // Left triangle
    0.0f, 0.5f, 0.0f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, 0.5f,

    // Back triangle
    0.0f, 0.5f, 0.0f,
    0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f
};

void Boid::initializeGeometry() {
    if (VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(BOID_VERTICES), BOID_VERTICES, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

Boid::Boid(glm::vec3 position, glm::vec3 velocity, int group, glm::vec3 color)
    : position(position), velocity(velocity), group(group), color(color) {
    initializeGeometry();
}

void Boid::updatePosition(const glm::vec3& newPos) {
    position = newPos;
}

glm::vec3 Boid::getPosition() const { return position; }
glm::vec3 Boid::getVelocity() const { return velocity; }
glm::vec3 Boid::getColor() const { return color; }

void Boid::setVelocity(const glm::vec3& velocity) {
    this->velocity = velocity;
}

void Boid::draw(const glm::mat4& view, const glm::mat4& projection, GLuint shader) const {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);

    if (glm::length(velocity) > 0.001f) {
        glm::vec3 dir = glm::normalize(velocity);
        float angle = atan2(dir.x, dir.z);
        model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform3fv(glGetUniformLocation(shader, "color"), 1, &color[0]);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    glBindVertexArray(0);
}

BoidSystem::BoidSystem(int numBoids, int numGroups) {
    srand(static_cast<unsigned int>(time(nullptr)));
    for (int i = 0; i < numBoids; ++i) {
        int group = i % numGroups;
        glm::vec3 color;
        switch (group) {
        case 0: color = glm::vec3(1.0f, 0.0f, 0.0f); break;
        case 1: color = glm::vec3(0.0f, 1.0f, 0.0f); break;
        default: color = glm::vec3(0.0f, 0.0f, 1.0f);
        }

        glm::vec3 pos(
            (rand() % 100 - 50) * 0.5f,
            (rand() % 100) * 0.1f + 2.0f,
            (rand() % 100 - 50) * 0.5f
        );

        glm::vec3 vel(
            (rand() % 100 - 50) * 0.01f,
            0.0f,
            (rand() % 100 - 50) * 0.01f
        );

        boids.emplace_back(pos, vel, group, color);
    }
}

void BoidSystem::update() {
    for (auto& boid : boids) {
        glm::vec3 sep = separation(boid);
        glm::vec3 ali = alignment(boid);
        glm::vec3 coh = cohesion(boid);

        glm::vec3 acceleration = sep * 1.5f + ali * 1.0f + coh * 1.2f;
        glm::vec3 newVelocity = boid.getVelocity() + acceleration;
        newVelocity = limit(newVelocity, MAX_SPEED);

        boid.setVelocity(newVelocity);
        boid.updatePosition(boid.getPosition() + newVelocity * 0.1f);
        keepWithinBounds(boid);
    }
    resolveCollisions();
}

void BoidSystem::draw(const glm::mat4& view, const glm::mat4& projection, GLuint shader) const {
    for (const auto& boid : boids) {
        boid.draw(view, projection, shader);
    }
}

bool BoidSystem::checkCollision(const Boid& a, const Boid& b) const {
    if (a.getColor() != b.getColor()) return false; // Tylko boidy tej samej grupy się odpychają

    float size = 0.5f; // Zmienna zależna od rozmiaru boida
    glm::vec3 posA = a.getPosition();
    glm::vec3 posB = b.getPosition();

    // Zmienna do wykrywania kolizji oparta na odległości
    return glm::distance(posA, posB) < size;
}

void BoidSystem::resolveCollisions() {
    for (size_t i = 0; i < boids.size(); ++i) {
        for (size_t j = i + 1; j < boids.size(); ++j) {
            if (checkCollision(boids[i], boids[j])) {
                glm::vec3 velA = boids[i].getVelocity();
                glm::vec3 velB = boids[j].getVelocity();

                // Zmiana prędkości tylko w obrębie tej samej grupy
                boids[i].setVelocity(-velA);
                boids[j].setVelocity(-velB);
            }
        }
    }
}


glm::vec3 BoidSystem::separation(const Boid& boid) const {
    glm::vec3 steering(0.0f);
    int count = 0;

    for (const auto& other : boids) {
        if (&boid != &other && boid.getColor() == other.getColor()) { // Sprawdzamy, czy są tej samej grupy
            float distance = glm::distance(boid.getPosition(), other.getPosition());
            if (distance < SEPARATION_RADIUS) {
                glm::vec3 diff = boid.getPosition() - other.getPosition();
                steering += diff / (distance * distance + 0.0001f);
                count++;
            }
        }
    }

    if (count > 0) {
        steering /= static_cast<float>(count);
        steering = limit(steering, MAX_SPEED);
    }
    return steering;
}

glm::vec3 BoidSystem::alignment(const Boid& boid) const {
    glm::vec3 avgVelocity(0.0f);
    int count = 0;

    for (const auto& other : boids) {
        if (&boid != &other && boid.getColor() == other.getColor()) { // Sprawdzamy, czy są tej samej grupy
            float distance = glm::distance(boid.getPosition(), other.getPosition());
            if (distance < ALIGNMENT_RADIUS) {
                avgVelocity += other.getVelocity();
                count++;
            }
        }
    }

    if (count > 0) {
        avgVelocity /= static_cast<float>(count);
        avgVelocity = limit(avgVelocity, MAX_SPEED);
    }
    return avgVelocity;
}

glm::vec3 BoidSystem::cohesion(const Boid& boid) const {
    glm::vec3 center(0.0f);
    int count = 0;

    for (const auto& other : boids) {
        if (&boid != &other && boid.getColor() == other.getColor()) { // Sprawdzamy, czy są tej samej grupy
            float distance = glm::distance(boid.getPosition(), other.getPosition());
            if (distance < COHESION_RADIUS) {
                center += other.getPosition();
                count++;
            }
        }
    }

    if (count > 0) {
        center /= static_cast<float>(count);
        glm::vec3 desired = center - boid.getPosition();
        desired = limit(desired, MAX_SPEED);
        return desired - boid.getVelocity();
    }
    return glm::vec3(0.0f);
}

void BoidSystem::keepWithinBounds(Boid& boid) {
    glm::vec3 pos = boid.getPosition();
    glm::vec3 vel = boid.getVelocity();

    const float margin = 2.0f;
    const float groundLevel = 9.0f;  // Minimalna wysokość (poziom ziemi)
    const float maxHeight = 40.0f;   // Maksymalna wysokość lotu

    // Ograniczenie dla osi X i Z (obecne)
    if (pos.x < -BOUNDARY + margin) vel.x = margin * 0.1f;
    if (pos.x > BOUNDARY - margin) vel.x = -margin * 0.1f;
    if (pos.z < -BOUNDARY + margin) vel.z = margin * 0.1f;
    if (pos.z > BOUNDARY - margin) vel.z = -margin * 0.1f;

    // Ograniczenie dla osi Y (zapobiega wlatywaniu w ziemię)
    if (pos.y < groundLevel) {
        pos.y = groundLevel;  // Jeśli boid jest poniżej ziemi, ustaw jego pozycję na poziomie ziemi
        vel.y = fabs(vel.y) * 0.5f; // Odbicie w górę z osłabieniem
    }

    // Ograniczenie dla maksymalnej wysokości
    if (pos.y > maxHeight) {
        pos.y = maxHeight;  // Jeśli boid przekroczy maxHeight, ustaw jego pozycję na granicy
        vel.y = -fabs(vel.y) * 0.5f; // Odbicie w dół z osłabieniem
    }

    // Ostateczne ograniczenie pozycji X, Z
    pos.x = glm::clamp(pos.x, -BOUNDARY, BOUNDARY);
    pos.z = glm::clamp(pos.z, -BOUNDARY, BOUNDARY);

    boid.setVelocity(vel);
    boid.updatePosition(pos);
}



glm::vec3 BoidSystem::limit(glm::vec3 vec, float max) const {
    float length = glm::length(vec);
    if (length > max) {
        return vec * (max / length);
    }
    return vec;
}
