#include <GL/glew.h>
#include "Boids.h"
#include <cmath>
#include <utility>
#include <cstdlib>
#include <iostream>

Boid::Boid(float x, float y, float vx, float vy, int group)
    : x(x), y(y), vx(vx), vy(vy), group(group) {
    // Assign colors based on group
    switch (group) {
    case 0: r = 1.0f; g = 0.0f; b = 0.0f; break; // Red
    case 1: r = 0.0f; g = 1.0f; b = 0.0f; break; // Green
    case 2: r = 0.0f; g = 0.0f; b = 1.0f; break; // Blue
    default: r = 1.0f; g = 1.0f; b = 1.0f; break; // White
    }
}

void Boid::update(const std::vector<Boid>& boids) {
    std::pair<float, float> alignment = align(boids);
    std::pair<float, float> cohesionForce = cohesion(boids);
    std::pair<float, float> separationForce = separation(boids);
    std::pair<float, float> interGroupSeparationForce = interGroupSeparation(boids);

    vx += alignment.first + cohesionForce.first + separationForce.first + interGroupSeparationForce.first;
    vy += alignment.second + cohesionForce.second + separationForce.second + interGroupSeparationForce.second;

    // Limit the speed of the boids
    float speed = std::sqrt(vx * vx + vy * vy);
    float maxSpeed = 1.0f; // Reduced max speed to make boids move slower
    float minSpeed = 0.5f; // Minimum speed to prevent boids from stopping
    if (speed > maxSpeed) {
        vx = (vx / speed) * maxSpeed;
        vy = (vy / speed) * maxSpeed;
    }
    else if (speed < minSpeed) {
        vx = (vx / speed) * minSpeed;
        vy = (vy / speed) * minSpeed;
    }

    x += vx;
    y += vy;

    // Keep boids within window bounds
    if (x < 0) x += 800;
    if (x > 800) x -= 800;
    if (y < 0) y += 600;
    if (y > 600) y -= 600;
}

void Boid::draw() const {
    glColor3f(r, g, b);
    glBegin(GL_TRIANGLES);
    glVertex2f(x, y);
    glVertex2f(x + 5.0f, y + 10.0f);
    glVertex2f(x - 5.0f, y + 10.0f);
    glEnd();
}

std::pair<float, float> Boid::align(const std::vector<Boid>& boids) {
    float neighborDist = 50.0f;
    float avgVx = 0.0f;
    float avgVy = 0.0f;
    int count = 0;

    for (const Boid& other : boids) {
        if (other.group != group) continue; // Only align with boids of the same group
        float d = std::sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
        if ((d > 0) && (d < neighborDist)) {
            avgVx += other.vx;
            avgVy += other.vy;
            count++;
        }
    }

    if (count > 0) {
        avgVx /= count;
        avgVy /= count;

        // Steer towards the average velocity
        avgVx = (avgVx - vx) * 0.05f;
        avgVy = (avgVy - vy) * 0.05f;
    }

    return { avgVx, avgVy };
}

std::pair<float, float> Boid::cohesion(const std::vector<Boid>& boids) {
    float neighborDist = 50.0f;
    float avgX = 0.0f;
    float avgY = 0.0f;
    int count = 0;

    for (const Boid& other : boids) {
        if (other.group != group) continue; // Only cohere with boids of the same group
        float d = std::sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
        if ((d > 0) && (d < neighborDist)) {
            avgX += other.x;
            avgY += other.y;
            count++;
        }
    }

    if (count > 0) {
        avgX /= count;
        avgY /= count;

        // Steer towards the average position
        avgX = (avgX - x) * 0.01f;
        avgY = (avgY - y) * 0.01f;
    }

    return { avgX, avgY };
}

std::pair<float, float> Boid::separation(const std::vector<Boid>& boids) {
    float desiredSeparation = 25.0f;
    float steerX = 0.0f;
    float steerY = 0.0f;
    int count = 0;

    for (const Boid& other : boids) {
        if (other.group != group) continue; // Only separate from boids of the same group
        float d = std::sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
        if ((d > 0) && (d < desiredSeparation)) {
            float diffX = x - other.x;
            float diffY = y - other.y;
            diffX /= d;
            diffY /= d;
            steerX += diffX;
            steerY += diffY;
            count++;
        }
    }

    if (count > 0) {
        steerX /= count;
        steerY /= count;
    }

    return { steerX, steerY };
}

std::pair<float, float> Boid::interGroupSeparation(const std::vector<Boid>& boids) {
    float desiredSeparation = 50.0f; // Larger separation distance for inter-group separation
    float steerX = 0.0f;
    float steerY = 0.0f;
    int count = 0;

    for (const Boid& other : boids) {
        if (other.group == group) continue; // Only separate from boids of different groups
        float d = std::sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
        if ((d > 0) && (d < desiredSeparation)) {
            float diffX = x - other.x;
            float diffY = y - other.y;
            diffX /= d;
            diffY /= d;
            steerX += diffX;
            steerY += diffY;
            count++;
        }
    }

    if (count > 0) {
        steerX /= count;
        steerY /= count;
    }

    return { steerX, steerY };
}

BoidSystem::BoidSystem(int numBoids) {
    for (int i = 0; i < numBoids; ++i) {
        int group = i % 3; // Assign groups in a round-robin fashion
        boids.emplace_back(rand() % 800, rand() % 600, (rand() % 100 - 50) / 200.0f, (rand() % 100 - 50) / 200.0f, group); // Reduced initial speed
    }
}

void BoidSystem::update() {
    for (Boid& boid : boids) {
        boid.update(boids);
    }
}

void BoidSystem::draw() const {
    for (const Boid& boid : boids) {
        boid.draw();
    }
}
