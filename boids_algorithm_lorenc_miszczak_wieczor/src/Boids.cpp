#include "Headers/boids.h"
#include <cmath>
#include <algorithm>
#include <GL/glew.h>
#include <gtc/matrix_transform.hpp>
#include <ctime>
#include <unordered_map>


GLuint Boid::VAO = 0;
GLuint Boid::VBO = 0;

const float Boid::BOID_VERTICES[] = {
 // Dziób (przedni trójkąt)
    // Przedni trójkąt (dziób)
     0.0f,  0.5f,  1.0f,  // Wierzchołek (dziób)
    -0.5f, -0.5f,  0.0f,  // Lewy dół
     0.5f, -0.5f,  0.0f,  // Prawy dół

    // Tylny trójkąt
     0.0f,  0.5f, -1.0f,  
    -0.5f, -0.5f,  0.0f,  
     0.5f, -0.5f,  0.0f,  

    // Lewa ściana
     0.0f,  0.5f,  1.0f,  
    -0.5f, -0.5f,  0.0f,  
     0.0f,  0.5f, -1.0f,  

    // Prawa ściana
     0.0f,  0.5f,  1.0f,  
     0.5f, -0.5f,  0.0f,  
     0.0f,  0.5f, -1.0f,  

    // Spód
    -0.5f, -0.5f,  0.0f,  
     0.5f, -0.5f,  0.0f,  
     0.0f, -0.5f, -1.0f,  

    // Skrzydło lewe
    -0.5f, -0.5f,  0.0f,  
    -1.0f,  0.0f, -0.5f,  
     0.0f, -0.5f, -0.5f,  

    // Skrzydło prawe
     0.5f, -0.5f,  0.0f,  
     1.0f,  0.0f, -0.5f,  
     0.0f, -0.5f, -0.5f  
};
// Initialize the geometry for the Boid class
void Boid::initializeGeometry() {
    if (VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(BOID_VERTICES), BOID_VERTICES, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);


        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}


// Constructor for the Boid class
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
// Draw the boid using the provided view, projection, and camera position
void Boid::draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos, bool lightingEnabled, GLuint shader) const {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);

    if (glm::length(velocity) > 0.001f) {
        glm::vec3 dir = glm::normalize(velocity);

        float yaw = atan2(dir.x, dir.z);
        float pitch = -asin(dir.y);

        model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));   
        model = glm::rotate(model, pitch, glm::vec3(1.0f, 0.0f, 0.0f)); 
    }

    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform3fv(glGetUniformLocation(shader, "color"), 1, &color[0]);
    glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, &cameraPos[0]);
	glUniform1i(glGetUniformLocation(shader, "lightingEnabled"), lightingEnabled);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 21);  
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

void BoidSystem::handleInput(GLFWwindow* window) {
    float speedChange = 0.1f;

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        MAX_SPEED += speedChange; 
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        MAX_SPEED = std::max(0.0f, MAX_SPEED - speedChange);
    }
}

float BoidSystem::getMaxSpeed() const {
    return MAX_SPEED;
}

void BoidSystem::update(GLFWwindow* window) {
    handleInput(window);

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

void BoidSystem::draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos, bool lightingEnabled, GLuint shader) const {
    for (const auto& boid : boids) {
        boid.draw(view, projection, cameraPos, lightingEnabled, shader);
    }
}

// AABB algorithm
bool BoidSystem::checkCollision(const Boid& a, const Boid& b) const {
    if (a.getColor() != b.getColor()) return false; 

    float size = 0.5f;
    glm::vec3 posA = a.getPosition();
    glm::vec3 posB = b.getPosition();

    return glm::distance(posA, posB) < size;
}
// AABB algorithm
void BoidSystem::resolveCollisions() {
    const float cellSize = 1.0f; 
    std::unordered_map<int, std::vector<Boid*>> grid;

    auto getCellIndex = [cellSize](const glm::vec3& pos) {
        int x = static_cast<int>(pos.x / cellSize);
        int y = static_cast<int>(pos.y / cellSize);
        int z = static_cast<int>(pos.z / cellSize);
        return (x * 73856093) ^ (y * 19349663) ^ (z * 83492791); 
        };

    for (auto& boid : boids) {
        int cellIndex = getCellIndex(boid.getPosition());
        grid[cellIndex].push_back(&boid);
    }

    for (auto& cell : grid) {
        auto& cellBoids = cell.second;
        for (size_t i = 0; i < cellBoids.size(); ++i) {
            for (size_t j = i + 1; j < cellBoids.size(); ++j) {
                if (checkCollision(*cellBoids[i], *cellBoids[j])) {
                    glm::vec3 velA = cellBoids[i]->getVelocity();
                    glm::vec3 velB = cellBoids[j]->getVelocity();

                    // Change velocity only within the same group
                    cellBoids[i]->setVelocity(-velA);
                    cellBoids[j]->setVelocity(-velB);
                }
            }
        }
    }
}


// implementations rules of boids algorithm
glm::vec3 BoidSystem::separation(const Boid& boid) const {
    glm::vec3 steering(0.0f);
    int count = 0;

    for (const auto& other : boids) {
        if (&boid != &other && boid.getColor() == other.getColor()) { 
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

// implementations rules of boids algorithm
glm::vec3 BoidSystem::alignment(const Boid& boid) const {
    glm::vec3 avgVelocity(0.0f);
    int count = 0;

    for (const auto& other : boids) {
        if (&boid != &other && boid.getColor() == other.getColor()) { 
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
// implementations rules of boids algorithm
glm::vec3 BoidSystem::cohesion(const Boid& boid) const {
    glm::vec3 center(0.0f);
    int count = 0;

    for (const auto& other : boids) {
        if (&boid != &other && boid.getColor() == other.getColor()) {
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
//limitation of boids movement space
void BoidSystem::keepWithinBounds(Boid& boid) {
    glm::vec3 pos = boid.getPosition();
    glm::vec3 vel = boid.getVelocity();

    const float margin = 2.0f;
    const float groundLevel = 16.0f;
    const float maxHeight = 40.0f;  

    if (pos.x < -BOUNDARY + margin) vel.x = margin * 0.1f;
    if (pos.x > BOUNDARY - margin) vel.x = -margin * 0.1f;
    if (pos.z < -BOUNDARY + margin) vel.z = margin * 0.1f;
    if (pos.z > BOUNDARY - margin) vel.z = -margin * 0.1f;

    if (pos.y < groundLevel) {
        pos.y = groundLevel;
        vel.y = fabs(vel.y) * 0.5f;
    }

    if (pos.y > maxHeight) {
        pos.y = maxHeight;  
        vel.y = -fabs(vel.y) * 0.5f;
    }

    pos.x = glm::clamp(pos.x, -BOUNDARY, BOUNDARY);
    pos.z = glm::clamp(pos.z, -BOUNDARY, BOUNDARY);

    boid.setVelocity(vel);
    boid.updatePosition(pos);
}

void BoidSystem::applyMouseForce(glm::vec3 mousePos, bool attract) {
    for (auto& boid : boids) {
        glm::vec3 direction = mousePos - boid.position;
        float distance = glm::length(direction);
        if (distance < 100.0f) { 
            float strength = 5.0f / (distance + 1.0f); 
            if (!attract) strength *= -1;
            boid.velocity += glm::normalize(direction) * strength;
        }
    }
}

glm::vec3 BoidSystem::limit(glm::vec3 vec, float max) const {
    float length = glm::length(vec);
    if (length > max) {
        return vec * (max / length);
    }
    return vec;
}
