#ifndef BOIDS_H
#define BOIDS_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>

class Boid {
public:
    Boid(float x, float y, float vx, float vy, int group);
    void update(const std::vector<Boid>& boids);
    void draw() const;
    float x, y;
    float vx, vy;
    float r, g, b;
    int group;

    std::pair<float, float> align(const std::vector<Boid>& boids);
    std::pair<float, float> cohesion(const std::vector<Boid>& boids);
    std::pair<float, float> separation(const std::vector<Boid>& boids);
    std::pair<float, float> interGroupSeparation(const std::vector<Boid>& boids);
};

class BoidSystem {
public:
    BoidSystem(int numBoids);
    void update();
    void draw() const;
private:
    std::vector<Boid> boids;
};

#endif // BOIDS_H
