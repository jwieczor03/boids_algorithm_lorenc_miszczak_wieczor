#include "Boids.h"
#include <cmath>
#include <GL/glew.h>
#include <vector>
#include <iostream>
#include <algorithm> 

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

Vec3::Vec3() : x(0), y(0), z(0) {}
Vec3::Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

Vec3 Vec3::operator+(const Vec3& other) const {
    return Vec3(x + other.x, y + other.y, z + other.z);
}

Vec3& Vec3::operator+=(const Vec3& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Vec3 Vec3::operator-(const Vec3& other) const {
    return Vec3(x - other.x, y - other.y, z - other.z);
}

Vec3 Vec3::operator*(float scalar) const {
    return Vec3(x * scalar, y * scalar, z * scalar);
}

Vec3& Vec3::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

float Vec3::length() const {
    return std::sqrt(x * x + y * y + z * z);
}

Vec3 Vec3::normalized() const {
    float len = length();
    if (len > 0) {
        return *this * (1.0f / len);
    }
    return Vec3(0, 0, 0);
}

Color::Color(float r, float g, float b) : r(r), g(g), b(b) {}

bool Color::operator==(const Color& other) const {
    return r == other.r && g == other.g && b == other.b;
}

bool Color::operator!=(const Color& other) const {
    return !(*this == other);
}

Boid::Boid(Vec3 position, Vec3 velocity, int group, Color color)
    : position(position), velocity(velocity), group(group), color(color) {
}

void Boid::updatePosition(const Vec3& newPos) {
    position = newPos;
}

Vec3 Boid::getPosition() const {
    return position;
}

Vec3 Boid::getVelocity() const {
    return velocity;
}

void Boid::setVelocity(const Vec3& velocity) {
    this->velocity = velocity;
}

void Boid::setPosition(Vec3 position) {
    this->position = position;
}

Color Boid::getColor() const {
    return color;
}

void Boid::draw() const {
    glColor3f(color.r, color.g, color.b);
    drawBoid();
}

void Boid::drawBoid() const {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glBegin(GL_TRIANGLES);
    glVertex3f(0.0f, 0.1f, 0.0f);  // Front point
    glVertex3f(-0.05f, -0.05f, 0.05f); // Left
    glVertex3f(0.05f, -0.05f, 0.05f);  // Right

    glVertex3f(0.0f, 0.1f, 0.0f);  // Front point
    glVertex3f(0.05f, -0.05f, 0.05f); // Right
    glVertex3f(0.05f, -0.05f, -0.05f); // Bottom

    glVertex3f(0.0f, 0.1f, 0.0f);  // Front point
    glVertex3f(0.05f, -0.05f, -0.05f); // Right
    glVertex3f(-0.05f, -0.05f, -0.05f); // Left

    glVertex3f(0.0f, 0.1f, 0.0f);  // Front point
    glVertex3f(-0.05f, -0.05f, -0.05f); // Left
    glVertex3f(-0.05f, -0.05f, 0.05f);  // Left
    glEnd();
    glPopMatrix();
}

BoidSystem::BoidSystem(int numBoids, int numGroups) {
    for (int i = 0; i < numBoids; ++i) {
        int group = i % numGroups;
        Color color;
        if (group == 0) {
            color = Color(1.0f, 0.0f, 0.0f); // Red
        }
        else if (group == 1) {
            color = Color(0.0f, 1.0f, 0.0f); // Green
        }
        else {
            color = Color(0.0f, 0.0f, 1.0f); // Blue
        }
        boids.emplace_back(Vec3(rand() % 10 - 5, rand() % 10 - 5, rand() % 20 - 20), Vec3(0.1f, 0.1f, 0.0f), group, color);
    }
}



Vec3 BoidSystem::separation(const Boid& boid, bool sameGroup) {
    Vec3 steering(0, 0, 0);
    int total = 0;
    for (const Boid& other : boids) {
        if (&boid != &other) {
            float distance = (boid.getPosition() - other.getPosition()).length();
            if (distance < SEPARATION_RADIUS) {
                Vec3 diff = boid.getPosition() - other.getPosition();
                diff = diff.normalized();
                steering += diff;
                total++;
            }
        }
    }
    if (total > 0) {
        steering *= (1.0f / total);
    }
    return steering;
}

Vec3 BoidSystem::alignment(const Boid& boid, bool sameGroup) {
    Vec3 steering(0, 0, 0);
    int total = 0;
    for (const Boid& other : boids) {
        if (&boid != &other && (sameGroup ? boid.getColor() == other.getColor() : true)) {
            steering += other.getVelocity();
            total++;
        }
    }
    if (total > 0) {
        steering *= (1.0f / total);
        steering = steering.normalized();
    }
    return steering;
}

Vec3 BoidSystem::cohesion(const Boid& boid, bool sameGroup) {
    Vec3 steering(0, 0, 0);
    int total = 0;
    for (const Boid& other : boids) {
        if (&boid != &other && (sameGroup ? boid.getColor() == other.getColor() : true)) {
            steering += other.getPosition();
            total++;
        }
    }
    if (total > 0) {
        steering *= (1.0f / total);
        steering = steering - boid.getPosition();
        steering = steering.normalized();
    }
    return steering;
}

Vec3 BoidSystem::groupAvoidance(const Boid& boid) {
    Vec3 steering(0, 0, 0);
    for (const Boid& other : boids) {
        if (&boid != &other && boid.getColor() != other.getColor()) {
            float distance = (boid.getPosition() - other.getPosition()).length();
            if (distance < GROUP_AVOID_RADIUS) {
                Vec3 diff = boid.getPosition() - other.getPosition();
                diff = diff.normalized();
                steering += diff;
            }
        }
    }
    return steering;
}

Vec3 BoidSystem::limit(const Vec3& v, float max) {
    if (v.length() > max) {
        return v.normalized() * max;
    }
    return v;
}
void BoidSystem::keepWithinBounds(Boid& boid) {
    Vec3 position = boid.getPosition();
    Vec3 velocity = boid.getVelocity();

    // Window boundaries
    float halfWidth = WINDOW_WIDTH / 2.0f;
    float halfHeight = WINDOW_HEIGHT / 2.0f;

    // Reflect off the boundaries in the X axis
    if (position.x < -halfWidth) {
        position.x = -halfWidth;
        velocity.x = std::abs(velocity.x); // Reflect off the left edge
    }
    else if (position.x > halfWidth) {
        position.x = halfWidth;
        velocity.x = -std::abs(velocity.x); // Reflect off the right edge
    }

    // Reflect off the boundaries in the Y axis
    if (position.y < -halfHeight) {
        position.y = -halfHeight;
        velocity.y = std::abs(velocity.y); // Reflect off the bottom edge
    }
    else if (position.y > halfHeight) {
        position.y = halfHeight;
        velocity.y = -std::abs(velocity.y); // Reflect off the top edge
    }

    // Reflect off the boundaries in the Z axis (in 3D)
    if (position.z < -halfHeight) {
        position.z = -halfHeight;
        velocity.z = std::abs(velocity.z); // Reflect off the back edge
    }
    else if (position.z > halfHeight) {
        position.z = halfHeight;
        velocity.z = -std::abs(velocity.z); // Reflect off the front edge
    }

    // Ensure the boid does not exceed the boundaries due to high speed
    boid.updatePosition(position);
    boid.setVelocity(velocity);
}






void BoidSystem::update() {
    for (Boid& boid : boids) {
        // Calculate the steering forces
        Vec3 sep = separation(boid, true);
        Vec3 ali = alignment(boid, true);
        Vec3 coh = cohesion(boid, true);

        // Calculate new velocity
        Vec3 newVelocity = boid.getVelocity() + sep + ali + coh;

        // Limit the velocity to a maximum value
        newVelocity = limit(newVelocity, MAX_SPEED);

        // Update the boid's velocity and position
        boid.setVelocity(newVelocity);
        boid.updatePosition(boid.getPosition() + boid.getVelocity());

        // Ensure the boid stays within the window boundaries
        keepWithinBounds(boid);
    }
}



void BoidSystem::draw() const {
    for (const Boid& boid : boids) {
        boid.draw();
    }
}
