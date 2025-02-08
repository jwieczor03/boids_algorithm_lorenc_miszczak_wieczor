#ifndef BOIDS_H
#define BOIDS_H

#include <vector>
#include <GL/glew.h>

class Vec3 {
public:
    float x, y, z;
    Vec3();
    Vec3(float x, float y, float z);
    Vec3 operator+(const Vec3& other) const;
    Vec3& operator+=(const Vec3& other);
    Vec3 operator-(const Vec3& other) const;
    Vec3 operator*(float scalar) const;
    Vec3& operator*=(float scalar);
    float length() const;
    Vec3 normalized() const;
};

// In Boids.h
class Color {
public:
    float r, g, b;
    Color() : r(0), g(0), b(0) {} // Default constructor
    Color(float r, float g, float b);
    bool operator==(const Color& other) const;
    bool operator!=(const Color& other) const;
};


class Boid {
public:
    Boid(Vec3 position, Vec3 velocity, int group, Color color);
    void updatePosition(const Vec3& newPos);
    Vec3 getPosition() const;
    Vec3 getVelocity() const;
    void setVelocity(const Vec3& velocity);
    void draw() const;
    void setPosition(Vec3 position);
    Color getColor() const;

private:
    Vec3 position;
    Vec3 velocity;
    int group;
    Color color;
    void drawBoid() const;
};

class BoidSystem {
public:
    BoidSystem(int numBoids, int numGroups);
    void update();
    void draw() const;

private:
    std::vector<Boid> boids;
    const float SEPARATION_RADIUS = 1.0f;
    const float ALIGNMENT_RADIUS = 3.0f;
    const float COHESION_RADIUS = 3.0f;
    const float GROUP_AVOID_RADIUS = 3.0f;
    const float MAX_SPEED = 0.3f;  // Stała prędkość
    const float MAX_FORCE = 0.03f;

    const float BOUNDARY_MIN = -5.0f;
    const float BOUNDARY_MAX = 5.0f;

    Vec3 separation(const Boid& boid, bool sameGroup);
    Vec3 alignment(const Boid& boid, bool sameGroup);
    Vec3 cohesion(const Boid& boid, bool sameGroup);
    Vec3 groupAvoidance(const Boid& boid);
    Vec3 limit(const Vec3& v, float max);
    void keepWithinBounds(Boid& boid);
};

#endif // BOIDS_H
